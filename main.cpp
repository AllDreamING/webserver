#include<stdio.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include"threadpool.h"
#include"locker.h"
#include<signal.h>
#include"http_conn.h"

#define MAX_FD 65535    //最大文件描述符个数
#define MAX_EVENT_NUMBER 10000  //最大监听事件个数

void addsig(int sig,void(handler)(int)){    //信号捕捉函数
    struct sigaction sa;        //信号处理结构体
    memset(&sa,0,sizeof(sa));   //清空
    sa.sa_handler = handler;    //处理函数
    sigfillset(&sa.sa_mask);    //将信号加入到阻塞集中
    sigaction(sig,&sa,NULL);    //sa对接收到的信号sig进行处理
}

extern void addfd(int epollfd,int fd,bool oneshot); //添加文件描述符到epoll中
extern void removefd(int epollfd,int fd);   //从epoll中删除文件描述符
extern void modfd(int epollfd,int fd,int ev);   //修改文件描述符,重置socket上的oneshot事件

int main(int argc,char* argv[]){

    if(argc<=1){
        printf("按照如下格式运行： %s port_number\n",basename(argv[0]));
        exit(-1);
    }

    int port = atoi(argv[1]);   //获取端口号

    addsig(SIGPIPE,SIG_IGN);    //忽略SIGPIE信号

    threadpool<http_conn>* pool = NULL; //创建线程池
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch(...)
    {
        exit(-1);
    }

    http_conn* users = new http_conn[MAX_FD];   //创建一个数组用来存放客户端信息

    int listenfd = socket(PF_INET,SOCK_STREAM,0);   //创建一个监听的套接字

    int reuse = 1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));  //设置端口复用,须在绑定之前

    struct sockaddr_in address;
    address.sin_family = AF_INET;   //设置地址家族，TCP/IP – IPv4
    address.sin_addr.s_addr = INADDR_ANY;   //IP地址为任意地址
    address.sin_port = htons(port); //端口号，将主机字节序转为网络字节序
    bind(listenfd,(struct sockaddr*)&address,sizeof(address));  //绑定

    listen(listenfd,5); //监听

    epoll_event events[MAX_EVENT_NUMBER];   //事件数组
    int epollfd  = epoll_create(5); //创建epoll对象

    addfd(epollfd,listenfd,false);
    http_conn::m_epollfd = epollfd;

    while (true){
        int num = epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if((num < 0) && (errno != EINTR)){
            printf("epoll fail\n");
            break;
        }

        for(int i=0;i<num;i++){
            int sockfd = events[i].data.fd;
            if(sockfd==listenfd){   //有客户端连接进来
                struct sockaddr_in client_address;
                socklen_t client_addrlen = sizeof(client_address);
                int connfd = accept(listenfd,(struct sockaddr*)&client_address,&client_addrlen);//建立连接

                if(http_conn::m_user_count >= MAX_FD){  //目前的连接数满了
                    close(connfd);
                    continue;
                }
                users[connfd].init(connfd,client_address);//将新的客户端的数据初始化放到数组中
            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){//对方异常断开或者错误的事件发生
                users[sockfd].close_conn();//关闭连接
            }else if(events[i].events & EPOLLIN){   //有 读 的事件发生
                if(users[sockfd].read()){   //一次性将所有数据读完
                    pool->append(users + sockfd);   //将任务转交线程进行执行
                }else{
                    users[sockfd].close_conn(); //读失败就关闭连接
                }
            }else if(events[i].events & EPOLLOUT){  //有 写 的事件发生
                if(!users[sockfd].write()){     //一次性写完所有数据
                    users[sockfd].close_conn(); //写完就关闭连接
                }
            }
        }
    }

    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;

    return 0;
}