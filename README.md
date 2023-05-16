# EasyWebserver
## 1. 一个简单的webserver
- 目标: 实现一个简单的webserver,完成web与本地server的连接以及信息的接收与发送 
- **review**: 线程池的设计与应用,
## 2. 线程池(threadpool)
- 工作原理: 使用任务队列进行任务分发 -> 线程执行任务
- **review**: 线程的同步机制,包括互斥量、条件变量、信号量。
### 线程(pthread_t)
-  pthread_create().创建线程.
-  pthread_detach().设置线程分离状态，此状态下线程终止后会自动释放资源，而不会产生僵尸线程.
### 互斥量(pthread_mutex_t)
- pthread_mutex_init().初始化.将临界区声明为互斥量,同一时间只能由一个线程对其进行操作.
- pthread_mutex_lock().上锁.操作之前要上锁.
- pthread_mutex_unlock().解锁.操作完之后要解锁.
### 条件变量(pthread_cond_t)
- pthread_cond_init().初始化.
- pthread_cond_wait().等待条件,将线程阻塞.
- pthread_cond_signal().唤醒一个等待的线程,需要与互斥锁结合使用.
- pthread_cond_broadcast().唤醒所有等待的线程,需要与互斥锁结合使用.
### 信号量
- sem_init().
- sem_wait().将信号量-1,如果为0就将线程阻塞在这.
- sem_post().将信号量+1.
## 3. 错误记录
- 在创建线程池的时候忘记 new 一个 m_threads ,导致程序对空指针进行操作.
- 多加了 EPOLLET 状态 ， 导致测试时在挂起状态时没有出现不停读取的效果，而是读取一次就停止了。