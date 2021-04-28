/******************************************************************************
* Author: jaron.ho
* Date: 2014-04-10
* Brief: trie node
******************************************************************************/
#include "TrieNode.h"
//--------------------------------------------------------------------------
TrieNode::TrieNode(const std::string& character) {
	mCharacter = character;
	mEndOfKeyword = false;
	mParent = NULL;
}
//--------------------------------------------------------------------------
TrieNode::~TrieNode(void) {
	std::map<std::string, TrieNode*>::iterator iter = mChildren.begin();
	std::map<std::string, TrieNode*>::iterator end = mChildren.end();
	for (; end!=iter; ++iter) {
		iter->second->mParent = NULL;
		delete iter->second;
		iter->second = NULL;
	}
	mChildren.clear();
}
//--------------------------------------------------------------------------
const std::string& TrieNode::getCharacter(void) {
	return mCharacter;
}
//--------------------------------------------------------------------------
void TrieNode::setEnd(bool end) {
	mEndOfKeyword = end;
}
//--------------------------------------------------------------------------
bool TrieNode::isEnd(void) {
	return mEndOfKeyword;
}
//--------------------------------------------------------------------------
TrieNode* TrieNode::getParent(void) {
	return mParent;
}
//--------------------------------------------------------------------------
bool TrieNode::hasChildren(void) {
	return !mChildren.empty();
}
//--------------------------------------------------------------------------
const std::map<std::string, TrieNode*>& TrieNode::getChildren(void) {
	return mChildren;
}
//--------------------------------------------------------------------------
TrieNode* TrieNode::getChild(const std::string& character) {
	std::map<std::string, TrieNode*>::iterator iter = mChildren.find(character);
	if (mChildren.end() == iter) {
		return NULL;
	}
	return iter->second;
}
//--------------------------------------------------------------------------
TrieNode* TrieNode::addChild(const std::string& character) {
	if (NULL == getChild(character)) {
		TrieNode* node = new TrieNode(character);
		node->mParent = this;
		mChildren.insert(std::pair<std::string, TrieNode*>(character, node));
		return node;
	}
	return NULL;
}
//--------------------------------------------------------------------------