/**
 * @file porter2_stemmer.cpp
 * @author Sean Massung
 * @date September 2012
 *
 * Implementation of
 * http://snowball.tartarus.org/algorithms/english/stemmer.html
 *
 * Copyright (C) 2012 Sean Massung
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <algorithm>
#include <utility>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "stemmers/porter2_stemmer.h"

using namespace Porter2Stemmer::internal;
using std::pair;
using std::string;
using std::vector;
using std::stringstream;
using std::endl;
using std::cout;

string Porter2Stemmer::stem(const string & toStem)
{
    if(toStem.size() <= 2)
        return toStem;

    string word = toStem;

    if(word[0] == '\'')
        word = word.substr(1, word.size() - 1);

    if(special(word))
        return word;

    changeY(word);
    int startR1 = getStartR1(word);
    int startR2 = getStartR2(word, startR1);

    step0(word);

    if(step1A(word))
    {
        std::replace(word.begin(), word.end(), 'Y', 'y');
        return word;
    }
    
    step1B(word, startR1);
    step1C(word);
    step2(word, startR1);
    step3(word, startR1, startR2);
    step4(word, startR2);
    step5(word, startR1, startR2);

    std::replace(word.begin(), word.end(), 'Y', 'y');
    return word;
}

string Porter2Stemmer::trim(const string & toStem)
{
    string word = "";
    for(auto ch: toStem)
    {
        if(ch >= 'A' && ch <= 'Z')
            ch += 32;
        if((ch >= 'a' && ch <= 'z') || ch == '\'')
            word += ch;
    }
    return word;
}

int Porter2Stemmer::internal::getStartR1(const string & word)
{
    // special cases
    if(word.substr(0, 5) == "gener")
        return 5;
    if(word.substr(0, 6) == "commun")
        return 6;
    if(word.substr(0, 5) == "arsen")
        return 5;

    // general case
    int startR1 = firstNonVowelAfterVowel(word, 1);

    return startR1;
}

int Porter2Stemmer::internal::getStartR2(const string & word, int startR1)
{
    if(startR1 == word.size())
        return startR1;

    int startR2 = firstNonVowelAfterVowel(word, startR1 + 1);

    return startR2;
}

int Porter2Stemmer::internal::firstNonVowelAfterVowel(const string & word, int start)
{
    for(size_t i = start; i != 0 && i < word.size(); ++i)
    {
        if(!isVowelY(word[i]) && isVowelY(word[i - 1]))
            return i + 1;
    }

    return word.size();
}

void Porter2Stemmer::internal::changeY(string & word)
{
    if(word.find_first_of("y") == string::npos)
        return;

    if(word[0] == 'y')
        word[0] = 'Y';

    for(size_t i = 1; i < word.size(); ++i)
    {
        if(word[i] == 'y' && isVowel(word[i - 1]))
            word[i++] = 'Y'; // skip next iteration
    }
}

/**
  Step 0
*/
void Porter2Stemmer::internal::step0(string & word)
{
    // short circuit the longest suffix
    replaceIfExists(word, "'s'", "", 0)
        || replaceIfExists(word, "'s", "", 0)
        || replaceIfExists(word, "'", "", 0);
}

/**
  Step 1a:

  sses
    replace by ss 

  ied   ies
    replace by i if preceded by more than one letter, otherwise by ie
    (so ties -> tie, cries -> cri) 

  us   ss
    do nothing

  s
    delete if the preceding word part contains a vowel not immediately before the
    s (so gas and this retain the s, gaps and kiwis lose it) 
*/
bool Porter2Stemmer::internal::step1A(string & word)
{
    if(!replaceIfExists(word, "sses", "ss", 0))
    {
        if(endsWith(word, "ied") || endsWith(word, "ies"))
        {
            // if preceded by only one letter
            if(word.size() <= 4)
                word = word.substr(0, word.size() - 1);
            else
                word = word.substr(0, word.size() - 2);
        }
        else if(endsWith(word, "s") && !endsWith(word, "us") && !endsWith(word, "ss"))
        {
            if(containsVowel(word, 0, word.size() - 2))
                word = word.substr(0, word.size() - 1);
        }
    }

    // special case after step 1a
    return word == "inning" || word == "outing" || word == "canning" || word == "herring" ||
        word == "earring" || word == "proceed" || word == "exceed" || word == "succeed";
}

