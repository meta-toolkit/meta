/**
 * string file porter2_stemmer.cpp
 */

#include <boost/regex.hpp>
#include "porter2_stemmer.h"

using namespace Porter2Stemmer::internal;
using std::string;
using boost::regex;
using boost::smatch;

#include <iostream>
using std::cout;
using std::endl;

string Porter2Stemmer::stem(const string & toStem)
{
    string word = prepareWord(toStem);

    if(returnImmediately(word))
        return finalStem(word);

    changeY(word);
    int startR1 = getStartR1(word);
    int startR2 = getStartR2(word, startR1);
    removeApostrophe(word);
    step1A(word);
    step1B(word, startR1);
    step1C(word);
    step2(word, startR1);
    step3(word, startR1, startR2);
    step4(word, startR2);
    step5(word, startR1, startR2);

    return finalStem(word);
}

string Porter2Stemmer::internal::finalStem(string & word)
{
    word = regex_replace(word, regex("Y"), "y");
    word = regex_replace(word, regex("'"), "");
    return word;
}

string Porter2Stemmer::internal::prepareWord(const string & toStem)
{
    string word = "";
    for(auto ch: toStem)
    {
        if(ch > 'A' && ch < 'Z')
            ch += 32;
        if((ch > 'a' && ch < 'z') || ch == '\'')
            word += ch;
    }
    return word;
}

bool Porter2Stemmer::internal::returnImmediately(const string & word)
{
    return word.size() <= 2;
}

int Porter2Stemmer::internal::getStartR1(const string & word)
{
    smatch results;
    if(regex_search(word, results, regex("[aeiouy][^aeiouy]")))
        return results.position() + 2;
    else
        return word.size();
}

int Porter2Stemmer::internal::getStartR2(const string & word, int startR1)
{
    if(startR1 == word.size())
        return startR1;

    string split = word.substr(startR1, word.size() - startR1);

    smatch results;
    if(regex_search(split, results, regex("[aeiouy][^aeiouy]")))
        return results.position() + startR1 + 2;
    else
        return word.size();
}

void Porter2Stemmer::internal::changeY(string & word)
{
    if(word.find_first_of("y") == string::npos)
        return;

    if(word[0] == 'y')
        word[0] = 'Y';

    word = regex_replace(word, regex("([aeiou])y"), "$1Y");
}

void Porter2Stemmer::internal::removeApostrophe(string & word)
{
    smatch results;
    word = regex_replace(word, regex("'s.*$"), "");
}

void Porter2Stemmer::internal::step1A(string & word)
{
    smatch results;
    if(regex_search(word, results, regex("sses$")))
        word = regex_replace(word, regex("(.*)sses$"), "$1ss");
    else if(regex_search(word, results, regex("(.+)(ied|ies)$")))
        word = regex_replace(word, regex("(.+)(ied|ies)$"), "$1i");
    else if(regex_search(word, results, regex("(.*)(ied|ies)$")))
        word = regex_replace(word, regex("(.*)(ied|ies)$"), "$1ie");
    else if(regex_search(word, results, regex("(u|s)s$")))
        return;
    else if(regex_search(word, results, regex(".*[aeiouy].+s$")))
        word = word.substr(0, word.size() - 1);
}

void Porter2Stemmer::internal::step1B(string & word, int startR1)
{
  //if word.search(/(eed|eedly)$/) >= startR1
  //return word.replace(/(\w*)(eed|eedly)/, "$1ee")
  //if word.match(/\w*?[aeiouy]\w+(ed|edly|ing|ingly)$/)
  //word = word.match(/^(\w*?[aeiouy]\w+)(ed|edly|ing|ingly)$/)[1]
  //return word + "e" if word.match(/(at|bl|iz)$/)
  //if word.match(/(bb|dd|ff|gg|mm|nn|pp|rr|tt)$/)
  //return word.slice(0, word.length - 1)
  //return word + "e" if string isShort(word, startR1)
}

void Porter2Stemmer::internal::step1C(string & word)
{
    word = regex_replace(word, regex("(.+[^aeiouy])(y|Y)$"), "$1i");
}

