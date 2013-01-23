/**
 * @file parser.cpp
 */

#include "parser.h"
#include "textfile.h"

using std::string;
using std::set;

Parser::Parser(string filename, string valid_chars, string starting_chars, string ending_chars){
    if(initTextFile(filename)){
        valid = true;
        initSetsViaValid(valid_chars, starting_chars, ending_chars);
        findNextToken();
    }
}

Parser::Parser(string filename, string delimiters){
    if(initTextFile(filename)){
        valid = true;
        initSetsViaDelims(delimiters);
        findNextToken();
    }
}

bool Parser::isValid() const {
    return valid;
}

bool Parser::hasNext() const {
    return token != "";
}

bool Parser::hasNextLine() const {
    return token != "";
}

string Parser::next() {

    // make sure we're not at the end
    if(token == "")
        return "";
    
    // save return value
    string ret(token);
  
    // find the next token, if there is one
    findNextToken();    

    return ret;
}

string Parser::peek() const {
    return token;
}

void Parser::findNextToken(){

    // if we're dealing with single characters only
    if(valid_charset.size() == 127)
    {
        if(cursor > filesize)
        {
            token = "";
            return;
        }
        token = text[cursor];
        ++cursor;
        return;
    }

    token = "";
    
    while(!startable(text[cursor])){
        // if at the end and still not startable chars,
        //   return and leave token as the empty string
        if(cursor >= filesize - 1)
           return;
        ++cursor;
    }

    // add on as many additional characters as possible
    while(tokenable(text[cursor])){
        token += text[cursor];
        if(cursor == filesize - 1){
            trim();
            return;
        }
        ++cursor;
    }
   
    // remove unendable characters from the end,
    //   leaving the cursor at the end of the last endable character
    trim();
}

void Parser::trim(){
    
    while(!endable(token[token.size()-1])){
        // if we somehow reach the beginning...
        if(cursor == 0 || token == ""){
            token = "";
            throw "Parser Error -- went past beginning\n";
        }
        --cursor;
        token.erase(token.end() - 1);
    }
}

bool Parser::startable(char ch){
    return starting_charset.find(ch) != starting_charset.end();
}

bool Parser::endable(char ch){
    return ending_charset.find(ch) != ending_charset.end();
}

bool Parser::tokenable(char ch){
    return valid_charset.find(ch) != valid_charset.end();
}

string Parser::finish(){
	string ret(token);
	token = "";
	return ret;	
}

string Parser::nextLine(){
	
	// start the next cycle
	if(cursor != filesize)
		++cursor;
	else
		return finish();

	// go as far as possible until the next newline
	while(text[cursor] != '\n'){
		if(cursor == filesize - 1){
			return finish();	
		}
		token += text[cursor];
		++cursor;
}
	
	string ret(finish());
	findNextToken();
	return ret;
}

void Parser::reset(){
    cursor = 0;
    findNextToken();
}

bool Parser::initTextFile(string filename){

    textfile = new TextFile(filename);
    text = textfile->opentext();
    
    if(!text){
        valid = false;
        return false;
    }

    filesize = textfile->get_size();
    token = "";
    cursor = 0;

    return true;
}

void Parser::initSetsViaValid(string valid_chars, string starting_chars, string ending_chars){
  
	for(size_t i = 0; i < valid_chars.size(); ++i)
        valid_charset.insert(valid_chars[i]);
   	
	for(size_t i = 0; i < starting_chars.size(); ++i)
        starting_charset.insert(starting_chars[i]);
	
	for(size_t i = 0; i < ending_chars.size(); ++i)
        ending_charset.insert(ending_chars[i]);
}

// when given only delimiters, we must invert the set of characters
// note how we assume we are dealing with ASCII values
void Parser::initSetsViaDelims(string delimiters){

    set<char> lookup;
    for(size_t i = 0; i < delimiters.size(); ++i)
		lookup.insert(delimiters[i]);

    for(char ch = 0; (int)ch < 127; ++ch){
        if(lookup.find(ch) == lookup.end()){
            valid_charset.insert(ch);
            starting_charset.insert(ch);
            ending_charset.insert(ch);
        }
    }
}

string Parser::getFilename() const {
    return textfile->get_title();
}

Parser::~Parser(){
    clear();
}

const Parser & Parser::operator=(const Parser & other){
    if(this != &other){
        clear();
        copy(other);
    }
    return *this;
}

Parser::Parser(const Parser & other){
    copy(other);
}

void Parser::copy(const Parser & other){
    textfile = new TextFile(*(other.textfile));
    filesize = other.filesize;
    cursor = other.cursor;
    text = textfile->opentext();
    valid = other.valid;
    token = other.token;
	valid_charset = other.valid_charset;
	starting_charset = other.starting_charset;
	ending_charset = other.ending_charset;
}

void Parser::clear(){
    textfile->closetext();
    delete textfile;
    textfile = NULL;
    text = NULL;
}