/**
  Step 1b:

  eed   eedly
      replace by ee if in R1 

  ed   edly   ing   ingly
      delete if the preceding word part contains a vowel, and after the deletion:
      if the word ends at, bl or iz add e (so luxuriat -> luxuriate), or
      if the word ends with a double remove the last letter (so hopp -> hop), or
      if the word is short, add e (so hop -> hope)
*/
void Porter2Stemmer::internal::step1B(string & word, int startR1)
{
    bool exists = endsWith(word, "eedly") || endsWith(word, "eed");

    if(exists)
        replaceIfExists(word, "eedly", "ee", startR1) || replaceIfExists(word, "eed", "ee", startR1);
    else
    {
        size_t size = word.size();
        bool deleted = (containsVowel(word, 0, size - 2) && replaceIfExists(word, "ed", "", 0))
            || (containsVowel(word, 0, size - 4) && replaceIfExists(word, "edly", "", 0))
            || (containsVowel(word, 0, size - 3) && replaceIfExists(word, "ing", "", 0))
            || (containsVowel(word, 0, size - 5) && replaceIfExists(word, "ingly", "", 0));

        if(deleted && (endsWith(word, "at") || endsWith(word, "bl") || endsWith(word, "iz")))
            word = word + "e";
        else if(deleted && endsInDouble(word))
            word = word.substr(0, word.size() - 1);
        else if(deleted && startR1 == word.size() && isShort(word))
            word = word + "e";
    }
}

/**
  Step 1c:

  Replace suffix y or Y by i if preceded by a non-vowel which is not the first
  letter of the word (so cry -> cri, by -> by, say -> say)
*/
void Porter2Stemmer::internal::step1C(string & word)
{
    size_t size = word.size();
    if(size > 2 && (word[size - 1] == 'y' || word[size - 1] == 'Y'))
        if(!isVowel(word[size - 2]))
            word[size - 1] = 'i';
}

/**
  Step 2:

  If found and in R1, perform the action indicated. 

  tional:               replace by tion
  enci:                 replace by ence
  anci:                 replace by ance
  abli:                 replace by able
  entli:                replace by ent
  izer, ization:        replace by ize
  ational, ation, ator: replace by ate
  alism, aliti, alli:   replace by al
  fulness:              replace by ful
  ousli, ousness:       replace by ous
  iveness, iviti:       replace by ive
  biliti, bli:          replace by ble
  fulli:                replace by ful
  lessli:               replace by less
  ogi:                  replace by og if preceded by l
  li:                   delete if preceded by a valid li-ending
*/
void Porter2Stemmer::internal::step2(string & word, int startR1)
{
    const vector<pair<string, string>> subs = {
        {"ational", "ate"}, {"tional", "tion"}, {"enci", "ence"}, {"anci", "ance"},
        {"abli", "able"}, {"entli", "ent"}, {"izer", "ize"}, {"ization", "ize"},
        {"ation", "ate"}, {"ator", "ate"}, {"alism", "al"}, {"aliti", "al"},
        {"alli", "al"}, {"fulness", "ful"}, {"ousli", "ous"}, {"ousness", "ous"},
        {"iveness", "ive"}, {"iviti", "ive"}, {"biliti", "ble"}, {"bli", "ble"},
        {"fulli", "ful"}, {"lessli", "less"}
    };

    for(auto & sub: subs)
        if(replaceIfExists(word, sub.first, sub.second, startR1))
            return;

    if(!replaceIfExists(word, "logi", "log", startR1 - 1))
    {
        // make sure we choose the longest suffix
        if(endsWith(word, "li") && !endsWith(word, "abli") && !endsWith(word, "entli")
                && !endsWith(word, "aliti") && !endsWith(word, "alli") && !endsWith(word, "ousli")
                && !endsWith(word, "bli") && !endsWith(word, "fulli") && !endsWith(word, "lessli"))
            if(word.size() > 3 && word.size() - 2 >= startR1 && isValidLIEnding(word[word.size() - 3]))
                word = word.substr(0, word.size() - 2);
    }
}

/**
  Step 3:
  
  If found and in R1, perform the action indicated. 

  ational:            replace by ate
  tional:             replace by tion
  alize:              replace by al
  icate, iciti, ical: replace by ic
  ful, ness:          delete
  ative:              delete if in R2
*/
void Porter2Stemmer::internal::step3(string & word, int startR1, int startR2)
{
    const vector<pair<string, string>> subs = {
        {"ational", "ate"}, {"tional", "tion"}, {"alize", "al"}, {"icate", "ic"},
        {"iciti", "ic"}, {"ical", "ic"}, {"ful", ""}, {"ness", ""}
    };

    for(auto & sub: subs)
        if(replaceIfExists(word, sub.first, sub.second, startR1))
            return;

    replaceIfExists(word, "ative", "", startR2);
}

