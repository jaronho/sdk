/******************************************************************************
* Author: jaron.ho
* Date: 2014-04-10
* Brief: trie
******************************************************************************/
#include "Trie.h"
//--------------------------------------------------------------------------
Trie::Trie(size_t pace) {
	mEmptyRoot = new TrieNode();
	mPace = pace;
}
//--------------------------------------------------------------------------
Trie::~Trie(void) {
	if (mEmptyRoot) {
		delete mEmptyRoot;
		mEmptyRoot = NULL;
	}
}
//--------------------------------------------------------------------------
void Trie::insert(const std::string& keyword) {
	insert(mEmptyRoot, keyword);
}
//--------------------------------------------------------------------------
void Trie::insert(TrieNode* node, const std::string& keyword) {
	if (NULL == node) {
		return;
	}
	if (keyword.empty()) {
		node->setEnd(true);
		return;
	}
	const std::string& character = substr(keyword, 0, mPace);
	TrieNode* child = node->getChild(character);
	if (NULL == child) {
		insertBranch(node, keyword);
		return;
	}
	const std::string& restString = substr(keyword, mPace, keyword.size());
	insert(child, restString);
}
//--------------------------------------------------------------------------
void Trie::insertBranch(TrieNode* node, const std::string& keyword) {
	if (NULL == node) {
		return;
	}
	if (keyword.empty()) {
		node->setEnd(true);
		return;
	}
	const std::string& character = substr(keyword, 0, mPace);
	TrieNode* child = node->addChild(character);
	if (NULL == child) {
		return;
	}
	const std::string& restString = substr(keyword, mPace, keyword.size());
	insertBranch(child, restString);
}
//--------------------------------------------------------------------------
std::string Trie::search(const std::string& str) {
	return search(mEmptyRoot, str);
}
//--------------------------------------------------------------------------
std::string Trie::search(TrieNode* node, const std::string& str) {
	if (NULL == node || str.empty() || !node->hasChildren()) {
		return "";
	}
	std::string keyword = "";
	std::string tempCharacters = "";
	TrieNode* tempNode = node;
	unsigned int length = str.size();
	for (unsigned int start=0; start<length; start+=mPace) {
		const std::string& character = substr(str, start, mPace);
		TrieNode* child = tempNode->getChild(character);
		if (NULL == child) {
			if (" " == character) {
				if (!tempCharacters.empty()) {
					tempCharacters += character;
					continue;
				}
			}
			break;
		}
		tempCharacters += character;
		if (child->isEnd()) {
			keyword = tempCharacters;
		}
		tempNode = child;
	}
	return keyword;
}
//--------------------------------------------------------------------------
std::string Trie::substr(const std::string& str, size_t start, size_t len) {
	size_t str_size = str.size();
	if (0 == str_size || start >= str_size) {
		return "";
	}
	if (start + len >= str_size) {
		len = str_size - start;
	}
	return str.substr(start, len);
}
//--------------------------------------------------------------------------