void Porter2Stemmer::internal::step2(string & word, int startR1)
{
    smatch results;
    if(regex_search(word, results, regex("ational$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)ational$"), "$1ate");
    else if(regex_search(word, results, regex("tional$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)tional$"), "$1tion");
    else if(regex_search(word, results, regex("ization$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)ization$"), "$1ize");
    else if(regex_search(word, results, regex("(.+)(ation|ator)$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)(ation|ator)$"), "$1ate");
    else if(regex_search(word, results, regex("(alism|aliti|alli)$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)(alism|aliti|alli)$"), "$1al");
    else if(regex_search(word, results, regex("enci$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)enci$"), "$1ence");
    else if(regex_search(word, results, regex("anci$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)anci$"), "$1ance");
    else if(regex_search(word, results, regex("abli$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)abli$"), "$1able");
    else if(regex_search(word, results, regex("entli$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)entli$"), "$1ent");
    else if(regex_search(word, results, regex("fulness$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)fulness$"), "$1ful");
    else if(regex_search(word, results, regex("(ousli|ousness)$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)(ousli|ousness)$"), "$1ous");
    else if(regex_search(word, results, regex("(iveness|iviti)$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)(iveness|iviti)$"), "$1ive");
    else if(regex_search(word, results, regex("(biliti|bli)$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)(biliti|bli)$"), "$1ble");
    else if(regex_search(word, results, regex("logi$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.*l)ogi$"), "$1og");
    else if(regex_search(word, results, regex("fulli$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)fulli$"), "$1ful");
    else if(regex_search(word, results, regex("lessli$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)(lessli)$"), "$1less");
    else if(regex_search(word, results, regex("[cdeghkmnrt]li$")) && results.position() >= startR1)
        word = regex_replace(word, regex("(.+)li$"), "$1");
}

void Porter2Stemmer::internal::step3(string & word, int startR1, int startR2)
{
  //if word.search(/ational$/) >= startR1
  //return word.replace /(\w*)ational$/, "$1ate"
  //if word.search(/tional$/) >= startR1
  //return word.replace /(\w*)tional$/, "$1tion"
  //if word.search(/alize$/) >= startR1
  //return word.replace /(\w*)alize$/, "$1al"
  //if word.search(/(icate|iciti|ical)$/) >= startR1
  //return word.replace /(\w*)(icate|iciti|ical)$/, "$1ic"
  //if word.search(/(ful|ness)$/) >= startR1
  //return word.replace /(\w*)(ful|ness)$/, "$1"
  //if word.search(/ative$/) >= startR2
  //return word.replace /(\w*)ative$/, "$1"
}

void Porter2Stemmer::internal::step4(string & word, int startR2)
{
  //if word.search(/ement$/) >= startR2
  //return word.replace /(\w*)ement$/, "$1"
  //if word.search(/ment$/) >= startR2
  //return word.replace /(\w*)ment$/, "$1"
  //if word.search(/(al|ance|ence|er|ic|able|ible|ant|ent|ism|ate|iti|ous|ive|ize)$/) >= startR2
  //return word.replace /(\w*)(al|ance|ence|er|ic|able|ible|ant|ent|ism|ate|iti|ous|ive|ize)$/, "$1"
  //if word.search(/(s|t)ion$/) >= startR2
  //return word.replace /(\w*)(s|t)ion$/, "$1"
}

void Porter2Stemmer::internal::step5(string & word, int startR1, int startR2)
{
  //if word.search(/e$/) >= startR2
  //return word.slice(0, word.length - 1)
  //if word.search(/e$/) >= startR1 and (not string isShort(word.match(/(\w*)e$/)[1], startR1))
  //return word.slice(0, word.length - 1)
  //if word.search(/ll$/) >= startR2
  //return word.slice(0, word.length - 1)
}

bool Porter2Stemmer::internal::isShort(const string & word, int startR1)
{
    //word.match(/^([aeouiy][^aeouiy]|\w*[^aeiouy][aeouiy][^aeouiyYwx])$/) != null and startR1 >= word.length
    return false;
}