/**
  Step 4:

  If found and in R2, perform the action indicated. 

  al ance ence er ic able ible ant ement ment ent ism ate
    iti ous ive ize
                              delete
  ion
                              delete if preceded by s or t
*/
void Porter2Stemmer::internal::step4(string & word, int startR2)
{
    const vector<pair<string, string>> subs = {
        {"al", ""}, {"ance", ""}, {"ence", ""}, {"er", ""}, {"ic", ""},
        {"able", ""}, {"ible", ""}, {"ant", ""}, {"ement", ""}, {"ment", ""},
        {"ism", ""}, {"ate", ""}, {"iti", ""}, {"ous", ""}, {"ive", ""}, {"ize", ""}
    };

    for(auto & sub: subs)
        if(replaceIfExists(word, sub.first, sub.second, startR2))
            return;

    // make sure we only choose the longest suffix
    if(!endsWith(word, "ement") && !endsWith(word, "ment"))
        if(replaceIfExists(word, "ent", "", startR2))
            return;

    // short circuit
    replaceIfExists(word, "sion", "s", startR2 - 1)
        || replaceIfExists(word, "tion", "t", startR2 - 1);
}

/**
  Step 5:

  e     delete if in R2, or in R1 and not preceded by a short syllable
  l     delete if in R2 and preceded by l
*/
void Porter2Stemmer::internal::step5(string & word, int startR1, int startR2)
{
    size_t size = word.size();
    if(word[size - 1] == 'e')
    {
        if(size - 1 >= startR2)
            word = word.substr(0, size - 1);
        else if(size - 1 >= startR1 && !isShort(word.substr(0, size - 1)))
            word = word.substr(0, size - 1);
    }
    else if(word[word.size() - 1] == 'l')
    {
        if(word.size() - 1 >= startR2 && word[word.size() - 2] == 'l')
            word = word.substr(0, word.size() - 1);
    }
}

/**
 * Determines whether a word ends in a short syllable.
 * Define a short syllable in a word as either
 *
 * (a) a vowel followed by a non-vowel other than w, x or Y and preceded by a non-vowel
 * (b) a vowel at the beginning of the word followed by a non-vowel.
 */
bool Porter2Stemmer::internal::isShort(const string & word)
{
    size_t size = word.size();

    if(size >= 3)
    {
        if(!isVowelY(word[size - 3]) && isVowelY(word[size - 2])
                && !isVowelY(word[size - 1]) && word[size - 1] != 'w'
                && word[size - 1] != 'x' && word[size - 1] != 'Y')
            return true;
    }
    return size == 2 && isVowelY(word[0]) && !isVowelY(word[1]);
}

bool Porter2Stemmer::internal::special(string & word)
{
    const std::unordered_map<string, string> exceptions = {
        {"skis", "ski"}, {"skies", "sky"}, {"dying", "die"}, {"lying", "lie"},
        {"tying", "tie"}, {"idly", "idl"}, {"gently", "gentl"}, {"ugly", "ugli"},
        {"early", "earli"}, {"only", "onli"}, {"singly", "singl"}
    };

    // special cases
    auto ex = exceptions.find(word);
    if(ex != exceptions.end())
    {
        word = ex->second;
        return true;
    }

    // invariants
    return word == "sky" || word == "news" || word == "howe" ||
       word == "atlas" || word == "cosmos" || word == "bias" ||
       word == "andes";
}

bool Porter2Stemmer::internal::isVowelY(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' ||
        ch == 'o' || ch == 'u' || ch == 'y';
}

bool Porter2Stemmer::internal::isVowel(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' ||
        ch == 'o' || ch == 'u';
}

bool Porter2Stemmer::internal::endsWith(const string & word, const string & str)
{
    return word.size() >= str.size() &&
        word.substr(word.size() - str.size()) == str;
}

bool Porter2Stemmer::internal::endsInDouble(const string & word)
{
    if(word.size() >= 2)
    {
        char a = word[word.size() - 1];
        char b = word[word.size() - 2];

        if(a == b)
            return a == 'b' || a == 'd' || a == 'f' || a == 'g'
                || a == 'm' || a == 'n' || a == 'p' || a == 'r' || a == 't';
    }

    return false;
}

bool Porter2Stemmer::internal::replaceIfExists(string & word,
        const string & suffix, const string & replacement, size_t start)
{
    if(!(start > word.size()) && endsWith(word.substr(start, word.size() - start), suffix))
    {
        word = word.substr(0, word.size() - suffix.size()) + replacement;
        return true;
    }
    return false;
}

bool Porter2Stemmer::internal::isValidLIEnding(char ch)
{
    return ch == 'c' || ch == 'd' || ch == 'e' || ch == 'g' || ch == 'h'
        || ch == 'k' || ch == 'm' || ch == 'n' || ch == 'r' || ch == 't';
}

bool Porter2Stemmer::internal::containsVowel(const string & word, int start, int end)
{
    if(start >=0 && end > 0 && start <= end && end < word.size())
    {
        for(size_t i = start; i < end; ++i)
            if(isVowelY(word[i]))
                return true;
    }
    return false;
}
