/*******************************Copyright (c)*********************************************
**Copyright Owner:	Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:		chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           Crawler.h
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Declaration of the class Crawler
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

#ifndef _CRAWLER_HEAD_H
#define _CRAWLER_HEAD_H


#include <stdio.h>
#include <time.h>
#include <ThreadPool.h>
#include <boost/asio.hpp>
//#include <boost/system.hpp>
#include <boost/thread.hpp>
#include <sys/stat.h>


using namespace std;

//Singleton Patterns. Using thread pool to realize parallel processing.
class Crawler 
{
public:
	virtual ~Crawler();
	int crawler_exit();
	
	//Return the unique pointer to the crawler instance.
	static Crawler* crawler_get_instance(size_t threadNum = 20, size_t maxUrlQueueLen = 10000); 
	
	//User interface for adding url seeds.
	int crawler_add_url(const string& url);   
	
	//Overload version
	int crawler_add_url(const list<string>& urlList); 

private:
	//Not need to be static.
	ThreadPool<string> m_crawlerThreadPool;
	
	//A flag mark the crawler has been exited or not.
	static bool m_crawlerIsExited;  
	
	//A mutex for the list file of saved pages.
	static pthread_mutex_t m_savedUrlLogFileMutex;  
	
	//The dest directory for storing the pages.
	static string m_destDirectory;  
	
	//The name of the list file of saved pages.
	const static string m_savedUrlLogFileName;    

	//Private constructor for singleton.
	Crawler(ThreadPool<string>::CALLEE_FUN userCallee, size_t threadNum = 20, size_t maxJobQueueLen = 10000); 
		
	//Private copy constructor.
	Crawler(const Crawler&) {}; 
	
	//Private assignment.
	Crawler operator=(const Crawler&) {}; 
	
	//The main loop to perform crawling and extract urls.
	static void* crawler_main_loop(string& url, ThreadPool<string> *pool); 
	
	//Crawl the page and store the page content in pageContent.
	static int crawler_crawl_page(const string& url, string& pageContent); 
	
	//Extract the urls from page content. And the urls will be devided into two types basic categories: internal link and external link.
	static int crawler_extract_url(const string& url, const string& pageContent, list<string> &externalUrlList, list<string>& internalUrlList);
	
	//Save the page into local file.
	static int crawler_save_page(const string& destDirectory, const string& savedUrlLogFileName, const string& url, const string& pageContent);
	
	//Internal interface for adding the urls which need to be crawled.
	static int crawler_add_url(ThreadPool<string>* pool, const string& url);
	
	//Overload version.
	static int crawler_add_url(ThreadPool<string>* pool, const list<string>& urlList);
	
	//Convert the current time into a string with format like Sat_Aug_24_15_48_12_2013
	static void get_time_string(string& timeStr);
	
	//Get protocol, domain and path information from a url.
	static int parse_url(const string& url, string& protocol, string& domain, string& path);
	
	//Convert the url into a valid local file name.
	static void convert_url_to_file_name(const string& url, string& fileName);
	
	//Like the function parse_url, but just return the domain information.
	static string get_domain_from_url(const string& url);
	
	//Analyze the extracted url link which marked by html tag "href=", and then create a standard url accord the url of current page. 
	static int joint_url(const string& currentPageLink, const string& jumpPageLink, string& jointedUrl);
};

#endif
