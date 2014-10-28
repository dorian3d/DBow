/**
 * File: DbInfo.cpp
 * Date: April 2010
 * Author: Dorian Galvez
 * Description: class to encapsulate db info
 */

#include "DbInfo.h"
#include <string>
#include <sstream>
using namespace std;

using namespace DBow;

DbInfo::DbInfo(void)
{
}

DbInfo::DbInfo(const DbInfo &v)
{
	*this = v;
}

DbInfo::DbInfo(const VocInfo &v)
{
	this->VocInfo::operator =(v);
}

DbInfo::~DbInfo(void)
{
}


DbInfo& DbInfo::operator=(const DBow::DbInfo &v)
{
	this->VocInfo::operator =(v);
	EntryCount = v.EntryCount;

	return *this;
}

string DbInfo::toString() const
{
	stringstream ss;
	ss << "Vocabulary information:" << endl
		<< VocInfo::toString() << endl;

	ss << "Database information:" << endl
		<< "Number of entries: " << EntryCount << endl
		<< endl;

	return ss.str();
}

