/*******************************Copyright (c)*********************************************
**Copyright Owner:			Cory Tang 
**Department:           University of Electronic Science and Technology of China  
**Email:								chong.tang@live.cn
**--------------------------- File Info ---------------------------------------------------
** File name:           IndexerTest.cpp
** Last modified Date:  2013-08-29
** Last Version:        0.1
** Descriptions:        Test example of the class Indexer.
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

int main()
{
	Indexer indexer("Sat_Aug_24_15_48_12_2013_crawled"); //Create a indexer to index the pages in given directory.

	index.start_index();     //Start indexing.
	index.query("boost");    //Perform a query.
	index.save_index("./");  //Save the index related structs into disk files.
}
