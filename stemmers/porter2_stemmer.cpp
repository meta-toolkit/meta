/**
 * string file porter2_stemmer.cpp
 */

#include <boost/regex.hpp>
#include <iostream>
#include "porter2_stemmer.h"

using namespace Porter2Stemmer::internal;
using std::string;

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
    word = regex_replace(word, boost::regex("Y"), "y");
    word = regex_replace(word, boost::regex("'"), "");
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
    return 1;
    //startR1 = word.search(/[aeiouy][^aeiouy]/)
    //if(startR1 == -1)
    //    return word.size();
    //else
    //    return std::distance(word.begin(), it) + 2;
}

int Porter2Stemmer::internal::getStartR2(const string & word, int startR1)
{
  //return startR1 if startR1 == word.length
  //r1 = word.slice(startR1)
  //startR2 = r1.search(/[aeiouy][^aeiouy]/)
  //if startR2 == -1 then word.length else startR1 + startR2 + 2
    return 1;
}

void Porter2Stemmer::internal::changeY(string & word)
{
  //return word if word.indexOf("y") == -1
  //word = "Y" + word.slice(1) if word.charAt(0) == "y"
  //word.replace(/([aeiou])y/g, "$1Y")
}

void Porter2Stemmer::internal::removeApostrophe(string & word)
{
  //match = word.match /^(\w*)('s?)$/
  //return word if match == null
  //match[1]
}

void Porter2Stemmer::internal::step1A(string & word)
{
  //if word.match /sses$/
  //return word.replace /(\w*)sses$/, "$1ss"
  //if word.match /(\w*)(ied|ies)$/
  //if word.match(/(\w*)(ied|ies)$/)[1].length > 1
  //return word.replace /(\w*)(ied|ies)$/, "$1i"
  //else
  //return word.replace /(\w*)(ied|ies)$/, "$1ie"
  //return word if word.match(/(\w*)(u|s)s$/)
  //if word.match(/\w*?[aeiouy]\w+s$/)
  //return word.slice(0, word.length - 1)
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
  //word.replace /(\w+[^aeiouy])(y|Y)$/, "$1i"
}

void Porter2Stemmer::internal::step2(string & word, int startR1)
{
  //if word.search(/ational$/) >= startR1
  //return word.replace /(\w*)ational$/, "$1ate"
  //if word.search(/tional$/) >= startR1
  //return word.replace /(\w*)tional$/, "$1tion"
  //if word.search(/ization$/) >= startR1
  //return word.replace /(\w*)ization$/, "$1ize"
  //if word.search(/(ation|ator)$/) >= startR1
  //return word.replace /(\w*)(ation|ator)$/, "$1ate"
  //if word.search(/(alism|aliti|alli)$/) >= startR1
  //return word.replace /(\w*)(alism|aliti|alli)$/, "$1al"
  //if word.search(/enci$/) >= startR1
  //return word.replace /(\w*)enci$/, "$1ence"
  //if word.search(/anci$/) >= startR1
  //return word.replace /(\w*)anci$/, "$1ance"
  //if word.search(/abli$/) >= startR1
  //return word.replace /(\w*)abli$/, "$1able"
  //if word.search(/entli$/) >= startR1
  //return word.replace /(\w*)entli$/, "$1ent"
  //if word.search(/fulness$/) >= startR1
  //return word.replace /(\w*)fulness$/, "$1ful"
  //if word.search(/(ousli|ousness)$/) >= startR1
  //return word.replace /(\w*)(ousli|ousness)$/, "$1ous"
  //if word.search(/(iveness|iviti)$/) >= startR1
  //return word.replace /(\w*)(iveness|iviti)$/, "$1ive"
  //if word.search(/(biliti|bli)$/) >= startR1
  //return word.replace /(\w*)(biliti|bli)$/, "$1ble"
  //if word.search(/logi$/) >= startR1
  //return word.replace /(\w*l)ogi$/, "$1og"
  //if word.search(/fulli$/) >= startR1
  //return word.replace /(\w*)fulli$/, "$1ful"
  //if word.search(/lessli$/) >= startR1
  //return word.replace /(\w*)lessli$/, "$1less"
  //if word.search(/[cdeghkmnrt]li$/) >= startR1
  //return word.replace /(\w*)li$/, "$1"
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

bool Porter2Stemmer::internal::isVowel(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' ||
           ch == 'u' || ch == 'o' || ch == 'y';
}
