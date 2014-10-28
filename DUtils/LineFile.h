/*	
 * File: LineFile.h
 * Project: DUtils library
 * Author: Dorian Galvez
 * Date: October 6, 2009
 * Description: reads and writes text line files
 *
 */

#pragma once
#ifndef __D_LINE_FILE__
#define __D_LINE_FILE__

#include "DException.h"
#include "FileModes.h"
#include <vector>
#include <fstream>
using namespace std;

namespace DUtils {

class LineFile
{
public:
	/* Creates a linefile with no file
	 */
	LineFile(void);

	/* Closes any opened file
	*/
	~LineFile(void);

	/* Creates a linefile by opening a file
	 * @param filename
	 * @param mode: READ or WRITE
	 * @throws DException if cannot open the file
	 */
	LineFile(const char *filename, const FILE_MODES mode);
	LineFile(const string &filename, const FILE_MODES mode);

	/* Opens a file for reading. It closes any other opened file
	 * @param filename
	 * @throws DException if cannot create the file
	 */
	void OpenForReading(const char *filename);
	inline void OpenForReading(const string &filename)
	{
		OpenForReading(filename.c_str());
	}

	/* Opens a file for writing. It closes any other opened file
	 * @param filename
	 * @throws DException if cannot create the file
	 */
	void OpenForWriting(const char *filename);
	inline void OpenForWriting(const string &filename)
	{
		OpenForWriting(filename.c_str());
	}

	/* Opens a file for writing at the end. It closes any other opened file
	 * @param filename
	 * @throws DException if cannot open the file
	 */
	void OpenForAppending(const char *filename);
	inline void OpenForAppending(const string &filename)
	{
		OpenForAppending(filename.c_str());
	}

	/* Says whether the end of the file has been reached. It is not necessary
	 * to read a last empty line to reach eof
	 * @return true iif there is nothing else to read
	 * @thows DException if wrong access mode
	 */
	bool Eof();

	/* Closes any opened file. It is not necessary to call this function
	 * explicitly
	 */
	void Close();

	/* Writes a line
	 * @thows DException if wrong access mode
	 */
	LineFile& operator<< (const char *s);
	inline LineFile& operator<< (const string &s)
	{
		return this->operator <<(s.c_str()); 
	}

	/* Reads a line
	 * @param s[]: buffer to store the string. Memory must be allocated
	 * @param s: string to write on
	 * @thows DException if wrong access mode
	 */
	LineFile& operator>> (string &s);

	/* Writes several lines at a time
	 * @v: vector of line strings
	 * @throws DException if wrong access mode
	 */
	void Dump(const vector<string> &v);
	inline LineFile& operator<< (const vector<string> &v)
	{
		Dump(v);
		return *this;
	}

	/* In reading mode, reads and throws away the next line
	 */
	void DiscardLine();

protected:

	/**
	 * Initializes the object by opening a file
	 * @param filename file to open
	 * @param mode opening mode
	 * @throws DException if cannot open the file
	 */
	void Init(const char *filename, const FILE_MODES mode);

protected:
	FILE_MODES m_mode;		// opening mode
	fstream m_f;			// fstream
	string m_next_line;	// next line to read
};

}

#endif

