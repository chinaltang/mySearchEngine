/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           Crawler.cpp
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Definition of the class Indexer
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

#include "Indexer.h"

Indexer::Indexer(const string& crawledDir, const bool& isPureEnglishIndex):m_crawledDir(crawledDir)//,m_isPureEnglishIndex(isPureEnglishIndex)
{
//	get_time_string(m_indexedDir);
//	m_indexedDir.append("_indexed");
}


int Indexer::get_doc_set(const string& crawledDir, DOC_SET& docSet)
{
	string savedUrlLogFile;

	if(*crawledDir.end() == '/')
	{
		savedUrlLogFile.append(crawledDir).append("saved_url_list.lst");
	}
	else
	{
		savedUrlLogFile.append(crawledDir).append("/").append("saved_url_list.lst");
	}

	fstream fsUrlList;
	fsUrlList.open(savedUrlLogFile.c_str(), ios_base::in);
	if(fsUrlList.fail())
	{
		fsUrlList.close();
		cout<<savedUrlLogFile<<" Open Failed"<<endl;
		
		return -1;
	}
	
	string tmpStringLen;
	string tmpUrl;
	string tmpFileName;
	size_t docId = 0;
	while(getline(fsUrlList, tmpStringLen))
	{
//		stringstream tmpStreamLen(tmpStringLen);
		stringstream tmpStreamLen(tmpStringLen);
		if(!(tmpStreamLen >> tmpUrl))
		{
			cout<<savedUrlLogFile<<" Format Error, Can't Get the URL" <<endl;
			continue;
		}
		if(!(tmpStreamLen >> tmpFileName))
		{
			cout<<savedUrlLogFile<<" Format Error, Can't Get the FileName" <<endl;
			continue;
		}

		++docId;
		docSet.insert(pair<DOC_ID, pair<DOC_URL, DOC_FILENAME> >(docId, pair<DOC_URL, DOC_FILENAME>(tmpUrl, tmpFileName)));
	}

	return docId;
}

int Indexer::get_text_from_html(const string& fileAbsolutePath, string& text)
{
	string fileContent;
	fstream fsFileContent;
	fsFileContent.open(fileAbsolutePath.c_str(), ios_base::in);
	
	if(fsFileContent.fail())
	{
		fsFileContent.close();
		cout<<fileAbsolutePath<<" Open Failed"<<endl;
		
		fsFileContent.close();
		return -1;
	}

	istreambuf_iterator<char> beginIter(fsFileContent), endIter;
	fileContent = string(beginIter, endIter);
	
//	int fileLength = 0;
//	fsFileContent.seekg(0, fsFileContent.end);
//	fileLength = fsFileContent.tellg();
//	fsFileContent.seekg(0, fsFileContent.beg);

	string tagBefore, tagAfter;
	size_t posTagBefore, posTagAfter, posStart = 0;

	if((posTagBefore = find_html_tag(fileContent, tagBefore, posStart)) == string::npos)
	{
		return -1;
	}
	
	posStart = posTagBefore + tagBefore.size();

	while((posTagAfter = find_html_tag(fileContent, tagAfter, posStart)) != string::npos)
	{
		if(posTagAfter - posTagBefore >= tagBefore.size() + 1 && tagBefore.find("<script") == string::npos && tagBefore.find("<style") == string::npos)
		{
			string textSegment = fileContent.substr(posTagBefore + tagBefore.size(), posTagAfter - (posTagBefore + tagBefore.size()));
			text.append(textSegment).append(" ");
		}

		posTagBefore = posTagAfter;
		tagBefore = tagAfter;
		posStart = posTagBefore + tagBefore.size();
	}
	
	fsFileContent.close();
	return 0;
}

