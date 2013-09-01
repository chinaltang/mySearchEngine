/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           Crawler.h
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Declaration of the class Indexer
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

#ifndef _INDEXER_HEAD_H
#define _INDEXER_HEAD_H

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <streambuf>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <iterator>
//#include <boost/unordered_map>
//#include <boost/unordered_set>
#include <boost/tokenizer.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

using namespace std;

class Indexer 
{
public:
	typedef string WORD;
	typedef unsigned int WORD_ID;
	typedef unsigned int DOC_ID;
	typedef unsigned char WORD_TF;
	typedef string DOC_URL;
	typedef string DOC_FILENAME;
	typedef DOC_ID POSTING_ITEM_KEY; // Be Const
	typedef WORD_TF POSTING_ITEM_VALUE;  //Customed
//	typedef unsigned char WORD_POS;        //For future reliazation.
//	typedef set<WORD_POS> WORD_POS_SET;    //For future reliazation.
//	typedef pair<WORD_TF, WORD_POS_SET> POSTING_ITEM_VALUE;			//For future reliazation.
	typedef pair<POSTING_ITEM_KEY, POSTING_ITEM_VALUE> POSTING_ITEM;
	typedef map<POSTING_ITEM_KEY, POSTING_ITEM_VALUE> POSTING_LIST; //map<unsigned int, unsigned char>
	typedef map<WORD, POSTING_LIST > POSTING_INDEX;  //map<string, map<unsigned int, unsigned char> >
	typedef map<DOC_ID, pair<DOC_URL, DOC_FILENAME> > DOC_SET; //In order to reduce the memory usage, it not include the absolute path of every doc. 
	
	//Constructor. Because I have not found a effective & open-source Chineese Tokenizer, so the Indexer is designed for indexing pure English content.
	Indexer(const string& crawledDir, const bool& isPureEnglishIndex = true);
	
	//User interface for starting an indexing.
	int start_index();
	
	//Serialize the data structs of posting index and doc set into disk file.
	int save_index(const string& indexedDir);
	
	//Deserialize the data structs of posting index and doc set from disk file.
	load_index(const string& idxFileAbsolutePath, const string& dstFileAbsolutePath);
	
	//Overload version.
	load_index(const string& indexedDir);
	
	//Simple user interface for querying a key word, here the "key word" must be single word at the moment. 
	//But it is not hard to support combination query known as bool model. This feature will be reliazed in the near future.
	int query(const string& queryStr);

private:	
//const bool m_isPureEnglishIndex; //default true;
	
	//The directory contains the crawled pages and the list file.
	string m_crawledDir;
	
	//The object of posting index.
	POSTING_INDEX m_postingIndex;
	
	//The object of doc set.
	DOC_SET m_docSet;
	
	//Private do_query, offer basic query service for query of user interface.
	POSTING_INDEX::iterator do_query(const string& keyWord);
	
	//Get doc set from the crawled directory.
	int get_doc_set(const string& crawledDir, DOC_SET& docSet);
	
	//Get the text from file of html. It means filtering all html tags we don't need.
	int get_text_from_html(const string& file, string& text);
	
	//Establish the posting index.
	int create_index( const string& crawledDir, const DOC_SET& docSet, POSTING_INDEX& postingIndex);

	//Convert the current time into a string with format like Sat_Aug_24_15_48_12_2013
	void get_time_string(string& timeStr);
	
	//Find a html tag and store it into parameter tag. Only the content at and after the given position will be considered.
	size_t find_html_tag(const string& pageContent, string& tag, const size_t position);
		
};

#endif
