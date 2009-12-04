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


#include "CTCPStream.h"

#include "CErrorHandler.h"
#include "CGeneralException.h"
#include "CLog.h"
#include "CProgress.h"

#if __dest_os == __win32_os
#include <WIN_LStream.h>
#elif __dest_os == __linux_os
#include "UNX_LStream.h"
#endif

#include <iomanip>

CTCPStream::CTCPStream() :
			std::ostream(0),
			std::istream(0),
			std::ios(0)
{
	rdbuf(this);

	// Turn on exception propogation
	exceptions(badbit | failbit);
}

CTCPStream::CTCPStream(const CTCPStream& copy) :
			std::ostream(0),
			std::istream(0),
			std::ios(0),
			CTCPStreamBuf(copy)
{
	rdbuf(this);

	// Turn on exception propogation
	exceptions(badbit | failbit);
}

// Open TCP comms
void CTCPStream::TCPOpen()
{
	// Clear any previous exception state
	clear();

	CTCPStreamBuf::TCPOpen();
}

CTCPStream& CTCPStream::qgetline (CTCPStreamBuf::char_type* s, std::streamsize n)
{
	std::istream::sentry ok(*this, true);

	if (s == 0 || n <= 0)
	{
		setstate(failbit);
		return *this;
	}

	if (ok)
	{
		iostate err = goodbit;
		try
		{
			while (true)
			{
				CTCPStreamBuf::int_type ci = rdbuf()->sbumpc();
				if (CTCPStreamBuf::traits_type::eq_int_type (ci, CTCPStreamBuf::traits_type::eof ()))
				{
					err |= eofbit;
					break;
				}
				CTCPStreamBuf::char_type c = CTCPStreamBuf::traits_type::to_char_type(ci);
				if (CTCPStreamBuf::traits_type::eq(c, '\n'))
				{
					//chcount++;
					break;
				}
				if (CTCPStreamBuf::traits_type::eq(c, '\r'))
				{
					//chcount++;
					continue;
				}
				if (!--n)
				{
					rdbuf()->sputbackc (c);
					err |= failbit;
					break;
				}
				*s++ = c;
				//chcount++;
			}
			*s = CTCPStreamBuf::char_type();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Important - do not let this failure throw an exception!!!
			// Any exception here is not handled properly by caller
			try
			{
#ifdef __GNUC__
				setstate(badbit);
#else
				state() |= badbit;
#endif
			}
			catch(...)
			{
				// Ignore
			}
			// Always throw up
			CLOG_LOGRETHROW;
			throw;
		}

		// Important - do not let this failure throw an exception!!!
		// Any exception here is not handled
		try
		{
#ifdef __GNUC__
			setstate(err);
#else
			state() = err;
#endif
		}
		catch(...)
		{
			// Ignore stream exception
		}

	}

	return *this;
}

void CTCPStream::gettostream(LStream& stream, std::ostream* log, long* len, CProgress* progress)
{
	char tmp[cTCPBufferSize];
	long total = *len;
	long remaining = *len;

	// Ignore progress if length is within single buffer size
	if (progress && (total <= cTCPBufferSize))
		progress = NULL;

	// Initialise progress
	if (progress)
		progress->SetPercentage(0);

	bool err_once = false;
	while(remaining)
	{
		// Determine next amount to read
		long rcv_len = remaining;
		if (rcv_len > cTCPBufferSize)
			rcv_len = cTCPBufferSize;

		// Read in from streambuf
		read(tmp, rcv_len);

		// Do not allow failure on write
		try
		{
			// Give to stream and adjust length
			if (!err_once)
				stream.WriteData(tmp, rcv_len);
			remaining -= rcv_len;
			
			// Do log
			if (log != NULL)
				log->write(tmp, rcv_len);
		}
		catch (CGeneralException& ex)
		{
			CLOG_LOGCATCH(CGeneralException&);

			// Dump handle from stream
			stream.SetLength(0);

			// Try alert
			if (!err_once)
			{
				CErrorHandler::PutOSErrAlertRsrc("Error::IMAP::OSErrReadMsg", ex.GetErrorCode());

				err_once = true;
			}

			// Ignore
			remaining -= rcv_len;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Ignore
			remaining -= rcv_len;
		}
		
		// Update progress
		if (progress)
			progress->SetPercentage((unsigned long)(((total - remaining) * 100.0) / total));
	}

	// Dump handle from stream if error
	if (err_once)
		stream.SetLength(0);
}

