/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           Crawler.cpp
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Definition of the class Crawler
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

Crawler::Crawler(ThreadPool<string>::CALLEE_FUN userCallee, size_t threadNum, size_t maxUrlQueueLen):m_crawlerThreadPool(userCallee, true, threadNum, maxUrlQueueLen)
{
	pthread_mutex_init(&m_savedUrlLogFileMutex, NULL);
	get_time_string(m_destDirectory);
	m_destDirectory.append("_crawled"); //Get the workspace path name.
}


Crawler::~Crawler()
{
	if(m_crawlerIsExited == false)
		crawler_exit();

}


int Crawler::crawler_exit()
{
	if(m_crawlerIsExited == true)
	{
		return -1;
	}
	m_crawlerIsExited = true;
	
	m_crawlerThreadPool.threadpool_destroy();

	pthread_mutex_destroy(&m_savedUrlLogFileMutex);
	return 0;
}


pthread_mutex_t Crawler::m_savedUrlLogFileMutex = PTHREAD_MUTEX_INITIALIZER;
//const string Crawler::m_savedUrlLogFileName = string("saved_url_list.txt");
const string Crawler::m_savedUrlLogFileName("saved_url_list.lst");
bool Crawler::m_crawlerIsExited = false; //Definnation and Initialization.
string Crawler::m_destDirectory; //Only Definition.


Crawler* Crawler::crawler_get_instance(size_t threadNum, size_t maxUrlQueueLen)
{
	if(m_crawlerIsExited == true)
	{
		cout<<"The crawler instance has exited. You may need restart your program to get a new instance"<<endl;
		return NULL;
	}
	static Crawler crawlerInstance(crawler_main_loop, threadNum, maxUrlQueueLen);
	return &crawlerInstance;
}


int Crawler::crawler_add_url(const string &url)
{
	return m_crawlerThreadPool.threadpool_add_job(url);
}


int Crawler::crawler_add_url(const list<string> &urlList)
{
	list<string>::const_iterator iter = urlList.begin();
	int rtn = 0;
	while(iter != urlList.end())
	{
		rtn += crawler_add_url(*iter);
		iter++;
	}
	return rtn;
}


int Crawler::crawler_add_url(ThreadPool<string>* pool, const string &url)
{
	return pool->threadpool_add_job(url);
}


int Crawler::crawler_add_url(ThreadPool<string>* pool, const list<string> &urlList)
{
	list<string>::const_iterator iter = urlList.begin();
	int rtn = 0;
	while(iter != urlList.end())
	{
		rtn += crawler_add_url(pool, *iter);
		iter++;
	}
	return rtn;
}


int Crawler::crawler_crawl_page(const string& url, string& pageContent)
{
	using namespace boost::asio; //name space
	using ip::tcp;

	std::string protocol, domain, path;
	
	if(parse_url(url, protocol, domain, path) < 0)
	{
		std::cout<<"parse_url failed: "<<url<<std::endl;
		return -1;
	}
	boost::asio::io_service io_service; //the object has the same name with class. This is a boost style.
	boost::system::error_code ec;

	tcp::resolver resolver(io_service);
	tcp::resolver::query query(domain.c_str(), protocol.c_str());
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query, ec);

	if(ec) //And error occured, when ec is non-zero.
	{
		std::cout<<boost::system::system_error(ec).what()<<std::endl;
		return -1;
	}

	tcp::socket socket(io_service);
	
	connect(socket, endpoint_iterator, ec);
	
	if(ec) //And error occured, when ec is non-zero.
	{
		std::cout<<boost::system::system_error(ec).what()<<std::endl;
		return -1;
	}

	std::cout<<"Crawl Start 3"<<url<<std::endl;
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream<<"GET "<<path<<" HTTP/1.0\r\n";
	request_stream<<"Host: " <<domain<<"\r\n";
	request_stream<<"Accept: */*\r\n"; //All file type is ok.
	request_stream<<"Connection: close\r\n\r\n";
	
	write(socket, request);
	//	ec = 0; //Reinitialize the error code.
	
	
	boost::asio::streambuf response;
	read_until(socket, response, "\r\n", ec);

	if(ec)
	{
		std::cout<<boost::system::system_error(ec).what()<<std::endl;
		return -1;
	}
	
	std::istream response_stream(&response); //Don't forget the "&", the parameter type is pointer.
	std::string http_version;
	response_stream>>http_version;
	unsigned int status_code;
	response_stream>>status_code;
	std::string status_message;
	std::getline(response_stream, status_message);
	if(!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		std::cout << "Invalid response." << std::endl;
		return -1;
	}

	if(status_code != 200)
	{
		std::cout << "Response return with unexpected status code "<< status_code << std::endl;
		return -1;
	}
	
	read_until(socket, response, "\r\n\r\n", ec);

	if(ec)
	{
		std::cout<<boost::system::system_error(ec).what()<<std::endl;
		return -1;
	}
	
	std::string header;
	std::stringstream pageContentStream;
	while(std::getline(response_stream, header) && header != "\r")
	{
//		std::cout<<header<<std::endl;
	}
