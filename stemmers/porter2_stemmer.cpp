/**
 * @file porter2_stemmer.cpp
 */

#include <algorithm>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include "porter2_stemmer.h"

#define DEBUG 1

using namespace Porter2Stemmer::internal;
using std::string;
using std::vector;
using std::stringstream;
using std::endl;
using std::cout;

string Porter2Stemmer::stem(const string & toStem)
{
    if(DEBUG) cout << __func__ << ": " << toStem << endl;
    string word = trim(toStem);

    if(returnImmediately(word))
        return finalStem(word);

    if(special(word))
        return finalStem(word);

    changeY(word);
    int startR1 = getStartR1(word);
    int startR2 = getStartR2(word, startR1);
    removeApostrophe(word);

    if(step1A(word))
        return finalStem(word);
    
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
    std::replace(word.begin(), word.end(), 'Y', 'y');
    std::remove(word.begin(), word.end(), '\'');
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

    // remove leading apostrophe
    if(word.length() >= 1 && word[0] == '\'')
        word = word.substr(1, word.length() - 1);

    return word;
}

bool Porter2Stemmer::internal::returnImmediately(const string & word)
{
    return word.length() <= 2;
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

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
    if(DEBUG) cout << "    startR1: " << startR1 << endl;

    return startR1;
}

int Porter2Stemmer::internal::getStartR2(const string & word, int startR1)
{
    if(startR1 == word.length())
        return startR1;

    int startR2 = firstNonVowelAfterVowel(word, startR1);

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
    if(DEBUG) cout << "    startR2: " << startR2 << endl;

    return startR2;
}

int Porter2Stemmer::internal::firstNonVowelAfterVowel(const string & word, int start)
{
    for(size_t i = start; i < word.length(); ++i)
    {
        if(isVowelY(word[i]) && !isVowelY(word[i - 1]))
            return i + 1;
    }

    return word.length();
}

