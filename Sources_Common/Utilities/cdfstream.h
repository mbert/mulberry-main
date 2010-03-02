/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 **  fstream
 **/

#ifndef _CDFSTREAM
#define _CDFSTREAM

#include "CUnicodeStdLib.h"

#if !defined(__GNUC__) && !defined(__VCPP__)
#include <mslconfig>
#else
#define MSL_FAKE_STLPORT
#endif

#ifndef _MSL_NO_IO

#include <iosfwd>
#include <streambuf>
#include <ostream>
#include <istream>
#include <cstdio>
#include <cstring>

#ifndef RC_INVOKED

#pragma options align=native
#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import on
#endif

#ifndef _MSL_NO_CPP_NAMESPACE
//	namespace std {
#endif

static const unsigned long cFileBufferSize = 8192;

template <class charT, class traits>
class basic_cdfilebuf
: public std::basic_streambuf<charT, traits>
{
public:
	typedef charT                     char_type;
	typedef typename traits::int_type int_type;
	typedef typename traits::pos_type pos_type;
	typedef typename traits::off_type off_type;
	typedef traits                    traits_type;

	//  lib.filebuf.cons Constructors/destructor:
	// Nonstandard, but serves as standard default constructor
	basic_cdfilebuf(FILE* file = 0, unsigned long bufsize = cFileBufferSize);
	virtual ~basic_cdfilebuf();

	//  lib.filebuf.members Members:
	bool is_open() const;
	basic_cdfilebuf* open(const char* s, std::ios_base::openmode mode);
	basic_cdfilebuf* close();

protected:
	//  lib.filebuf.virtuals Overridden virtual functions:
	// virtual streamsize showmanyc();  // use basic_streambuf
	virtual int_type underflow();
	virtual int_type pbackfail(int_type c = traits::eof());
	virtual int_type overflow (int_type c = traits::eof());

	// virtual basic_streambuf<charT,traits>* setbuf(char_type* s, streamsize n);  // default behavior which should be "do nothing"
	virtual pos_type seekoff(off_type off, std::ios_base::seekdir way,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);
	virtual pos_type seekpos(pos_type sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out); // set overflow_called_last_
	virtual int      sync();
private:
	enum EReadWriteState
	{
		eNeutral = 0,
		eRead,
		eWrite
	};

	FILE* file_;
	std::ios_base::openmode mode_;
	char_type* buf_;
	unsigned long buf_size_;
#ifdef MSL_FAKE_STLPORT
	long f_pos_;
#else
	fpos_t f_pos_;
#endif
	EReadWriteState rw_;
};

template <class charT, class traits>
class basic_cdifstream
	: public std::basic_istream<charT, traits>
{
public:
	typedef charT                     char_type;
	typedef typename traits::int_type int_type;
	typedef typename traits::pos_type pos_type;
	typedef typename traits::off_type off_type;
	typedef traits                    traits_type;

	//  lib.ifstream.cons Constructors:
	basic_cdifstream(unsigned long bufsize = cFileBufferSize);
	explicit basic_cdifstream(const char* s,
								std::ios_base::openmode mode = std::ios_base::in,
								unsigned long bufsize = cFileBufferSize);

	//  lib.ifstream.members Members:
	basic_cdfilebuf<charT, traits>* rdbuf() const;

	bool is_open();
	void open(const char* s, std::ios_base::openmode mode = std::ios_base::in);
	void close();
private:
	basic_cdfilebuf<charT, traits> sb_;
};

template <class charT, class traits>
class basic_cdofstream
	: public std::basic_ostream<charT, traits>
{
public:
	typedef charT                     char_type;
	typedef typename traits::int_type int_type;
	typedef typename traits::pos_type pos_type;
	typedef typename traits::off_type off_type;
	typedef traits                    traits_type;

	//  lib.ofstream.cons Constructors:
	basic_cdofstream(unsigned long bufsize = cFileBufferSize);
	explicit basic_cdofstream(const char* s,
								std::ios_base::openmode mode = std::ios_base::out,
								unsigned long bufsize = cFileBufferSize);

	//  lib.ofstream.members Members:
	basic_cdfilebuf<charT,traits>* rdbuf() const;

	bool is_open();
	void open(const char* s, std::ios_base::openmode mode = std::ios_base::out);
	void close();
private:
	basic_cdfilebuf<charT, traits> sb_;
};

