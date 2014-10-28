/*	
 * File: DException.h
 * Project: DUtils library
 * Author: Dorian Galvez
 * Date: October 6, 2009
 * Description: general exception of the library
 *
 */

#pragma once

#ifndef __D_EXCEPTION__
#define __D_EXCEPTION__

#include <stdexcept>
#include <string>
using namespace std;

namespace DUtils {

class DException :
	public exception
{
public:
	/* Creates an exception with a general error message
	 */
	DException(void) throw(): m_message("DUtils exception"){}

	/* Creates an exception with a custom error message
	 * @param msg: message
	 */
	DException(const char *msg) throw(): m_message(msg){}
	DException(const string &msg) throw(): m_message(msg){}

	~DException(void) throw(){}

	/* Returns the exception message
	 * @overrides exception::what
	 */
	virtual const char* what() const throw()
	{
		return m_message.c_str();
	}

protected:
	string m_message;
};

}

#endif

