/**
 * @file parser.h
 * Originally for CS 225 Honors Fall 2011
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <string>
#include <set>

#include "textfile.h"

using std::string;
using std::set;

/**
 * Provides a Java-like file parsing utiltity.
 */
class Parser {
public:

	/**
	 * Parser ValidChars Constructor - creates parsing rules based on groups of characters
     *
     * filename - which file to open for parsing
     * valid_chars - a string of characters that could be in a token
     * starting_chars - a string of characters that may occur at the beginning of a token
     * ending_chars - a string of characters that may occur at the end of a token
     *
     * Note: it is assumed that starting_chars and ending_chars are (possibly equal) subsets
     *   of valid_chars. If not, behavior of Parser is undefined.
     */
	Parser(string filename, string valid_chars, string starting_chars, string ending_chars);

	/**
     * Parser Delimiter Constructor - creates parsing rules based on delimiters.
     *
     * Internally, this is translated into parsing rules based on groups of 
     *   valid characters like the other Parser constructor
     */
	Parser(string filename, string delimiters);

	/**
     * Returns whether the current Parser object has a valid file to parse
     */
	bool isValid() const;
	
	/**
     * Returns true if this Parser has another token in its input.
     *
     * The Parser does not advance past any input. 
     */
	bool hasNext() const;

	/**
     * Returns true if this Parser has another line in its input.
     *
     * The Parser does not advance past any input. 
     */
	bool hasNextLine() const;

	/**
     * Finds and returns the next complete token from this Parser
     */
	string next();

	/**
     * Finds and returns the next complete line from this Parser,
     *   disregarding any previous tokens, even if they would be on this line.
     *
     * A line is defined as a sequence of tokens delimited by a newline character.
     */
	string nextLine();

	/**
     * Returns the Parser to the beginning of the file
     */
	void reset();

	/**
     * The destructor closes the input file if it is still open as well as normal memory duties
     */
	virtual ~Parser();

	/**
     * Assigns the current parser the state of another parser
     */
	const Parser & operator=(const Parser & other);

	/**
     * Copy constructor - assigns the current parser the state of another parser
     */
	Parser(const Parser & other);

private:

	TextFile* textfile;

	size_t filesize;
    size_t cursor;
	string token;
    char* text;
	bool valid;

	set<char> valid_charset;
	set<char> starting_charset;
	set<char> ending_charset;

	bool initTextFile(string filename);
	void initSetsViaValid(string valid_chars, string starting_chars, string ending_chars);
	void initSetsViaDelims(string delimiters);

    void findNextToken();
    void trim();
	string finish();

    bool startable(char ch);
    bool endable(char ch);
    bool tokenable(char ch);

	void copy(const Parser & other);
	void clear();
};

#endif
