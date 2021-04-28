/******************************************************************************
* Author: jaron.ho
* Date: 2014-04-10
* Brief: trie
******************************************************************************/
#ifndef _TRIE_H_
#define	_TRIE_H_

#include <string>
#include "TrieNode.h"

class Trie {
public:
	Trie(size_t pace = 1);
	~Trie(void);

public:
	void insert(const std::string& keyword);

	std::string search(const std::string& str);

private:
	void insert(TrieNode* node, const std::string& keyword);

	void insertBranch(TrieNode* node, const std::string& keyword);

	std::string search(TrieNode* node, const std::string& str);

	std::string substr(const std::string& str, size_t start, size_t len);

private:
	TrieNode* mEmptyRoot;			// root node of trie
	unsigned int mPace;				// how many characters that a trie node contains
};

#endif	// _TRIE_H_