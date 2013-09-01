/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           ThreadPool.cpp
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Definition of the class ThreadPool
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

template <class T>
ThreadPool<T>::ThreadPool(CALLEE_FUN userCallee, bool needCheckWhenAddJob, size_t threadNum, size_t maxJobQueueLen):m_userCallee(userCallee), m_needCheckWhenAddJob(needCheckWhenAddJob), m_threadNum(threadNum), m_maxJobQueueLen(maxJobQueueLen), m_completedJobNum(0), m_threadpoolIsClosed(false)
{	
//	cout<<"In ThreadPool Constructor!"<<endl;
	pthread_mutex_init(&m_jobQueueMutex, NULL);
	pthread_mutex_init(&m_jobSetMutex, NULL);

	pthread_cond_init(&m_jobQueueIsNotEmpty, NULL);
	pthread_cond_init(&m_jobQueueIsNotFull, NULL);

	list<pthread_t>::iterator tmpIter = m_threadIdList.begin();
	pthread_t tmpThreadId;

	for(size_t i = 0; i < m_threadNum; i++)
	{
		pthread_create(&tmpThreadId, NULL, threadpool_caller, this); //pass the "this" as parameter
		tmpIter = m_threadIdList.insert(tmpIter, tmpThreadId); //insert the object ahead of tmpIter
	}
}

template <class T>
ThreadPool<T>::~ThreadPool()
{	
	if(m_threadpoolIsClosed == false)
		threadpool_destroy();
}

template <class T>
int ThreadPool<T>::threadpool_destroy()
{
	if(m_threadpoolIsClosed == true)
	{
//		cout<<"ThreadPool has been closed!"<<endl;
		return -1; 
	}
	
	m_threadpoolIsClosed = true;
	pthread_cond_broadcast(&m_jobQueueIsNotEmpty);
	pthread_cond_broadcast(&m_jobQueueIsNotFull);
	
	list<pthread_t>::iterator tmpIter = m_threadIdList.begin();
	while(tmpIter != m_threadIdList.end())
	{
		pthread_join(*tmpIter, NULL);
		tmpIter++;
	}
	pthread_mutex_destroy(&m_jobQueueMutex);
	pthread_cond_destroy(&m_jobQueueIsNotEmpty);
	pthread_cond_destroy(&m_jobQueueIsNotFull);
	
	return 0;
}


template <class T>
T ThreadPool<T>::threadpool_get_job()
{
	T job = m_jobToDoQueue.front();
	m_jobToDoQueue.pop_front();
	return job;

}

template <class T>
T ThreadPool<T>::threadpool_get_job(pthread_t tId)
{
	//This function should return an job according to the thread id, but by default it just call the simple overload version.
	return threadpool_get_job();
}



template <class T>
void* ThreadPool<T>::threadpool_caller(void *arg)
{
//	cout<<"in threadpool_caller"<<endl;
	ThreadPool<T>* pTP = (ThreadPool<T> *)arg;
	while(1)
	{
//		cout<<"in threadpool_caller while(1)"<<endl;
		pthread_mutex_lock(&(pTP->m_jobQueueMutex));
//		cout<<"in threadpool_caller while(1) after lock"<<endl;
		while(((pTP->m_jobToDoQueue).empty() == true) &&(pTP->m_threadpoolIsClosed == false))
		{
			
//			cout<<"in threadpool_caller while(cond)"<<endl;
			pthread_cond_wait(&(pTP->m_jobQueueIsNotEmpty), &(pTP->m_jobQueueMutex));
		}
		if(pTP->m_threadpoolIsClosed == true)
		{
			pthread_mutex_unlock(&(pTP->m_jobQueueMutex));
			pthread_exit(NULL);
		}
	
	
//		T job = (pTP->m_jobToDoQueue).front();
//		(pTP->m_jobToDoQueue).pop();
		T job = pTP->threadpool_get_job();
		(pTP->m_completedJobNum)++;

		if((pTP->m_jobToDoQueue).size() == pTP->m_maxJobQueueLen-1)
		{
			pthread_cond_broadcast(&(pTP->m_jobQueueIsNotFull));
		}
		
		pthread_mutex_unlock(&(pTP->m_jobQueueMutex));
		
//		cout<<"Before call callee"<<endl;
		if((pTP->m_userCallee) != NULL)
			(*(pTP->m_userCallee))(job, pTP);

	}
}

template <class T>
int ThreadPool<T>::threadpool_add_job_no_check(const T& job)
{
	cout<<"In threadpool_add_job start"<<endl;
	pthread_mutex_lock(&m_jobQueueMutex);
	cout<<"In threadpool_add_job after lock"<<endl;
	
	while((m_jobToDoQueue.size() == m_maxJobQueueLen) && (m_threadpoolIsClosed == false))
	{	
		cout<<"In threadpool_add_job in while(cond)"<<endl;
		pthread_cond_wait(&m_jobQueueIsNotFull, &m_jobQueueMutex);
	}

	if(m_threadpoolIsClosed == true)
	{
		pthread_mutex_unlock(&m_jobQueueMutex);
		return -1;
	}
	
	m_jobToDoQueue.push_back(job);
	
	if(m_jobToDoQueue.size() == 1)
	{
		pthread_cond_broadcast(&m_jobQueueIsNotEmpty);
	}
	pthread_mutex_unlock(&m_jobQueueMutex);

//	cout<<"In threadpool_add_job after unlock"<<endl;
	return 1;
}


template <class T>
int ThreadPool<T>::threadpool_add_job_check_dup(const T& job)
{
	pthread_mutex_lock(&m_jobSetMutex);
/*	
 	if(m_jobAllSet.size()*sizeof(T) > 4000000)
	{
		cout<<"The size of the job set is more than 4MB."<<endl;
		cout<<"The thread pool is forced to be finished."<<endl;
//		using ThreadPool<T>::threadpool_destroy;
		threadpool_destroy();
		return -1;
	}
*/	
	if(m_jobAllSet.count(job))
	{
		pthread_mutex_unlock(&m_jobSetMutex);
		return 0;
	}
	
	m_jobAllSet.insert(job);
	pthread_mutex_unlock(&m_jobSetMutex);

	return threadpool_add_job_no_check(job);
/*	

//	cout<<"In threadpool_add_job start"<<endl;
	pthread_mutex_lock(&m_jobQueueMutex);
//	cout<<"In threadpool_add_job after lock"<<endl;
	
	while((m_jobToDoQueue.size() == m_maxJobQueueLen) && (m_threadpoolIsClosed == false))
	{	
//		cout<<"In threadpool_add_job in while(cond)"<<endl;
		pthread_cond_wait(&m_jobQueueIsNotFull, &m_jobQueueMutex);
	}

	if(m_threadpoolIsClosed == true)
	{
		pthread_mutex_unlock(&m_jobQueueMutex);
		return -1;
	}
	
	m_jobToDoQueue.push(job);
	if(m_jobToDoQueue.size() == 1)
	{
		pthread_cond_broadcast(&m_jobQueueIsNotEmpty);
	}
	pthread_mutex_unlock(&m_jobQueueMutex);

//	cout<<"In threadpool_add_job after unlock"<<endl;
	return 0;
*/
}


template <class T>
int ThreadPool<T>::threadpool_add_job(const T& job)
{
	if(m_needCheckWhenAddJob == false)
	{
		return threadpool_add_job_no_check(job);
	}
	else
	{
		return threadpool_add_job_check_dup(job);
	}
}