//	exit(-1);
	while(read(socket, response, transfer_at_least(1), ec))
	{
//	if(ec && (ec != error::eof))
		if((ec != error::eof))
		{
		//	break;
			std::cout<<boost::system::system_error(ec).what()<<std::endl;
		//	return -1;
		}
		pageContentStream<<&response;
//		std::cout<<&response;
//		response_stream>>pageContent;
//		while(response_stream>>pageContent);
	}

	pageContent = pageContentStream.str();
	std::size_t pageSize = pageContent.size();

	std::cout<<"Crawled: "<<url<<" size: "<<pageSize<<" Successfully."<<std::endl;
	
	return pageSize;
}


int Crawler::crawler_extract_url(const string& url, const string& pageContent, list<string>& externalUrlList, list<string>& internalUrlList)
{
	size_t posLinkMark, posLeftQuotation, posRightQuotation, posStart = 0;
	const size_t numAllowedSpace = 2;
	
	size_t urlLen = 0;
	size_t urlCount = 0;
	size_t posTmp;
	string strTmp;

	if((posTmp = url.find("://")) == string::npos)
	{
		return -1;
		cout<<"1 return"<<endl;
	}
	
	if( (*(url.end() - 1) != '/') && ((posTmp = url.substr(posTmp + 3).rfind('/')) != string::npos))
	{
		if(posTmp = (url.substr(posTmp + 1).rfind('.')) != string::npos)
		{
			strTmp = url.substr(posTmp + 1);
			if(strTmp.size() <= 4)
			{	
				if(strTmp != string("html") && strTmp != string("htm") &&  \
					strTmp != string("php") && strTmp != string("jsp") &&  \
						strTmp != string("aspx") && strTmp != string("asp") && strTmp != string("cgi"))
				{
					cout<<"2 return"<<endl;
					return -1;
				}
			}
		}
	}

	while(1)
	{
		cout<<"Extract URL start!"<<endl;
		if((posLinkMark  = pageContent.find("href=", posStart)) == string::npos)
		{
			cout<<"Extract Error!"<<endl;
			break;
		}

		posLeftQuotation = pageContent.find('\"', posLinkMark + 5);
	
		if(posLeftQuotation == string::npos)
		{

			cout<<"Extract Error!"<<endl;
			break;
		}
		
		if(posLeftQuotation - (posLinkMark + 5) > numAllowedSpace)
		{
			posStart = posLinkMark + 5;
			continue;
		}
	
		posRightQuotation = pageContent.find('\"', posLeftQuotation + 1);
		
		if(posRightQuotation == string::npos)
		{
			break;
		}
	
		if((posRightQuotation - posLeftQuotation) == 1)   //href=""
		{
			posStart = posRightQuotation + 1;
			continue;
		}
	
		string urlTmp;
		urlLen = posRightQuotation - 1 - (posLeftQuotation + 1) + 1;  //When href="/path/file", urlTmp = /path/file, don't involve the quotations
		urlTmp = pageContent.substr(posLeftQuotation + 1, urlLen);

		string jointedUrl;
		int rtn = joint_url(url, urlTmp, jointedUrl);
		if(rtn == 0)
		{
			internalUrlList.push_back(jointedUrl);
			cout<<"Get an internal url: "<<jointedUrl<<endl;
			urlCount++;

		}
		else if(rtn == 1)
		{
			externalUrlList.push_back(jointedUrl);
			cout<<"Get an external url: "<<jointedUrl<<endl;
			urlCount++;
		}
		else
		{
			;
		}

		posStart = posRightQuotation + 1;
	}//end while(1)
	
	cout<<"Extracted "<<urlCount<<" "<<"from "<<url<<"."<<endl;
	return urlCount;
}


