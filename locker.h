#pragma once
#ifndef LOCKER_H
#define LOCKER_H

#include <pthread.h>
#include <exception>
#include <semaphore.h>

//线程同步机制封装类，互斥锁、条件、信号量

class locker{   //互斥锁
public:
    locker(){
        if(pthread_mutex_init(&m_mutex,NULL) != 0){
            throw std::exception();
        }
    }

    ~locker(){
        pthread_mutex_destroy(&m_mutex);
    }

    bool lock(){    //上锁
        return pthread_mutex_lock(&m_mutex) == 0;
    }

    bool unlock(){  //解锁
        return pthread_mutex_unlock(&m_mutex) == 0;
    }

    pthread_mutex_t * get(){
        return &m_mutex;
    }

private:
    pthread_mutex_t m_mutex;

};

class cond{ //条件变量
public:
    cond(){
        if(pthread_cond_init(&m_cond,NULL) != 0){
            throw std::exception();
        }
    }

    ~cond(){
        pthread_cond_destroy(&m_cond);
    }

    bool wait(pthread_mutex_t* mutex){  //无条件等待
        return pthread_cond_wait(&m_cond,mutex)==0;
    }

    bool timewait(pthread_mutex_t* mutex,struct timespec t){    //限时等待
        return pthread_cond_timedwait(&m_cond,mutex,&t) == 0;
    }

    bool signal(pthread_mutex_t* mutex){    //唤醒一个线程
        return pthread_cond_signal(&m_cond) == 0;
    }

    bool broadcast(pthread_mutex_t* mutex){ //唤醒所有线程
        return pthread_cond_broadcast(&m_cond) == 0;
    }

private:
    pthread_cond_t m_cond;
};

class sem{  //信号量
public:
    sem(){
        if(sem_init(&m_sem,0,0)!=0){
            throw std::exception();
        }
    }
    ~sem(){
        sem_destroy(&m_sem);
    }
    
    bool wait(){    //等待信号量
        return sem_wait(&m_sem) == 0;
    }
    
    bool post(){    //增加信号量
        return sem_post(&m_sem) == 0;
    }
private:
    sem_t m_sem;
};


#endif