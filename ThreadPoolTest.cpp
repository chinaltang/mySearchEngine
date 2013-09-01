/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           ThreadPoolTest.cpp
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Test example of the class ThreadPool.
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


#include "ThreadPool.h"

void* user_callee(int& data, ThreadPool<int> *pool)
{
	pthread_t tid = pthread_self();
	
	cout<<"My thread id is: "<<tid<<endl;   
	cout<<"And my job is compute the square of "<<data<<endl;
	cout<<"The result is "<<data*data<<endl; //The job is done.  
	
	if(tid%10 == 0 || tid%10 == 1)          //According to the different pthread id, the user task can perform differently.
	{
		pool->threadpool_add_job(data+10);    //Adding a job in the user task.
	}
	else
	{
		;	                                    
	}

	sleep(2);
	return NULL;
}

int main()
{
	ThreadPool<int> testPool(user_callee, false, 10, 1000);  //Create a thread pool to perform the user task user_callee. It allow duplicate jobs, and has 10 threads and a job queue whose length is 1000.

	testPool.threadpool_add_job(1); 
	testPool.threadpool_add_job(2);
	testPool.threadpool_add_job(3);
	testPool.threadpool_add_job(4); 
	testPool.threadpool_add_job(5);
	testPool.threadpool_add_job(6);
	
//alarm(1000); The thread pool progress will be aborted after 1000 seconds.
//Or call testPool.threadpool_destroy() to exit the thread pool actively, whenever you want. 
//Here for test just using while(1) to let the thread pool keep running.
	while(1);

	return 0;

}