int Crawler::crawler_save_page(const string& destDirectory, const string& savedUrlLogFileName, const string& url, const string& pageContent)
{
	static bool isDestDirectoryCreated = false;
//	static bool isSavedUrlLogFileCreated = false;
//	fstream fsPageContent, fsSavedUrlLog;

	if(!isDestDirectoryCreated) //Create the work directory. Will be created only once.
	{
		if(mkdir(destDirectory.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) < 0 )
		{
			if(errno != EEXIST)
			{
				cout<<"Create Directory: "<<destDirectory<<"Failed!"<<endl;
				cout<<"Error Code is "<<errno<<": "<<strerror(errno)<<endl;
				return -1;
			}
		}
		else
		{
			isDestDirectoryCreated = true;
		}
	}
	

	fstream fsPageContent;
	string pageContentFileName, pageContentFileRelativePath;
	
	convert_url_to_file_name(url, pageContentFileName);
	pageContentFileRelativePath.append("./").append(destDirectory).append("/").append(pageContentFileName);
	fsPageContent.open(pageContentFileRelativePath.c_str(), ios_base::out);
	
	if(fsPageContent.fail())
	{
//	fsPageContent.clear();
		fsPageContent.close();
		cout<<pageContentFileName<<" Open Failed"<<endl;
		
		return -1;
	}
	else
	{
		fsPageContent.write(pageContent.c_str(), pageContent.size());
		if(fsPageContent.fail())
		{
			cout<<pageContentFileName<<" Write Failed"<<endl;
			fsPageContent.close();

			return -1;
		}
		fsPageContent.close();
	}
	
	pthread_mutex_lock(&m_savedUrlLogFileMutex);
	
	fstream fsSavedUrlLog;
	string savedUrlLogFileRelativePath;
	savedUrlLogFileRelativePath.append("./").append(destDirectory).append("/").append(savedUrlLogFileName);
	
	fsSavedUrlLog.clear();
	fsSavedUrlLog.close();

	fsSavedUrlLog.open(savedUrlLogFileRelativePath.c_str(), ios_base::out|ios_base::app); //To be investigated! The mode and the return value as well as the error indications.
		
	if(fsSavedUrlLog.fail())
	{
		cout<<savedUrlLogFileName<<" Open Failed"<<endl;

		fsSavedUrlLog.clear();
		fsSavedUrlLog.close();
		return -1;
	}
	else
	{
		fsSavedUrlLog<<url<<" "<<pageContentFileName<<" "<<pageContent.size()<<endl;
		
		fsSavedUrlLog.clear();
		fsSavedUrlLog.close();
	}

	pthread_mutex_unlock(&m_savedUrlLogFileMutex);
	
	return 0;
//	return pageContent.size();
}


void* Crawler::crawler_main_loop(string& url, ThreadPool<string> *pool)
{
	string pageContent;
	if(crawler_crawl_page(url, pageContent) < 0)
	{
		return (void*)(-1);
	}

	if(crawler_save_page(m_destDirectory, m_savedUrlLogFileName, url, pageContent) < 0)
	{
		return (void*)(-1);
	}
	
	list<string> externalUrlList, internalUrlList;

	if(crawler_extract_url(url, pageContent, externalUrlList, internalUrlList) < 0 )	
	{
		return (void*)(-1);
	}

	if(crawler_add_url(pool, internalUrlList) < 0) //Note that external urls will NOT be added. 
	{
		return (void*)(-1);
	}
}


void Crawler::get_time_string(string& timeStr)
{
	time_t t;
	time(&t);
	timeStr = string(ctime(&t));//The returned char* has an unexpected charactor '\n', so we need erase it.
	string::iterator iter = timeStr.begin();
	while(iter != timeStr.end())
	{
		if(*iter == ' ' || *iter == ':')
		{
			*iter = '_';
			iter++;
		}
		else if(*iter == '\n')
		{
			iter = timeStr.erase(iter); //Erase the '\n', and the iterator will automatically move aheead. Bye the way, the size or the length ot the string will also changed automatically after the erasing.
		}
		else
		{
			iter++;
		}
	}
	
}


int Crawler::parse_url(const string& url, string& protocol, string& domain, string& path)
{
	size_t posFound, posStart;

	posFound = url.find("://");
	
	if(posFound == string::npos || posFound == 0) 
	{	
		cout<<"Bad url format, can't get the protocol!"<<endl;
		return -1;
	}
	
	protocol = url.substr(0, posFound);
	posStart = posFound + 3;
	
	if((protocol.size() + 3) >= url.size())
	{
		cout<<"Bad url format, can't get the domain!"<<endl;
		return -1;
	}
	
	posFound = url.find('/', posStart);
	
	if(posFound == posStart) //When the url is http:////aa
	{
		cout<<"Bad url format, can't get the domain!"<<endl;
		return -1;
	}
	else if(posFound == string::npos)
	{
		domain = url.substr(posStart, url.size() - posStart);
	}
	else
	{
		domain = url.substr(posStart, posFound - posStart);	
	}

	posStart = posFound;

	if((protocol.size() + 3 +  domain.size()) >= url.size())
	{
		path = string("/");
	}
	else 
	{
		size_t lenWithoutPath = protocol.size() + 3 + domain.size();
		path = url.substr(lenWithoutPath, url.size() - lenWithoutPath);
	}

	cout<<"In parse fun, the url is: "<<url<<" protocol is "<<protocol<<" domain is "<<domain<<" path is "<<path<<endl;
	return 0; 
}


