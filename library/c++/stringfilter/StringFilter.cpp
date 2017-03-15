/******************************************************************************
* Author: jaron.ho
* Date: 2014-04-10
* Brief: string filter
******************************************************************************/
#include "StringFilter.h"
//--------------------------------------------------------------------------
StringFilter::StringFilter(void) {
	mLoaded = false;
}
//--------------------------------------------------------------------------
StringFilter::~StringFilter(void){
}
//--------------------------------------------------------------------------
bool StringFilter::load(const char* filePath) {
	if (mLoaded) {
		return false;
	}
	FILE* fp = fopen(filePath, "rb");
	if (NULL == fp) {
		return false;
	}
	char buf[128];
	while (NULL != fgets(buf, sizeof(buf) - 1, fp)) {
		size_t wordLen = strlen(buf);
		for (size_t i=0; i<wordLen; ++i) {
			if ('\r' == buf[i] || '\n' == buf[i]) {
				buf[i] = '\0';
				break;
			}
		}
		mTrie.insert(replace(buf, " ", ""));
	}
	fclose(fp);
	mLoaded = true;
	return true;
}
//--------------------------------------------------------------------------
bool StringFilter::parse(const char* fileData) {
	if (mLoaded || NULL == fileData) {
		return false;
	}
	const std::vector<std::string>& strVec = split(replace(fileData, " ", ""), "\r\n");
	for (unsigned int i=0; i<strVec.size(); ++i) {
		if (strVec[i].empty()) {
			continue;
		}
		mTrie.insert(strVec[i]);
	}
	mLoaded = true;
	return true;
}
//--------------------------------------------------------------------------
std::string StringFilter::censor(std::string source, const char& mask) {
	if (!mLoaded) {
		return source;
	}
	unsigned int length = source.size();
	std::string target;
	std::string substring;
	std::string keyword;
	for (unsigned int start=0; start<length;) {
		substring = source.substr(start, length - start);
		keyword = mTrie.search(substring);
		if (keyword.empty()) {
			target += source[start];
			start += 1;
		} else {
			unsigned int ks = keyword.size();
			for (unsigned int i=0; i<ks;) {
				target += (' ' == keyword[i]) ? ' ' : mask;
				unsigned int count = characterPlaceholder(keyword[i]);
				i += count;
			}
			start += ks;
		}
	}
	return target;
}
//--------------------------------------------------------------------------
std::string StringFilter::replace(std::string str, const std::string& src, const std::string& dest) {
	unsigned int srclen = src.size();
	unsigned int destlen = dest.size();
	std::string::size_type pos = 0;
	while (std::string::npos != (pos = str.find(src, pos))) {
		str.replace(pos, srclen, dest);
		pos += destlen;
	}
	return str;
}
//--------------------------------------------------------------------------
std::vector<std::string> StringFilter::split(std::string str, const std::string& pattern) {
	std::vector<std::string> result;
	if (str.empty() || pattern.empty()) {
		return result;
	}
	str += pattern;		// extend string
	size_t strSize = str.size();
	size_t patternSize = pattern.size();
	std::string::size_type pos = 0;
	for (size_t i=0; i<strSize; ++i) {
		pos = str.find(pattern, i);
		if (pos < strSize) {
			result.push_back(str.substr(i, pos - i));
			i = pos + patternSize - 1;
		}
	}
	return result;
}
//--------------------------------------------------------------------------
unsigned int StringFilter::characterPlaceholder(unsigned char ch) {
	const unsigned char charsets[] = {0, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc};
	size_t i = sizeof(charsets);
	while (i > 0) {
		if (ch >= charsets[i - 1]) {
			return i;
		}
		--i;
	}
	return 1;
}
//--------------------------------------------------------------------------