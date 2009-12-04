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


#ifndef __CTCPSTREAM__MULBERRY__
#define __CTCPSTREAM__MULBERRY__

#include <iostream>
#include "CTLSSocket.h"

class CProgress;
class LStream;

class CTCPStreamBuf :  public std::streambuf, public CTLSSocket
{
public:
		CTCPStreamBuf();
		CTCPStreamBuf(const CTCPStreamBuf& copy);
		~CTCPStreamBuf() {};

	virtual void TCPOpen();
	virtual void TCPCloseConnection();
	int		is_open() const { return (TCPGetState() == TCPConnected); }

protected :
	virtual int 	overflow(int = EOF);
	virtual int 	sync();

	virtual int 	underflow();

private:
	char			mBufIn[cTCPBufferSize];
	char			mBufOut[cTCPBufferSize];

			int		flush_output();

};

class CTCPStream : virtual public CTCPStreamBuf,
					virtual public std::ostream,
					virtual public std::istream
{
public:
	CTCPStream();
	CTCPStream(const CTCPStream& copy);

	~CTCPStream() {};

	virtual void TCPOpen();

	virtual CTCPStream& qgetline(CTCPStreamBuf::char_type* s, std::streamsize n);
	virtual void gettostream(LStream& stream, std::ostream* log, long* len, CProgress* progress = NULL);
	virtual void gettostream(std::ostream& stream, std::ostream* log, long* len, CProgress* progress = NULL);
};

#endif