void Crawler::convert_url_to_file_name(const string& url, string& fileName)
{
	const size_t row = 4;
	char substitute[row][2] = {{'/', '^'}, {'?', '['}, {'&', ']'}, {':', '@'}};
	
	fileName = url;

	string::iterator iter = fileName.begin();

	while(iter != fileName.end())
	{
		for(int i = 0; i < 4; i++)
		{
			if(*iter == substitute[i][0])
			{
				*iter = substitute[i][1];
				break;
			}
		}
		iter++;
	}
}


string Crawler::get_domain_from_url(const string& url)
{
	string protocol, domain, path;
	parse_url(url, protocol, domain, path);
	
	return domain; 
}


int Crawler::joint_url(const string& currentPageLink, const string& jumpPageLink, string& jointedUrl)
{
	//Internal link return 0
	//External link return 1
	//Error -1
	if((jumpPageLink.find("mailto:") != string::npos) || (jumpPageLink.find("javascript:") != string::npos))  //href="" or href="mailto:" or href="javascript:"
	{
		return -1;
	}

	if(jumpPageLink.find("://") != string::npos) //This is an complete link with protocol feild.
	{
		if(get_domain_from_url(currentPageLink) != get_domain_from_url(jumpPageLink))
		{
			jointedUrl = jumpPageLink;
			return 1;

		}
		else
		{
			jointedUrl = jumpPageLink;
			return 0;
		}

	}
	else
	{
		if(*(jumpPageLink.begin()) == '/')                                       //jumpPageLink is like /index.html. It is an absolute path from root.
		{
			size_t posTmp = 0;
			if((posTmp = currentPageLink.find("://")) == string::npos)  
			{
				return -1;
			}
			if((posTmp = currentPageLink.find("/", posTmp + 3)) == string::npos) //currentPageLink is like http://www.sina.com
			{
				jointedUrl.append(currentPageLink).append(jumpPageLink);
				return 0;
			}
			else															   //currentPageLink is like http://www.sina.com/page1/page1_1/
			{
//				cout<<"currentPageLink.substr(0, posTmp) = "<< currentPageLink.substr(0, posTmp)<<" posTmp = "<<posTmp<<endl;
				jointedUrl.append(currentPageLink.substr(0, posTmp)).append(jumpPageLink);           // (posTmp - 1) - 0 + 1 is the length of url with protocol and domain
				return 0;
			}
		}
		else if(*jumpPageLink.begin() == '.')                //jumpPageLink is like ../../../page.html
		{
			size_t posTmp = 0, posStart = 0, backtrackDepth = 0;
			while((posTmp = jumpPageLink.find("../", posStart)) != string::npos)
			{
				++backtrackDepth;
				posStart = posTmp + 3;
			}
			
			size_t posEnd = 0;
			posTmp = currentPageLink.rfind('/');  //Go back tu the current diectory. If jumpPageLink is ../page2, It means from http://www.sina.com/page1/page1_1 to http://www.sina.com/page2
			posEnd = posTmp - 1;

			while(backtrackDepth > 0)
			{
				posTmp = currentPageLink.rfind('/', posEnd);
				if(posTmp == string::npos || currentPageLink[posTmp -1] == '/' || currentPageLink[posTmp + 1] == '/') //Backstrack overstep or the posEnd point to the '/' of "http://" 
				{
					return -1;
				}
				posEnd = posTmp - 1;
				--backtrackDepth;
			}

//		jointedUrl.append(currentPageLink.substr(0, posEnd + 1)).append("/").append(jumpPageLink.substr(posStart));
			jointedUrl.append(currentPageLink.substr(0, posEnd + 2)).append(jumpPageLink.substr(posStart)); //posEnd + 2 = (posEnd + 1)/*point to '/'*/ + 1 /*length from 0 to posEnd + 1*/
			return 0;
			
		}
		else                              //jumpPageLink is like page_another.html
		{
			size_t posTmp = 0;
			posTmp = currentPageLink.rfind('/');
			if(posTmp == string::npos)
			{
				return -1;
			}
			if(currentPageLink[posTmp - 1] == '/')    //currentPageLink is like http://www.sina.com
			{	
				jointedUrl.append(currentPageLink).append("/").append("/").append(jumpPageLink);
				return 0;
			}
			else                        //currentPageLink is like http://www.sian.com/ or http://www.sina.com/page1.html. posTmp point to the last '/'.
			{
				jointedUrl.append(currentPageLink.substr(0, posTmp + 1)).append(jumpPageLink);
				return 0;
			}
		}

	}	
//	return -1;
}
