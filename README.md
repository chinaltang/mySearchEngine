1 General Description:
This is an personal search engine developed with C++. And used some feature of boost, like asio, tokenizer, serialization. This search engin support some simple but important features like pages crawling, posting index establishing and key word querying. And I also specially developed a simple and reliable thread pool which is class template supporting different types of jobs. More features will developed in the future, as I learn more on search engine and related natural language processing. 

2 Files Description:
PART I. 
	Source files of the thread pool, crawler and Indexer.
--ThreadPool.h
--ThreadPool.cpp
--Crawler.h
--Crawler.cpp
--Indexer.h
--Indexer.cpp
PART II
	Test example files of the thread pool, crawler and Indexer.
--ThreadPoolTest.cpp
--CrawlerTest.cpp
--IndexerTest.cpp
PART III
	Executable binary files. If you have not installed the needed boost library, it may not run as expected.
--CrawlerTest
--IndexerTest
PART IV
	Output files of the crawler and indexer during run. Including a depository of pages crawled from www.boost.org, serialized posting index(.idx) and doc set(.dst).
--Sat_Aug_24_15_48_12_2013_crawled
--Sat_Aug_31_21_02_09_2013_indexed.idx
--Sat_Aug_31_21_02_09_2013_indexed.dst


3 Build Description:
Because this program used some feature of boost, like asio, tokenizer, serialization. So if you want to build the source code you may need install a boost library first. The boost version used in the developing is 1.54.0. 
Here just give the simple g++ command line when build the three examples for reference.

//ThreadPoolTest.cpp
g++ ThreadPool.h ThreadPoolTest.cpp -o ThreadPoolTest -lpthread

//Note that threadPool.h is not a ordinary header file. It's a combination source file of class template.

//CrawlerTest.cpp
g++ Crawler.cpp CrawlerTest.cpp -o CrawlerTest -lpthread -lboost_system -lboost_regex -lboost_exception -lboost_thread

//IndexerTest.cpp
g++ Indexer.cpp IndexerTest.cpp -o IndexerTest -lboost_serialization
