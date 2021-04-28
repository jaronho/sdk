/**********************************************************************
* 作者：hezhr
* 时间：2012-9-10
* 描述：异常
**********************************************************************/

#include "Exception.h"


//----------------------------------------------------------------------
Exception::Exception(const std::string &description, const std::string &func, const char *file, long line)
:mDescription(description)
,mFunction(func)
,mFile(file)
,mLine(line)
{
}
//----------------------------------------------------------------------
Exception::Exception(const Exception &rhs)
:mDescription(rhs.mDescription)
,mFunction(rhs.mFunction)
,mFile(rhs.mFile)
,mLine(rhs.mLine)
{
}
//----------------------------------------------------------------------
Exception::~Exception()
{
}
//----------------------------------------------------------------------
void Exception::operator = (const Exception &rhs)
{
	mDescription = rhs.mDescription;
	mFunction = rhs.mFunction;
	mFile = rhs.mFile;
	mLine = rhs.mLine;
}
//----------------------------------------------------------------------
const std::string& Exception::getDescription(void) const
{
	return mDescription;
}
//----------------------------------------------------------------------
const std::string& Exception::getFunction(void) const
{
	return mFunction;
}
//----------------------------------------------------------------------
const std::string& Exception::getFile(void) const
{
	return mFile;
}
//----------------------------------------------------------------------
long Exception::getLine(void) const
{
	return mLine;
}
//----------------------------------------------------------------------
const std::string& Exception::getFullDescription(void) const
{
	if (mFullDescription.empty())
	{
		char buf[128];
		::sprintf_s(buf, sizeof(buf), "%l", mLine);
		mFullDescription = "EXCEPTION: " + mDescription + " in " + mFunction + " at " + mFile + " (line " + buf + ")";
	}
	
	return mFullDescription;
}
//----------------------------------------------------------------------
const char* Exception::what() const
{
	return getFullDescription().c_str();
}
//----------------------------------------------------------------------