void CTCPStream::gettostream(std::ostream& stream, std::ostream* log, long* len, CProgress* progress)
{
	char tmp[cTCPBufferSize];
	long total = *len;
	long remaining = *len;

	// Initialise progress
	if (progress)
		progress->SetPercentage(0);

	while(remaining)
	{
		long rcv_len = remaining;
		if (rcv_len > cTCPBufferSize)
			rcv_len = cTCPBufferSize;

		// Read in from streambuf
		read(tmp, rcv_len);

		// Do not allow failure on write
		try
		{
			// Give to stream and adjust length
			stream.write(tmp, rcv_len);
			remaining -= rcv_len;
			
			// Do log
			if (log != NULL)
				log->write(tmp, rcv_len);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Ignore
			remaining -= rcv_len;
		}
		
		// Update progress
		if (progress)
			progress->SetPercentage((unsigned long)(((total - remaining) * 100.0) / total));
	}
}

// Constructor
CTCPStreamBuf::CTCPStreamBuf() : std::streambuf()
{
	// Set buffers
	setg(mBufIn, mBufIn, mBufIn);
	setp(mBufOut, mBufOut + cTCPBufferSize);
};

// Copy constructor
CTCPStreamBuf::CTCPStreamBuf(const CTCPStreamBuf& copy)
	: std::streambuf(),
	  CTLSSocket(copy)
{
	// Set buffers
	setg(mBufIn, mBufIn, mBufIn);
	setp(mBufOut, mBufOut + cTCPBufferSize);
};

// Open TCP comms
void CTCPStreamBuf::TCPOpen()
{
	// Reset buffers to clear any stale data
	setg(mBufIn, mBufIn, mBufIn);
	setp(mBufOut, mBufOut + cTCPBufferSize);

	CTLSSocket::TCPOpen();
}

// Close network stream
void CTCPStreamBuf::TCPCloseConnection()
{
	// Send any remaining data
	sync();

	// Close the socket
	CTLSSocket::TCPCloseConnection();
}

// Sync buffer
int CTCPStreamBuf::sync()
{
	// flush buffers
	flush_output();

	return 0;
}

// Process chars in buffer area
int CTCPStreamBuf::overflow(int c)
{
	// If flush requested force output
    if (CTCPStreamBuf::traits_type::eq_int_type (c, CTCPStreamBuf::traits_type::eof ()))
    {
		flush_output();
        return CTCPStreamBuf::traits_type::not_eof(c);
    }

	// Check if we are at end of buffer
	if (!(pptr() && (pptr() < epptr())))
		flush_output();

	// store char
	*pptr() = c;
	pbump(1);

	return c;
}

// Flush output buffer
int CTCPStreamBuf::flush_output()
{
	// can only output when open.
	if (!is_open())
		return CTCPStreamBuf::traits_type::eof();

	int len =  pptr() - pbase();

	// Check that there is something to send
	if (len)
	{
		// Send data
		TCPSendData(pbase(), len);

		// Reset buffer
		setp(mBufOut, mBufOut + cTCPBufferSize);
	}

	return len;
}

// Request for input
int CTCPStreamBuf::underflow()
{
	// Check for overrun
	if (!(gptr() && (gptr() < egptr())))
	{
		// Try to read into buffer
		long len = cTCPBufferSize;

		// Get data
		TCPReceiveData(eback(), &len);

		// Reset buffer
		setg(mBufIn, mBufIn, mBufIn + len);
	}

	return CTCPStreamBuf::traits_type::to_int_type(*gptr());
}
