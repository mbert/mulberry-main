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


// Abstract class for reading/writing an ordered sequence of bytes

#ifndef __WIN_LSTREAM__MULBERRY__
#define __WIN_LSTREAM__MULBERRY__

enum EStreamFrom {
  streamFrom_Start = 1,
  streamFrom_End,
  streamFrom_Marker
};


class LStream
{
 public:
  LStream();
  virtual  ~LStream();
 
  virtual void SetMarker(
			 SInt32  inOffset,
			 EStreamFrom inFromWhere);
  virtual SInt32 GetMarker() const;
 
  virtual void SetLength(
			 SInt32  inLength);
  virtual UInt32 GetLength() const;
 
  bool  AtEnd() const
    {
      return (UInt32)GetMarker() >= GetLength();
    }

  // Write Operations

  virtual ExceptionCode PutBytes(const void *inBuffer,
																 SInt32  &ioByteCount);
 
  SInt32  WriteData(
		   const void *inBuffer,
		   SInt32  inByteCount)
    {
      PutBytes(inBuffer, inByteCount);
      return inByteCount;
    }
    
  void  WriteBlock(
		   const void *inBuffer,
		   SInt32  inByteCount);
   

  // Read Operations
    
  virtual ExceptionCode GetBytes(
				 void  *outBuffer,
				 SInt32  &ioByteCount);
  virtual SInt32 ReadData(
			 void  *outBuffer,
			 SInt32  inByteCount)
    {
      GetBytes(outBuffer, inByteCount);
      return inByteCount;
    }
 
  void  ReadBlock(
		  void  *outBuffer,
		  SInt32  inByteCount);
   
  SInt32  PeekData(
		  void  *outButter,
		  SInt32  inByteCount);
   
 
  SInt32  mMarker;
  SInt32  mLength;
	void Abort() {} //I have no idea what this should do -- honus
};

#endif