void Porter2Stemmer::internal::changeY(string & word)
{
    if(word.find_first_of("y") == string::npos)
        return;

    if(word[0] == 'y')
        word[0] = 'Y';

    for(size_t i = 1; i < word.length(); ++i)
    {
        if(word[i] == 'y' && isVowel(word[i - 1]))
            word[i++] = 'Y'; // skip next iteration
    }

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::removeApostrophe(string & word)
{
    if(endsWith(word, "'s'"))
        word = word.substr(0, word.length() - 3);
    else if(endsWith(word, "s'"))
        word = word.substr(0, word.length() - 2);
    else if(endsWith(word, "'"))
        word = word.substr(0, word.length() - 1);

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

bool Porter2Stemmer::internal::step1A(string & word)
{
    /*
    const vector<Replacement> replacements = {
        Replacement("sses$", "(.+)sses$", "$1ss"),
        Replacement("(ied|ies)$", "(..+)(ied|ies)$", "$1i"),
        Replacement("(ied|ies)$", "(.)(ied|ies)$", "$1ie")
    };

    if(!replace(replacements, word, 0))
    {
        smatch results;
        if(regex_search(word, results, regex("(u|s)s$")))
            return false;
        else if(regex_search(word, results, regex(".*[aeiouy].+s$")))
            word = word.substr(0, word.length() - 1);
    }
    */

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;

    // special case after step 1a
    return word == "inning" || word == "outing" || word == "canning" || word == "herring" ||
        word == "earring" || word == "proceed" || word == "exceed" || word == "succeed";
}

void Porter2Stemmer::internal::step1B(string & word, int startR1)
{
    /*
    smatch results;
    if(regex_search(word, results, regex("(eed|eedly)$")))
    {
        if(results.position() >= startR1)
            word = regex_replace(word, regex("(.+)(eed|eedly)$"), "$1ee");
    }
    else if(regex_search(word, results, regex("^.*[aeiouy].*(ed|edly|ing|ingly)$")))
    {
        word = regex_replace(word, regex("(^.*[aeiouy].*)(ed|edly|ing|ingly)"), "$1");
        if(regex_search(word, results, regex("(at|bl|iz)$")) || isShort(word, startR1))
        {
            cout << "adding e" << endl;
            word = word + "e";
        }
        else if(regex_search(word, results, regex("(bb|dd|ff|gg|mm|nn|pp|rr|tt)$")))
            word = word.substr(0, word.length() - 1);
    }
    */

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step1C(string & word)
{
    //word = regex_replace(word, regex("(.+[^aeiouy])(y|Y)$"), "$1i");
    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step2(string & word, int startR1)
{
    /*
    const vector<Replacement> replacements = {
        Replacement("ational$", "(.+)ational$", "$1ate"),
        Replacement("tional$", "(.+)tional$", "$1tion"),
        Replacement("ization$", "(.+)ization$", "$1ize"),
        Replacement("(ation|ator)$", "(.+)(ation|ator)$", "$1ate"),
        Replacement("(alism|aliti|alli)$", "(.+)(alism|aliti|alli)$", "$1al"),
        Replacement("enci$", "(.+)enci$", "$1ence"),
        Replacement("anci$", "(.+)anci$", "$1ance"),
        Replacement("abli$", "(.+)abli$", "$1able"),
        Replacement("entli$", "(.+)entli$", "$1ent"),
        Replacement("fulness$", "(.+)fulness$", "$1ful"),
        Replacement("(ousli|ousness)$", "(.+)(ousli|ousness)$", "$1ous"),
        Replacement("(iveness|iviti)$", "(.+)(iveness|iviti)$", "$1ive"),
        Replacement("(biliti|bli)$", "(.+)(biliti|bli)$", "$1ble"),
        Replacement("logi$", "(.*l)ogi$", "$1og"),
        Replacement("fulli$", "(.+)fulli$", "$1ful"),
        Replacement("lessli$", "(.+)(lessli)$", "$1less"),
        Replacement("[cdeghkmnrt]li$", "(.+)li$", "$1")
    };

    replace(replacements, word, startR1);
    */

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step3(string & word, int startR1, int startR2)
{
    /*
    const vector<Replacement> replacements = {
        Replacement("ational$", "(.+)ational$", "$1ate"),
        Replacement("tional$", "(.+)tional$", "$1tion"),
        Replacement("alize$", "(.+)alize$", "$1al"),
        Replacement("(icate|iciti|ical)$", "(.+)(icate|iciti|ical)$", "$1ic"),
        Replacement("(ful|ness)$", "(.+)(ful|ness)$", "$1")
    };

    if(!replace(replacements, word, startR1))
    {
        const vector<Replacement> other = { Replacement("ative$", "(.+)ative$", "$1") };
        replace(other, word, startR2);
    }
    */

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step4(string & word, int startR2)
{
    /*
    const vector<Replacement> replacements = {
        Replacement("e?ment$", "(.*)e?ment$", "$1"), // combined two here
        Replacement("(al|ance|ence|er|ic|able|ible|ant|ent|ism|ate|iti|ous|ive|ize)$",
                    "(.*)(al|ance|ence|er|ic|able|ible|ant|ent|ism|ate|iti|ous|ive|ize)$", "$1"),
        Replacement("(s|t)ion$", "(.*(s|t))ion$", "$1")
    };

    replace(replacements, word, startR2);
    */

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

void Porter2Stemmer::internal::step5(string & word, int startR1, int startR2)
{
    /*
    smatch matches;
    if(regex_search(word, matches, regex("e$")))
    {
        if(matches.position() >= startR2 ||
                (
                 matches.position() >= startR1 &&
                 !regex_search(word, matches, regex("^([aeouiy][^aeouiy]|.*[^aeiouy][aeouiy][^aeouiyYwx])e$"))
                )
          )
            word = word.substr(0, word.length() - 1);
    }
    else if(regex_search(word, matches, regex("ll$")) && matches.position() >= startR2)
    {
        word = word.substr(0, word.length() - 1);
    }
    else if(regex_search(word, matches, regex("e$")) && matches.position() >= startR1)
    {
        if(!isShort(regex_replace(word, regex("(.+)e$"), "$1"), startR1))
            word = word.substr(0, word.length() - 1);
    }
    else if(regex_search(word, matches, regex("ll$")) && matches.position() >= startR2)
        word = word.substr(0, word.length() - 1);
    */

    if(DEBUG) cout << "  " << __func__ << ": " << word << endl;
}

/**
 * Define a short syllable in a word as either (a) a vowel followed by a
 * non-vowel other than w, x or Y and preceded by a non-vowel, or * (b) a vowel
 * at the beginning of the word followed by a non-vowel.
 *
 * A word is called short if it ends in a short syllable, and if R1 is null.
 */
bool Porter2Stemmer::internal::isShort(const string & word, int startR1)
{
    /*
    smatch results;
    return regex_search(word, results, regex("^([aeouiy][^aeouiy]|.*[^aeiouy][aeouiy][^aeouiyYwx])$"))
        && startR1 >= word.length();
    */
}

bool Porter2Stemmer::internal::special(string & word)
{
    const std::unordered_map<string, string> exceptions = {
        {"skis", "ski"}, {"skies", "sky"},
        {"dying", "die"}, {"lying", "lie"},
        {"tying", "tie"}, {"idly", "idl"},
        {"gently", "gentl"}, {"ugly", "ugli"},
        {"early", "earli"}, {"only", "onli"},
        {"singly", "singl"} };

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

bool Porter2Stemmer::internal::replace(const vector<Replacement> & replacements, string & word, int position)
{
    /*
    smatch results;
    for(auto & rep: replacements)
    {
        if(regex_search(word, results, regex(rep.searchRegex)))
        {
            if(DEBUG) cout << "      regex_search true for " << rep.searchRegex << endl;
            if(results.position() >= position - 1)
            {
                word = regex_replace(word, regex(rep.replaceRegex), rep.replaceStr);
                if(DEBUG) cout << "      replaced with " << rep.searchRegex << endl;
                return true;
            }
        }
    }
    */
    return false;
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
    return word.length() >= str.length() &&
        word.substr(word.length() - str.length()) == str;
}
