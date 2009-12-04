/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
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

#ifndef __UNX_LFILESTREAM__MULBERRY__
#define __UNX_LFILESTREAM__MULBERRY__

#include "UNX_LStream.h"
#include "cdstring.h"

#include <stdio.h>
#include <exception>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

class LFileStreamError: public std::exception {
 public:
  LFileStreamError(const char* why) : why_(why) {}
  virtual ~LFileStreamError() throw() {}
  virtual const char* what() const throw() {return why_;}
 protected:
  cdstring why_;
};
typedef LFileStreamError CFileException ;

class CantOpenFile : public LFileStreamError {
 public:
  CantOpenFile():LFileStreamError(strerror(errno)){}
};
class CantReadFromFile : public LFileStreamError {
 public:
  CantReadFromFile():LFileStreamError("Can't read from file."){}
};
class CantWriteToFile : public LFileStreamError {
 public:
  CantWriteToFile():LFileStreamError("Can't write to file."){}
};
class CantCloseFile : public LFileStreamError {
 public:
  CantCloseFile():LFileStreamError("Can't close file."){}
};


class	LFileStream : public LStream
{
public:
  LFileStream();
  LFileStream(const char* inFileName, int flags = O_RDWR | O_CREAT);
  virtual ~LFileStream();
  
  virtual void SetMarker( SInt32   inOffset, EStreamFrom  inFromWhere);
  virtual SInt32  GetMarker() const;

	virtual void SeekToBegin() {lseek(theFilePointer, 0, SEEK_SET);}
	virtual void SeekToEnd() {lseek(theFilePointer, 0, SEEK_END);}
  
  virtual void  SetLength(SInt32   inLength);
  virtual UInt32  GetLength() const;
  
	virtual void SetSpecifier(const char* spec);
	virtual cdstring& GetSpecifier() {return mFileName;}
  virtual ExceptionCode PutBytes(const void  *inBuffer,
																 SInt32   &ioByteCount);
  virtual ExceptionCode GetBytes(void   *outBuffer,
																 SInt32   &ioByteCount);
  virtual void Close();
	virtual const cdstring& GetFilePath() const {return mFileName;}//the entire path, including filename
	virtual const cdstring& GetFileName() const; //just the filename part
	void Abort() {}; //does nothing, here for Win32 compatability
	void GetStatus(struct stat& buf);
  void Open(const char* name, int flags = O_RDWR | O_CREAT);
 private:
  int theFilePointer;
 protected:
	cdstring mFileName;
	mutable cdstring mFileNamePart;
};

#endif
