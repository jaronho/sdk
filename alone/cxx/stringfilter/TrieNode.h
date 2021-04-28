/******************************************************************************
* Author: jaron.ho
* Date: 2014-04-10
* Brief: trie node
******************************************************************************/
#ifndef _TRIE_NODE_H_
#define	_TRIE_NODE_H_

#include <string>
#include <map>

class TrieNode {
public:
	TrieNode(const std::string& character = "");
	~TrieNode(void);
	
public:
	const std::string& getCharacter(void);

	void setEnd(bool end);

	bool isEnd(void);

	TrieNode* getParent(void);

	bool hasChildren(void);

	const std::map<std::string, TrieNode*>& getChildren(void);

	TrieNode* getChild(const std::string& character);

	TrieNode* addChild(const std::string& character);

private:
	std::string mCharacter;			// character of this node
	bool mEndOfKeyword;				// if is the end character of keyword
	TrieNode* mParent;
	std::map<std::string, TrieNode*> mChildren;
};

#endif	// _TRIE_NODE_H_