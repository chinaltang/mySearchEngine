/*******************************Copyright (c)*********************************************
**Copyright Owner:	Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:		chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           CrawlerTest.cpp
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Test example of the class Crawler.
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


#include "Crawler.h"

int main()
{
	Crawler* pCrawler = Crawler::crawler_get_instance(20, 10000); //Create a crawler with a thread pool including 20 threads, and its job queue can store 10000 urls waiting to be crawled at most.
	
	pCrawler->crawler_add_url("http://www.boost.org/");   //Add a good url seed for test
	pCrawler->crawler_add_url("http://www.bad-url.net/"); //Add a non-existed url seed for test
		
//alarm(1000); The crawler progress will be aborted after 1000 seconds.
//Or call pCrawler->crawler_exit() to exit the crawler actively, whenever you want. 
//Here for test just using while(1) to let the crawler keep crawling.
	while(1);
	
	return 0;
}
