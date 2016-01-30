/**
 * @file porter2_stemmer.cpp
 * @author Sean Massung
 * @date September 2012
 *
 * Implementation of
 * http://snowball.tartarus.org/algorithms/english/stemmer.html
 */

#include <algorithm>
#include <utility>
#include <unordered_map>
#include "meta/analyzers/filters/porter2_stemmer.h"

namespace meta
{
namespace analyzers
{
namespace filters
{
namespace porter2
{
namespace
{
bool is_vowel(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' || ch == 'o' || ch == 'u';
}

bool is_vowel_y(char ch)
{
    return ch == 'e' || ch == 'a' || ch == 'i' || ch == 'o' || ch == 'u'
           || ch == 'y';
}

size_t first_non_vowel_after_vowel(const std::string& word, size_t start)
{
    for (size_t i = start; i != 0 && i < word.size(); ++i)
    {
        if (!is_vowel_y(word[i]) && is_vowel_y(word[i - 1]))
            return i + 1;
    }

    return word.size();
}

bool replace_if_exists(std::string& word, meta::util::string_view suffix,
                       meta::util::string_view replacement, size_t start)
{
    if (suffix.size() > word.size())
        return false;

    size_t idx = word.size() - suffix.size();
    if (idx < start)
        return false;

    auto diff = static_cast<std::string::iterator::difference_type>(idx);
    if (std::equal(word.begin() + diff, word.end(), suffix.begin()))
    {
        word.replace(idx, suffix.size(), replacement.data());
        return true;
    }
    return false;
}

size_t get_start_r1(const std::string& word)
{
    // special cases
    if (word.size() >= 5 && word[0] == 'g' && word[1] == 'e' && word[2] == 'n'
        && word[3] == 'e' && word[4] == 'r')
        return 5;
    if (word.size() >= 6 && word[0] == 'c' && word[1] == 'o' && word[2] == 'm'
        && word[3] == 'm' && word[4] == 'u' && word[5] == 'n')
        return 6;
    if (word.size() >= 5 && word[0] == 'a' && word[1] == 'r' && word[2] == 's'
        && word[3] == 'e' && word[4] == 'n')
        return 5;

    // general case
    return first_non_vowel_after_vowel(word, 1);
}

size_t get_start_r2(const std::string& word, size_t startR1)
{
    if (startR1 == word.size())
        return startR1;

    return first_non_vowel_after_vowel(word, startR1 + 1);
}

bool ends_with(meta::util::string_view word, meta::util::string_view str)
{
    if (word.size() < str.size())
        return false;

    return word.substr(word.size() - str.size()) == str;
}

bool ends_in_double(const std::string& word)
{
    if (word.size() >= 2)
    {
        char a = word[word.size() - 1];
        char b = word[word.size() - 2];

        if (a == b)
            return a == 'b' || a == 'd' || a == 'f' || a == 'g' || a == 'm'
                   || a == 'n' || a == 'p' || a == 'r' || a == 't';
    }

    return false;
}

bool is_valid_li_ending(char ch)
{
    return ch == 'c' || ch == 'd' || ch == 'e' || ch == 'g' || ch == 'h'
           || ch == 'k' || ch == 'm' || ch == 'n' || ch == 'r' || ch == 't';
}

bool contains_vowel(const std::string& word, size_t start, size_t end)
{
    if (end <= word.size())
    {
        for (size_t i = start; i < end; ++i)
            if (is_vowel_y(word[i]))
                return true;
    }
    return false;
}

void change_y(std::string& word)
{
    if (word[0] == 'y')
        word[0] = 'Y';

    for (size_t i = 1; i < word.size(); ++i)
    {
        if (word[i] == 'y' && is_vowel(word[i - 1]))
            word[i++] = 'Y'; // skip next iteration
    }
}

/*
  Determines whether a word ends in a short syllable.
  Define a short syllable in a word as either

  (a) a vowel followed by a non-vowel other than w, x or Y and preceded by a
      non-vowel
  (b) a vowel at the beginning of the word followed by a non-vowel.
*/
bool is_short(const std::string& word)
{
    size_t size = word.size();

    if (size >= 3)
    {
        if (!is_vowel_y(word[size - 3]) && is_vowel_y(word[size - 2])
            && !is_vowel_y(word[size - 1]) && word[size - 1] != 'w'
            && word[size - 1] != 'x' && word[size - 1] != 'Y')
            return true;
    }
    return size == 2 && is_vowel_y(word[0]) && !is_vowel_y(word[1]);
}

bool special(std::string& word)
{
    static const std::unordered_map<meta::util::string_view,
                                    meta::util::string_view> exceptions
        = {{"skis", "ski"},     {"skies", "sky"},   {"dying", "die"},
           {"lying", "lie"},    {"tying", "tie"},   {"idly", "idl"},
           {"gently", "gentl"}, {"ugly", "ugli"},   {"early", "earli"},
           {"only", "onli"},    {"singly", "singl"}};

    // special cases
    auto ex = exceptions.find(word);
    if (ex != exceptions.end())
    {
        word = ex->second.to_string();
        return true;
    }

    // invariants
    return word.size() >= 3 && word.size() <= 5
           && (word == "sky" || word == "news" || word == "howe"
               || word == "atlas" || word == "cosmos" || word == "bias"
               || word == "andes");
}

/*
  Step 0
*/
void step0(std::string& word)
{
    // short circuit the longest suffix
    replace_if_exists(word, "'s'", "", 0)
        || replace_if_exists(word, "'s", "", 0)
        || replace_if_exists(word, "'", "", 0);
}

/*
  Step 1a:

  sses
    replace by ss

  ied   ies
    replace by i if preceded by more than one letter, otherwise by ie
    (so ties -> tie, cries -> cri)

  us   ss
    do nothing

  s
    delete if the preceding word part contains a vowel not immediately before
  the
    s (so gas and this retain the s, gaps and kiwis lose it)
*/
bool step1a(std::string& word)
{
    if (!replace_if_exists(word, "sses", "ss", 0))
    {
        if (ends_with(word, "ied") || ends_with(word, "ies"))
        {
            // if preceded by only one letter
            if (word.size() <= 4)
                word.pop_back();
            else
            {
                word.pop_back();
                word.pop_back();
            }
        }
        else if (ends_with(word, "s") && !ends_with(word, "us")
                 && !ends_with(word, "ss"))
        {
            if (word.size() > 2 && contains_vowel(word, 0, word.size() - 2))
                word.pop_back();
        }
    }

    // special case after step 1a
    return (word.size() == 6 || word.size() == 7)
           && (word == "inning" || word == "outing" || word == "canning"
               || word == "herring" || word == "earring" || word == "proceed"
               || word == "exceed" || word == "succeed");
}

/*
  Step 1b:

  eed   eedly
      replace by ee if in R1

  ed   edly   ing   ingly
      delete if the preceding word part contains a vowel, and after the
  deletion:
      if the word ends at, bl or iz add e (so luxuriat -> luxuriate), or
      if the word ends with a double remove the last letter (so hopp -> hop), or
      if the word is short, add e (so hop -> hope)
*/
void step1b(std::string& word, size_t startR1)
{
    bool exists = ends_with(word, "eedly") || ends_with(word, "eed");

    if (exists) // look only in startR1 now
        replace_if_exists(word, "eedly", "ee", startR1)
            || replace_if_exists(word, "eed", "ee", startR1);
    else
    {
        size_t size = word.size();
        bool deleted = (contains_vowel(word, 0, size - 2)
                        && replace_if_exists(word, "ed", "", 0))
                       || (contains_vowel(word, 0, size - 4)
                           && replace_if_exists(word, "edly", "", 0))
                       || (contains_vowel(word, 0, size - 3)
                           && replace_if_exists(word, "ing", "", 0))
                       || (contains_vowel(word, 0, size - 5)
                           && replace_if_exists(word, "ingly", "", 0));

        if (deleted && (ends_with(word, "at") || ends_with(word, "bl")
                        || ends_with(word, "iz")))
            word.push_back('e');
        else if (deleted && ends_in_double(word))
            word.pop_back();
        else if (deleted && startR1 == word.size() && is_short(word))
            word.push_back('e');
    }
}

/*
  Step 1c:

  Replace suffix y or Y by i if preceded by a non-vowel which is not the first
  letter of the word (so cry -> cri, by -> by, say -> say)
*/
void step1c(std::string& word)
{
    size_t size = word.size();
    if (size > 2 && (word[size - 1] == 'y' || word[size - 1] == 'Y'))
        if (!is_vowel(word[size - 2]))
            word[size - 1] = 'i';
}

/*
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
void step2(std::string& word, size_t startR1)
{
    static const std::pair<meta::util::string_view, meta::util::string_view>
        subs[] = {{"ational", "ate"}, {"tional", "tion"}, {"enci", "ence"},
                  {"anci", "ance"},   {"abli", "able"},   {"entli", "ent"},
                  {"izer", "ize"},    {"ization", "ize"}, {"ation", "ate"},
                  {"ator", "ate"},    {"alism", "al"},    {"aliti", "al"},
                  {"alli", "al"},     {"fulness", "ful"}, {"ousli", "ous"},
                  {"ousness", "ous"}, {"iveness", "ive"}, {"iviti", "ive"},
                  {"biliti", "ble"},  {"bli", "ble"},     {"fulli", "ful"},
                  {"lessli", "less"}};

    for (auto& sub : subs)
        if (replace_if_exists(word, sub.first, sub.second, startR1))
            return;

    if (!replace_if_exists(word, "logi", "log", startR1 - 1))
    {
        // make sure we choose the longest suffix
        if (ends_with(word, "li") && !ends_with(word, "abli")
            && !ends_with(word, "entli") && !ends_with(word, "aliti")
            && !ends_with(word, "alli") && !ends_with(word, "ousli")
            && !ends_with(word, "bli") && !ends_with(word, "fulli")
            && !ends_with(word, "lessli"))
            if (word.size() > 3 && word.size() - 2 >= startR1
                && is_valid_li_ending(word[word.size() - 3]))
            {
                word.pop_back();
                word.pop_back();
            }
    }
}

/*
  Step 3:

  If found and in R1, perform the action indicated.

  ational:            replace by ate
  tional:             replace by tion
  alize:              replace by al
  icate, iciti, ical: replace by ic
  ful, ness:          delete
  ative:              delete if in R2
*/
void step3(std::string& word, size_t startR1, size_t startR2)
{
    static const std::pair<meta::util::string_view, meta::util::string_view>
        subs[] = {{"ational", "ate"}, {"tional", "tion"}, {"alize", "al"},
                  {"icate", "ic"},    {"iciti", "ic"},    {"ical", "ic"},
                  {"ful", ""},        {"ness", ""}};

    for (auto& sub : subs)
        if (replace_if_exists(word, sub.first, sub.second, startR1))
            return;

    replace_if_exists(word, "ative", "", startR2);
}

/*
  Step 4:

  If found and in R2, perform the action indicated.

  al ance ence er ic able ible ant ement ment ent ism ate
    iti ous ive ize
                              delete
  ion
                              delete if preceded by s or t
*/
void step4(std::string& word, size_t startR2)
{
    static const std::pair<meta::util::string_view, meta::util::string_view>
        subs[] = {{"al", ""},    {"ance", ""}, {"ence", ""}, {"er", ""},
                  {"ic", ""},    {"able", ""}, {"ible", ""}, {"ant", ""},
                  {"ement", ""}, {"ment", ""}, {"ism", ""},  {"ate", ""},
                  {"iti", ""},   {"ous", ""},  {"ive", ""},  {"ize", ""}};

    for (auto& sub : subs)
        if (replace_if_exists(word, sub.first, sub.second, startR2))
            return;

    // make sure we only choose the longest suffix
    if (!ends_with(word, "ement") && !ends_with(word, "ment"))
        if (replace_if_exists(word, "ent", "", startR2))
            return;

    // short circuit
    replace_if_exists(word, "sion", "s", startR2 - 1)
        || replace_if_exists(word, "tion", "t", startR2 - 1);
}

/*
  Step 5:

  e     delete if in R2, or in R1 and not preceded by a short syllable
  l     delete if in R2 and preceded by l
*/
void step5(std::string& word, size_t startR1, size_t startR2)
{
    size_t size = word.size();
    if (word[size - 1] == 'e')
    {
        if (size - 1 >= startR2)
            word.pop_back();
        else if (size - 1 >= startR1 && !is_short(word.substr(0, size - 1)))
            word.pop_back();
    }
    else if (word[word.size() - 1] == 'l')
    {
        if (word.size() - 1 >= startR2 && word[word.size() - 2] == 'l')
            word.pop_back();
    }
}
}

void stem(std::string& word)
{
    // special case short words or sentence tags
    if (word.size() <= 2 || word == "<s>" || word == "</s>")
        return;

    // max word length is 35 for English
    if (word.size() > 35)
        word = word.substr(0, 35);

    if (word[0] == '\'')
        word = word.substr(1, word.size() - 1);

    if (special(word))
        return;

    change_y(word);
    size_t startR1 = get_start_r1(word);
    size_t startR2 = get_start_r2(word, startR1);

    step0(word);

    if (step1a(word))
    {
        std::replace(word.begin(), word.end(), 'Y', 'y');
        return;
    }

    step1b(word, startR1);
    step1c(word);
    step2(word, startR1);
    step3(word, startR1, startR2);
    step4(word, startR2);
    step5(word, startR1, startR2);

    std::replace(word.begin(), word.end(), 'Y', 'y');
    return;
}
}
}
}
}