template <class charT, class traits>
class basic_cdfstream
	: public std::basic_iostream<charT, traits>
{
public:
	typedef charT                     char_type;
	typedef typename traits::int_type int_type;
	typedef typename traits::pos_type pos_type;
	typedef typename traits::off_type off_type;
	typedef traits                    traits_type;

	//  constructors/destructor
	basic_cdfstream(unsigned long bufsize = cFileBufferSize);
	explicit basic_cdfstream(const char* s,
								std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out,
								unsigned long bufsize = cFileBufferSize);

	//  Members:
	basic_cdfilebuf<charT,traits>* rdbuf() const;
	bool is_open();
	void open(const char* s, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
	void close();
private:
	basic_cdfilebuf<charT, traits> sb_;
};

// basic_cdfilebuf Implementation

// Nonstandard, but serves as standard default constructor

template <class charT, class traits>
basic_cdfilebuf<charT, traits>::basic_cdfilebuf(FILE* file, unsigned long bufsize)
	: file_(file),
	  mode_(std::ios_base::openmode(0)),
	  buf_(NULL),
	  buf_size_(bufsize),
	  f_pos_(0),
	  rw_(eNeutral)
{
}

template <class charT, class traits>
inline
basic_cdfilebuf<charT, traits>::~basic_cdfilebuf()
{
	if (file_ != stdin && file_ != stdout && file_ != stderr)
		close();
	else
		delete [] buf_;
}

template <class charT, class traits>
inline
bool
basic_cdfilebuf<charT, traits>::is_open() const
{
	return file_ != 0;
}

template <class charT, class traits>
basic_cdfilebuf<charT,traits>*
basic_cdfilebuf<charT, traits>::open(const char* s, std::ios_base::openmode mode)
{
	if (file_ != 0)
		return 0;
	const char* modstr;
	mode_ = mode;
	switch (mode & ~std::ios_base::ate)
	{
	case std::ios_base::out:
	case (int)std::ios_base::out | (int)std::ios_base::trunc:
		modstr = "w";
		break;
	case (int)std::ios_base::out | (int)std::ios_base::app:
		modstr = "a";
		break;
	case std::ios_base::in:
		modstr = "r";
		break;
	case (int)std::ios_base::in | (int)std::ios_base::out:
		modstr = "r+";
		break;
		case (int)std::ios_base::in | (int)std::ios_base::out | (int)std::ios_base::trunc:
		modstr = "w+";
		break;
	case (int)std::ios_base::binary | (int)std::ios_base::out:
	case (int)std::ios_base::binary | (int)std::ios_base::out | (int)std::ios_base::trunc:
		modstr = "wb";
		break;
	case (int)std::ios_base::binary | (int)std::ios_base::out | (int)std::ios_base::app:
		modstr = "ab";
		break;
	case (int)std::ios_base::binary | (int)std::ios_base::in:
		modstr = "rb";
		break;
	case (int)std::ios_base::binary | (int)std::ios_base::in | (int)std::ios_base::out:
		modstr = "r+b";
		break;
	case (int)std::ios_base::binary | (int)std::ios_base::in | (int)std::ios_base::out | (int)std::ios_base::trunc:
		modstr = "w+b";
		break;
	default:
		return 0;
	}
	file_ = fopen_utf8(s, modstr);
	if (file_ == 0)
		return 0;
	if (mode & std::ios_base::ate && fseek(file_, 0, SEEK_END))
	{
		close();
		return 0;
	}

	// Create default buffer
	buf_ = new char_type[buf_size_];
	f_pos_ = 0;
	rw_ = eNeutral;
	return this;
}

template <class charT, class traits>
basic_cdfilebuf<charT, traits>*
basic_cdfilebuf<charT, traits>::close()
{
	if (file_ == 0)
		return 0;
	
	// Must sync - don't let this fail otherwise we cannot close the file
	sync();
	//if (sync())
	//	return 0;

	basic_cdfilebuf<charT, traits>* result = this;
	if (fclose(file_) != 0)
		result = 0;
	file_ = 0;

	// Clear get/put area
	this->setg(NULL, NULL, NULL);
	this->setp(NULL, NULL);

	delete [] buf_;
	buf_ = NULL;

	f_pos_ = 0;
	rw_ = eNeutral;

	return result;
}

template <class charT, class traits>
typename basic_cdfilebuf<charT, traits>::int_type
basic_cdfilebuf<charT, traits>::underflow()
{
	if (file_ == 0)
		return traits::eof();
	
	// Check for change in read-write state
	if (rw_ != eRead)
	{
		// Check for previous write
		if (rw_ == eWrite)
		{
			// Flush output buffer
			sync();
			
			// Clear put area
			this->setp(NULL, NULL);
		}
		
		rw_ = eRead;
	}

	// Read data from the file
	f_pos_ = ::ftell(file_);
	size_t nread = ::fread(buf_, sizeof(char_type), buf_size_, file_);

	if (nread)
	{
		// Set get buffer based on the number actually read in
		this->setg(buf_, buf_, buf_ + nread);

		return traits::to_int_type(*buf_);
	}
	else
	{
		// Clear get area
		this->setg(NULL, NULL, NULL);

		return traits::eof();
	}
}

template <class charT, class traits>
typename basic_cdfilebuf<charT, traits>::int_type
basic_cdfilebuf<charT, traits>::pbackfail(int_type c)
{
	if (file_ == 0 || this->gptr() <= this->eback())
		return traits::eof();
	this->gbump(-1);
	if (!traits::eq_int_type(c, traits::eof()))
		*this->gptr() = traits::to_char_type(c);
	return traits::not_eof(c);
}

template <class charT, class traits>
typename basic_cdfilebuf<charT, traits>::int_type
basic_cdfilebuf<charT, traits>::overflow(int_type c)
{
	if (file_ == 0)
		return traits::eof();
	if (traits::eq_int_type(c, traits::eof()))
		return traits::not_eof(c);

	// Check for change in read-write state
	if (rw_ != eWrite)
	{
		// Check for previous write
		if (rw_ == eWrite)
		{
			// Clear get area
			this->setg(NULL, NULL, NULL);
		}
		
		rw_ = eWrite;
	}

	// Write data to the file
	size_t numwrite = this->pptr() - this->pbase();
	size_t nwrite = numwrite ? ::fwrite(buf_, sizeof(char_type), numwrite, file_) : 0;
	f_pos_ = ::ftell(file_);

	if (nwrite == numwrite)
	{
		// Set put buffer to entire buffer
		this->setp(buf_, buf_ + buf_size_);
		*this->pptr() = (char)c;
		this->pbump(1);

		return c;
	}
	else
		return traits::eof();
}

template <class charT, class traits>
typename basic_cdfilebuf<charT, traits>::pos_type
basic_cdfilebuf<charT, traits>::seekoff(off_type off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
	if (file_ == 0)
		return pos_type(-1);

	std::ios_base::openmode inout = std::ios_base::in | std::ios_base::out;
	if ((((which & inout) == inout) && (way == std::ios_base::cur)) || ((which & inout) == 0))
		return pos_type(-1);

	// Special for tellg/tellp
	if ((way == std::ios_base::cur) && (off == 0))
	{
		switch(rw_)
		{
		case eRead:
			return f_pos_ + this->gptr() - this->eback();
		case eWrite:
			return f_pos_ + this->pptr() - this->pbase();
		case eNeutral:
		default:
			return f_pos_;
		}
	}

	// Always flush if writing
	if (rw_ == eWrite)
		sync();

	// Now check internal mode
	if (which & std::ios_base::in && (rw_ != eRead))
	{
		// See if write was done last
		if (rw_ == eWrite)
		{
			// Adjust actual file pos
			f_pos_ += this->pptr() - this->pbase();

			// Clear put area
			this->setp(NULL, NULL);
		}

		// Set to neutral
		rw_ = eNeutral;
	}
	else if (which & std::ios_base::out && (rw_ != eWrite))
	{
		// See if read was done last
		if (rw_ == eRead)
		{
			// Adjust actual file pos
			f_pos_ += this->gptr() - this->eback();
			
			// Clear get area
			this->setg(NULL, NULL, NULL);
		}

		// Set to neutral
		rw_ = eNeutral;
	}

	off_type absoff;
	int whence;
	switch (way)
	{
	case std::ios_base::beg:
		absoff = off;
		whence = SEEK_SET;
		break;
	case std::ios_base::cur:
		switch(rw_)
		{
		case eNeutral:
		default:
			absoff = f_pos_ + off;
			break;
		case eRead:
			absoff = f_pos_ + this->gptr() - this->eback() + off;
			break;
		case eWrite:
			absoff = f_pos_ + this->pptr() - this->pbase() + off;
			break;
		}
		whence = SEEK_SET;
		break;
	case std::ios_base::end:
		absoff = off;
		whence = SEEK_END;
		break;
	default:
		return pos_type(-1);
	}

	// Check input or output
	switch(rw_)
	{
	case eNeutral:
	default:
		// Seek file
		if (::fseek(file_, absoff, whence))
			return pos_type(-1);
		f_pos_ = ::ftell(file_);
		return pos_type(f_pos_);
	case eRead:
		if ((whence == SEEK_END) ||
			(absoff >= (f_pos_ + this->egptr() - this->eback())) ||
			(absoff < f_pos_))
		{
			// Invalidate get buffer to force underflow
			this->setg(NULL, NULL, NULL);
			
			// Seek file
			if (::fseek(file_, absoff, whence))
				return pos_type(-1);
			f_pos_ = ::ftell(file_);
		}
		else
			// Advance gnext
			this->setg(this->eback(), absoff - f_pos_ + this->eback(), this->egptr());
		return pos_type(f_pos_ + this->gptr() - this->eback());
	case eWrite:
		if ((whence == SEEK_END) ||
			(absoff >= (f_pos_ + this->pptr() - this->pbase())) ||
			(absoff < f_pos_))
		{
			// Flush current
			sync();

			// Invalidate put buffer to force overflow
			this->setp(NULL, NULL);
			
			// Seek file
			if (::fseek(file_, absoff, whence))
				return pos_type(-1);
			f_pos_ = ::ftell(file_);
		}
		else
		{
			// Advance pnext
			this->setp(this->pbase(), this->epptr());
			this->pbump(absoff - f_pos_);
		}
		return pos_type(f_pos_ + this->pptr() - this->pbase());
	}
}

template <class charT, class traits>
typename basic_cdfilebuf<charT, traits>::pos_type
basic_cdfilebuf<charT, traits>::seekpos(pos_type sp, std::ios_base::openmode which)
{
	return seekoff(sp, std::ios_base::beg, which);
}

template <class charT, class traits>
inline
int
basic_cdfilebuf<charT, traits>::sync()
{
	if (file_ == 0)
		return -1;
	
	// Check for pending output
	if ((rw_ == eWrite) &&
		(this->pptr() != this->pbase()))
	{
		// Write remainder to file as an overflow with a dummy char
		if (traits::eq_int_type(overflow(0), traits::eof()))
			return -1;
		
		// Reset put area
		this->setp(buf_, buf_ + buf_size_);
	}

	// Flush the file itself
	if (::fflush(file_))
		return -1;

	return 0;
}

// basic_cdifstream Implementation

template <class charT, class traits>
basic_cdifstream<charT, traits>::basic_cdifstream(unsigned long bufsize)
#ifdef MSL_FAKE_STLPORT
	: std::basic_istream<charT, traits>(0),
		sb_(NULL, bufsize)
#else
	: sb_(NULL, bufsize),
		std::basic_istream(&sb_)
#endif
{
#ifdef MSL_FAKE_STLPORT
      this->init(&sb_);
#endif
}

template <class charT, class traits>
basic_cdifstream<charT, traits>::basic_cdifstream(const char* s, std::ios_base::openmode mode, unsigned long bufsize)
#ifdef MSL_FAKE_STLPORT
	: std::basic_istream<charT, traits>(0),
		sb_(NULL, bufsize)
#else
	: sb_(NULL, bufsize),
		std::basic_istream(&sb_)
#endif
{
#ifdef MSL_FAKE_STLPORT
      this->init(&sb_);
#endif
      if (rdbuf()->open(s, mode | std::ios_base::in) == 0)
	this->setstate(std::ios_base::failbit);
}

template <class charT, class traits>
inline
basic_cdfilebuf<charT, traits>*
basic_cdifstream<charT, traits>::rdbuf() const
{
	return const_cast<basic_cdfilebuf<charT, traits>*>(&sb_);
}

template <class charT, class traits>
inline
bool
basic_cdifstream<charT, traits>::is_open()
{
	return rdbuf()->is_open();
}

template <class charT, class traits>
inline
void
basic_cdifstream<charT, traits>::open(const char* s, std::ios_base::openmode mode)
{
  if (rdbuf()->open(s, mode | std::ios_base::in) == 0)
    this->setstate(std::ios_base::failbit);
}

template <class charT, class traits>
inline
void
basic_cdifstream<charT, traits>::close()
{
	if (rdbuf()->close() == 0)
	  this->setstate(std::ios_base::failbit);
}

// basic_cdofstream Implementation

template <class charT, class traits>
basic_cdofstream<charT, traits>::basic_cdofstream(unsigned long bufsize)
#ifdef MSL_FAKE_STLPORT
	: std::basic_ostream<charT, traits>(0),
		sb_(NULL, bufsize)
#else
	: sb_(NULL, bufsize),
		std::basic_ostream(&sb_)
#endif
{
#ifdef MSL_FAKE_STLPORT
      this->init(&sb_);
#endif
}

template <class charT, class traits>
basic_cdofstream<charT, traits>::basic_cdofstream(const char* s, std::ios_base::openmode mode, unsigned long bufsize)
#ifdef MSL_FAKE_STLPORT
	: std::basic_ostream<charT, traits>(0),
		sb_(NULL, bufsize)
#else
	: sb_(NULL, bufsize),
		std::basic_ostream(&sb_)
#endif
{
#ifdef MSL_FAKE_STLPORT
      this->init(&sb_);
#endif
      if (rdbuf()->open(s, mode | std::ios_base::out) == 0)
	this->setstate(std::ios_base::failbit);
}

template <class charT, class traits>
inline
basic_cdfilebuf<charT,traits>*
basic_cdofstream<charT, traits>::rdbuf() const
{
	return const_cast<basic_cdfilebuf<charT, traits>*>(&sb_);
}

template <class charT, class traits>
inline
bool
basic_cdofstream<charT, traits>::is_open()
{
	return rdbuf()->is_open();
}

template <class charT, class traits>
inline
void
basic_cdofstream<charT, traits>::open(const char* s, std::ios_base::openmode mode)
{
  if (rdbuf()->open(s, mode | std::ios_base::out) == 0)
    this->setstate(std::ios_base::failbit);
}

template <class charT, class traits>
inline
void
basic_cdofstream<charT, traits>::close()
{
	if (rdbuf()->close() == 0)
	  this->setstate(std::ios_base::failbit);
}

// basic_cdfstream Implementation

template <class charT, class traits>
basic_cdfstream<charT, traits>::basic_cdfstream(unsigned long bufsize)
#ifdef MSL_FAKE_STLPORT
	: std::basic_iostream<charT, traits>(0),
		sb_(NULL, bufsize)
#else
	: sb_(NULL, bufsize),
		std::basic_iostream(&sb_)
#endif
{
#ifdef MSL_FAKE_STLPORT
      this->init(&sb_);
#endif
}

template <class charT, class traits>
basic_cdfstream<charT, traits>::basic_cdfstream(const char* s, std::ios_base::openmode mode, unsigned long bufsize)
#ifdef MSL_FAKE_STLPORT
	: std::basic_iostream<charT, traits>(0),
		sb_(NULL, bufsize)
#else
	: sb_(NULL, bufsize),
		std::basic_iostream(&sb_)
#endif
{
#ifdef MSL_FAKE_STLPORT
      this->init(&sb_);
#endif
	if (rdbuf()->open(s, mode) == 0)
	  this->setstate(std::ios_base::failbit);
}

template <class charT, class traits>
inline
basic_cdfilebuf<charT,traits>*
basic_cdfstream<charT, traits>::rdbuf() const
{
	return const_cast<basic_cdfilebuf<charT, traits>*>(&sb_);
}

template <class charT, class traits>
inline
bool
basic_cdfstream<charT, traits>::is_open()
{
	return rdbuf()->is_open();
}

template <class charT, class traits>
inline
void
basic_cdfstream<charT, traits>::open(const char* s, std::ios_base::openmode mode)
{
	if (rdbuf()->open(s, mode) == 0)
	  this->setstate(std::ios_base::failbit);
}

template <class charT, class traits>
inline
void
basic_cdfstream<charT, traits>::close()
{
	if (rdbuf()->close() == 0)
	  this->setstate(std::ios_base::failbit);
}

typedef basic_cdfilebuf<char, std::char_traits<char> >  cdfilebuf;
typedef basic_cdifstream<char, std::char_traits<char> > cdifstream;
typedef basic_cdofstream<char, std::char_traits<char> > cdofstream;
typedef basic_cdfstream<char, std::char_traits<char> >  cdfstream;

#ifndef _MSL_NO_CPP_NAMESPACE
//	} // namespace std
#endif

#if defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
	#pragma import reset
#endif
#pragma options align=reset

#endif // RC_INVOKED

#endif // _MSL_NO_IO

#endif // _FSTREAM

// hh 971220 fixed MOD_INCLUDE
// hh 971221 Changed filename from fstream.h to fstream
// hh 971221 Made include guards standard
// hh 971229 deleted unused arguments from setbuf
// hh 971230 added RC_INVOKED wrapper
// hh 980108 added explicit bool cast in case it is typedefed to int in is_open
// hh 980129 changed to <istream> from <iostream>
// hh 981220 Added typename to appropriate return types
// hh 990104 Rewrote