int Indexer::create_index(const string& crawledDir, const DOC_SET& docSet, POSTING_INDEX& postingIndex)
{
	string tmpCrawledDir = crawledDir;
	if(*(tmpCrawledDir.end() - 1) != '/')
	{
		tmpCrawledDir.append("/");
	}
	
	boost::char_separator<char> sep(",.-|@\"\"\'\' \n");
	DOC_SET::const_iterator docSetIter = docSet.begin();
	while(docSetIter != docSet.end())
	{
		string tmpFileAbsolutePath;
		tmpFileAbsolutePath.append(tmpCrawledDir).append((docSetIter->second).second);
		
		cout<<tmpFileAbsolutePath<<endl;

		string docText;
		if(get_text_from_html(tmpFileAbsolutePath, docText) < 0)
		{
			docSetIter++;
			continue;
		}
		
		boost::tokenizer<boost::char_separator<char> > tokens(docText, sep);

		for(boost::tokenizer<boost::char_separator<char> >::iterator tokIter = tokens.begin(); tokIter != tokens.end(); ++tokIter)
		{
			POSTING_INDEX::iterator postingIndexIter = postingIndex.find(*tokIter);
			if(postingIndexIter != postingIndex.end()) //Not a new word
			{
				POSTING_LIST::iterator postingListIter = (postingIndexIter->second).find(docSetIter->first);
				if(postingListIter != (postingIndexIter->second).end()) //Not a new word and Not a new doc, just modify the TF
				{
					(postingListIter->second)++;
				}
				else
				{
					(postingIndexIter->second).insert(POSTING_ITEM(docSetIter->first, 1)); //Not a new word, but it appear in a new doc, add new posting item
				}
				
			}
			else //A new word
			{
				POSTING_LIST tmpPostingList;
				tmpPostingList.insert(POSTING_ITEM(docSetIter->first, 1));
				postingIndex.insert(pair<WORD, POSTING_LIST>(*tokIter, tmpPostingList));
			}
		}//end for
//		if(m_isPureEnglishIndex)
		
		docSetIter++;
	}//end while
	
	return 0;
}

