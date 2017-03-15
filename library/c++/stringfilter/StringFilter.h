/******************************************************************************
* Author: jaron.ho
* Date: 2014-04-10
* Brief: string filter
******************************************************************************/
#ifndef _STRING_FILTER_H_
#define	_STRING_FILTER_H_

#include <string>
#include <vector>
#include "Trie.h"

class StringFilter {
public:
	StringFilter(void);
	~StringFilter(void);

public:
	// load keywords from specific file
	bool load(const char* fileName);

	// parse keywords from specific file data
	bool parse(const char* fileData);

	// check if string contains keyword, return a new string that keyword is replaced by mask one by one
	std::string censor(std::string source, const char& mask = '*');

private:
	// replace src with dest in string, then return a new string
	std::string replace(std::string str, const std::string& src, const std::string& dest);

	// split string with pattern, return string array
	std::vector<std::string> split(std::string str, const std::string& pattern);

	// calculate character placeholder
	unsigned int characterPlaceholder(unsigned char ch);

private:
	bool mLoaded;
	Trie mTrie;
};

#endif	// _STRING_FILTER_H_