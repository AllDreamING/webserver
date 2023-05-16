#pragma once
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<pthread.h>
#include<list>
#include"locker.h"

template<typename T>
class threadpool{
public:
    threadpool(int thread_number = 8, int max_requests = 10000);
    ~threadpool();
    bool append(T* request);    //将任务添加到请求队列

private:
    static void* worker(void* arg);
    void run();

private:
    int m_thread_number;    //线程数量
    pthread_t* m_threads;   //线程池数组
    int m_max_requests;     //请求队列最多允许的等待处理的请求的数量
    std::list<T*> m_workqueue;  //请求队列
    locker m_queuelocker;   //互斥锁
    sem m_queuestat;    //信号量用来判断是否有任务等待处理
    bool m_stop;        //是否结束线程
};

#endif

template <typename T>
threadpool<T>::threadpool(int thread_number, int max_requests):
m_thread_number(thread_number),m_max_requests(max_requests),m_stop(false),m_threads(NULL){
    if(thread_number<=0 || max_requests<=0){    //首先判断线程数量请求队列长度是否合法
        throw std::exception();
    }

    for(int i=0;i<thread_number;i++){   //创建 m_thread_number 个线程，并将其设置为脱离
        printf("create the %dth thread \n",i);
        
        m_threads = new pthread_t[m_thread_number];
        if(!m_threads){ //创建未成功
            throw std::exception();
        }

        if(pthread_create(m_threads+i,NULL,worker,this)!=0){    //创建线程
            delete [] m_threads;
            printf("创建线程失败 \n");
            throw std::exception();
        }

        if(pthread_detach(m_threads[i])){    //设置线程分离
            delete[] m_threads;
            printf("设置线程分离失败 \n");
            throw std::exception();
        }
    }
}

template <typename T>
threadpool<T>::~threadpool(){
    delete[] m_threads;
    m_stop = true;
}

template <typename T>
bool threadpool<T>::append(T *request){
    m_queuelocker.lock();   //先上锁
    if(m_workqueue.size() > m_max_requests){    //容量不足无法添加
        m_queuelocker.unlock(); 
        return false;
    }

    m_workqueue.push_back(request); //将任务添加到队列
    m_queuelocker.unlock(); //解锁
    m_queuestat.post();     //改变信号量
    return true;
}

template <typename T>
void *threadpool<T>::worker(void *arg){ //静态函数无法访问非静态成员，需要封装一下
    threadpool* pool = (threadpool*)arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run(){
    while(!m_stop){
        m_queuestat.wait(); //是否有任务
        m_queuelocker.lock();   //上锁
        if(m_workqueue.empty()){    //任务队列为空
            m_queuelocker.unlock();
            continue;
        }

        T* request = m_workqueue.front();   //获取第一个
        m_workqueue.pop_front();    //删掉
        m_queuelocker.unlock();     //解锁

        if(!request){
            continue;
        }

        request->process();     //执行任务
    }
}
