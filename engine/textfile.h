/*	Part of CS 296-25 Honors project Spring 2011
 *	textfile.* - allows programming with (hopefully) fewer I/O bottlenecks
 */

#ifndef _TEXTFILE_H_
#define _TEXTFILE_H_

#include <iostream>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using std::string;

class TextFile {

public:

	TextFile(string title);

	char* opentext();
	unsigned int get_size() const;
	string get_title() const;
	bool closetext();

private:

	string _title;
	char* start;
	unsigned int size;
	int file_descriptor;

};

#endif
