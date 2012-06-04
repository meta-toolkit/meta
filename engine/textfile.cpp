/*	Part of CS 296-25 Honors project Spring 2011
 *	textfile.* - allows programming with (hopefully) fewer I/O bottlenecks
 */

#define USE_MMAP false

#include <iostream>
#include "textfile.h"

using namespace std;

TextFile::TextFile(string title){

	_title = title;
	start = NULL;
	size = 0;
	file_descriptor = -1;
	
}

char* TextFile::opentext(){

	// get file size
	struct stat st;
	stat(_title.c_str(), &st);
	
	size = st.st_size;
	
	// get file descriptor
	file_descriptor = open(_title.c_str(), O_RDONLY);
	if(file_descriptor < 0){
		cerr << "Error obtaining file descriptor for: " << _title.c_str() << endl;
		return NULL;
	}
	
	// memory map
	start = (char*) mmap(NULL, size, PROT_READ, MAP_SHARED, file_descriptor, 0);
	if(start == NULL){
		cerr << "Error memory-mapping the file\n";
		close(file_descriptor);
		return NULL;
	}
	
	return start;
}

unsigned int TextFile::get_size() const {
	return size;
}

string TextFile::get_title() const {
	return _title;
}

bool TextFile::closetext(){
	
	if( start == NULL){
		cerr << "Error closing file. Was closetext() called without a matching opentext() ?\n";
		return false;
	}
	
	// unmap memory and close file
	bool ret = munmap(start, size) != -1 && close(file_descriptor) != -1;
	start = NULL;
	return ret;
	
}
