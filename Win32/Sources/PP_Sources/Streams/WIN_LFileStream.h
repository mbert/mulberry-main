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


#ifndef __WIN_LFILESTREAM__MULBERRY__
#define __WIN_LFILESTREAM__MULBERRY__

#include <WIN_LStream.h>

class	LFileStream : public LStream,
						public CFile
{
public:
						LFileStream();
						LFileStream(
								LPCTSTR	inFileName,
								UINT flags);
	virtual				~LFileStream();
	
	virtual void		SetMarker(
								SInt64			inOffset,
								EStreamFrom		inFromWhere);
	virtual SInt64		GetMarker() const;

	virtual void		SetLength(
								SInt64			inLength);
	virtual UInt64		GetLength() const;

	virtual ExceptionCode	PutBytes(
								const void		*inBuffer,
								SInt32			&ioByteCount);
	virtual ExceptionCode	GetBytes(
								void			*outBuffer,
								SInt32			&ioByteCount);

};

#endif
