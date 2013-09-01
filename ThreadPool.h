/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           ThreadPool.h
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Declaration of the class ThreadPool
**                      Based on Linux 2.6.36. 
**------------------------------------------------------------------------------------------
** Created by:          Cory Tang
** Created date:        2013-08-05
**------------------------------------------------------------------------------------------
** Modified by:   
** Modified date: 
** Version:    
** Descriptions: 
*******************************************************************************************/

#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <list>
#include <string>
#include <stdexcept>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

template <class T>
class ThreadPool{
public:
	typedef void* (* CALLEE_FUN)(T& job, ThreadPool<T> *pool);
//	typedef void* (* ADD_JOB_FUN)(const T& job);
//	ADD_JOB_FUN threadpool_add_job;
	ThreadPool(CALLEE_FUN userCallee = NULL, bool needCheckWhenAddJob = false, size_t threadNum = 20, size_t maxJobQueueLen = 200);
	virtual ~ThreadPool();
	int threadpool_add_job(const T& job);
	int threadpool_destroy();

private:
	T threadpool_get_job(pthread_t tId); //Should be private. Called auto by threadpool_caller.
	T threadpool_get_job();//Should be private. 
	int threadpool_add_job_no_check(const T& job); //pass by reference to elavte the effect.
	int threadpool_add_job_check_dup(const T& job);
	const bool m_needCheckWhenAddJob;
	static void* threadpool_caller(void *arg); //pass "this" as parameter
	CALLEE_FUN m_userCallee;
	const size_t m_threadNum;
	const size_t m_maxJobQueueLen;
	size_t m_completedJobNum;
	pthread_cond_t m_jobQueueIsNotEmpty;
	pthread_cond_t m_jobQueueIsNotFull;
	pthread_mutex_t m_jobQueueMutex;
	pthread_mutex_t m_jobSetMutex;
	list<T> m_jobToDoQueue;
	set<T> m_jobAllSet; //
	list<pthread_t> m_threadIdList;
	bool m_threadpoolIsClosed;
};

#include "ThreadPool.cpp"

#endif