void Indexer::get_time_string(string& timeStr)
{
	time_t t;
	time(&t);
	timeStr = string(ctime(&t));

	string::iterator iter = timeStr.begin();
	while(iter != timeStr.end())
	{
		if(*iter == ' ' || *iter == ':')
		{
			*iter = '_';
		}
		else if(*iter == '\n')
		{
			iter = timeStr.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

size_t Indexer::find_html_tag(const string& pageContent, string& tag, const size_t position)
{
	size_t tagStart, tagEnd;
	if((tagStart = pageContent.find('<', position)) == string::npos)
	{
		return string::npos;
	}
	
	if((tagEnd = pageContent.find('>', tagStart + 1)) == string::npos)
	{
		return string::npos;
	}

	tag = pageContent.substr(tagStart, tagEnd - tagStart + 1);

	return tagStart;
}

/*
inline unsigned int get_doc_id(const DOC_ID& doc_id)
{
	return doc_id&0x00FFFFFF;
}

inline unsigned int increase_tf(DOC_ID& doc_id)
{
	doc_id = doc_id + 0x01000000;
	return doc_id;
}
*/


int Indexer::start_index()
{
	if(get_doc_set(m_crawledDir, m_docSet) < 0)
	{
		return -1;
	}
	
	return	create_index(m_crawledDir, m_docSet, m_postingIndex);
}

int Indexer::save_index(const string& indexedDir)
{
	string tmpIndexDir = indexedDir;
	if(*(tmpIndexDir.end() - 1) != '/')
	{
		tmpIndexDir.append("/");
	}
	
	string timeStr;
	get_time_string(timeStr);
	string postingIndexFileAbsolutePath;
	postingIndexFileAbsolutePath.append(tmpIndexDir).append(timeStr).append("_indexed").append(".idx");

	string docSetFileAbsolutePath;
	docSetFileAbsolutePath.append(tmpIndexDir).append(timeStr).append("_indexed").append(".dst");

	fstream fsPostingIndex;
	fstream fsDocSet;

	fsPostingIndex.open(postingIndexFileAbsolutePath.c_str(), ios_base::out);
	if(fsPostingIndex.fail())
	{
		fsPostingIndex.close();
		cout<<postingIndexFileAbsolutePath<<" Open Failed"<<endl;

		return -1;
	}

	fsDocSet.open(docSetFileAbsolutePath.c_str(), ios_base::out);
	if(fsDocSet.fail())
	{
		fsDocSet.close();
		cout<<docSetFileAbsolutePath<<" Open Failed"<<endl;

		return -1;
	}

	boost::archive::text_oarchive oaPostingIndex(fsPostingIndex);
	boost::archive::text_oarchive oaDocSet(fsDocSet);

	oaPostingIndex<<m_postingIndex;
	oaDocSet<<m_docSet;

	fsPostingIndex.close();
	fsDocSet.close();

	return 0;
}

int Indexer::load_index(const string& idxFileAbsolutePath, const string& dstFileAbsolutePath)
{
	postingIndexFileAbsolutePath = idxFileAbsolutePath;
	docSetFileAbsolutePath = dstFileAbsolutePath;
	
	fstream fsPostingIndex;
	fstream fsDocSet;

	fsPostingIndex.open(postingIndexFileAbsolutePath.c_str(), ios_base::in);
	if(fsPostingIndex.fail())
	{
		fsPostingIndex.close();
		cout<<postingIndexFileAbsolutePath<<" Open Failed"<<endl;

		return -1;
	}

	fsDocSet.open(docSetFileAbsolutePath.c_str(), ios_base::in);
	if(fsDocSet.fail())
	{
		fsDocSet.close();
		cout<<docSetFileAbsolutePath<<" Open Failed"<<endl;

		return -1;
	}

	boost::archive::text_iarchive oaPostingIndex(fsPostingIndex);
	boost::archive::text_iarchive oaDocSet(fsDocSet);

	oaPostingIndex>>m_postingIndex;
	oaDocSet>>m_docSet;

	fsPostingIndex.close();
	fsDocSet.close();

	return 0;
}


Indexer::POSTING_INDEX::iterator Indexer::do_query(const string& keyWord)
{
	POSTING_INDEX::iterator postingIndexIter = m_postingIndex.find(keyWord);
//	if(postingIndexIter == m_postingIndex.end())
	
	return postingIndexIter;
}


int Indexer::query(const string& keyWord) //This user interface can be redefine to support different indexing method, such as simple bool methnd.
{
	POSTING_INDEX::iterator postingIndexIter = do_query(keyWord);

	if(postingIndexIter == m_postingIndex.end())
	{
		return -1;
	}
	
	POSTING_LIST::iterator postingListIter;
	DOC_SET::iterator docSetIter;

	cout<<"Qeury Result: The following URL contained the key word -"<<keyWord<<endl;
	
	size_t countUrl = 0;
	for(postingListIter = (postingIndexIter->second).begin(); postingListIter != (postingIndexIter->second).end(); postingListIter++)
	{
		docSetIter = m_docSet.find(postingListIter->first);

		countUrl++;
		cout<<countUrl<<": "<<(docSetIter->second).first<<endl;
	}

	return (postingIndexIter->second).size();
}


int Indexer::load_index(const string& indexedDir)
{
//To be realized
	cout<<"Not realized"<<endl;
/*
	string tmpIndexDir = indexedDir;
	if(*(tmpIndexDir.end() - 1) != '/')
	{
		tmpIndexDir.append("/");
	}

	string timeStr;
	get_time_string(timeStr);
	string postingIndexFileAbsolutePath;
	postingIndexFileAbsolutePath.append(tmpIndexDir).append(timeStr).append("_indexed").append(".idx");

	string docSetFileAbsolutePath;
	docSetFileAbsolutePath.append(tmpIndexDir).append(timeStr).append("_indexed").append(".dlt");
*/
	return -1;	
}