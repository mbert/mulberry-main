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


// filterbuf.cp

#include "filterbuf.h"

#if __dest_os == __win32_os
#include <WIN_LStream.h>
#elif __dest_os == __linux_os
#include "UNX_LStream.h"
#endif

#pragma mark ____________________________filterbuf

filterbuf::filterbuf(bool encodeit, CProgress* progress)
{
	mOut = NULL;
	mLOut = NULL;
	mBOut = NULL;
	mBOutSize = 0;
	mBOutWritten = 0;
	mEncode = encodeit;
	mProgress = progress;

 	CreateBuffer();
 }

filterbuf::~filterbuf()
{
	sync();

	mOut = NULL;
	mLOut = NULL;
	mBOut = NULL;
	delete mBuffer;
	mBuffer = NULL;
	mBufferNext = NULL;
}

void filterbuf::CreateBuffer()
{
	mBufferNext = mBuffer = new char[filterbuf_size];
	mBufferCount = 0;
}

void filterbuf::WriteBuffer(const char_type* s, std::streamsize n)
{
	// See if new data would overflow buffer
	if (mBufferCount + n > filterbuf_size)
		// Write out current buffer and reset it
		CheckBuffer(true);
	
	// See if new data is bigger than buffer
	if (n > filterbuf_size)
		write(s, n);
	else
	{
		// Add to existing buffer
		::memcpy(mBufferNext, s, n);
		mBufferNext += n;
		mBufferCount += n;
		CheckBuffer();
	}
}

bool filterbuf::CheckBuffer(bool force)
{
	if ((mBufferCount == filterbuf_size) || force)
	{
		write(mBuffer, mBufferCount);
		mBufferCount = 0;
		mBufferNext = mBuffer;
		return true;
	}
	else
		return false;
}

void filterbuf::write(const char_type* s, std::streamsize n)
{
	if (mOut)
		mOut->write(s, n);
	if (mLOut)
	{
		long ntemp = n;
		mLOut->PutBytes(s, ntemp);
	}
	if (mBOut)
	{
		mBOutWritten = std::min((unsigned long) n, mBOutSize);
		::memcpy(mBOut, s, mBOutWritten);
	}
}

#pragma mark ____________________________crlf_filterbuf

std::streamsize crlf_filterbuf::encode (const char_type* s, std::streamsize n)
{
	const char* out_endl = ::get_endl(mOutEndl);
	unsigned long out_endl_len = ::get_endl_len(mOutEndl);

	// Filter from one endl to another
	const char_type* p;
	std::streamsize ntemp = n;
	while(ntemp)
	{
		// Start at beginning of next bit
		p = s;

		// Might have an endl2 at start due to buffer boundary between \r\n
		if (mCRLast && (*p == '\n'))
		{
			p++;
			ntemp--;
			mCRLast = false;
			
			// Now recycle for next line
			s = p;
			continue;
		}

		// Accumulate up to next endl
		while((ntemp != 0) && (*p != '\r') && (*p != '\n'))
		{
			p++;
			ntemp--;
		}

		// Write out what was found
		if (p - s)
			WriteBuffer(s, p - s);

		// Now write out new endl and punt past old
		if ((ntemp != 0) && ((*p == '\r') || (*p == '\n')))
		{
			WriteBuffer(out_endl, out_endl_len);

			char c = *p;
			p++;
			ntemp--;
			
			if (c == '\r')
			{
				if ((ntemp != 0) && (*p == '\n'))
				{
					p++;
					ntemp--;
				}
				else
					mCRLast = true;
			}
		}

		// Adjust ptrs if more to come
		s = p;
	}

	return n;
}

std::streamsize crlf_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Does nothing!
	return n;
}

#pragma mark ____________________________dotstuff_filterbuf

std::streamsize dotstuff_filterbuf::encode (const char_type* s, std::streamsize n)
{
	// Dot stuff contents
	const char_type* p;
	std::streamsize ntemp = n;
	while(ntemp)
	{
		// Start at beginning of next bit
		p = s;

		// See if dot-stuffing needed
		if (got_crlf && (*p == '.'))
		{
			*mBufferNext++ = '.';
			mBufferCount++;
			if (mBufferCount == filterbuf_size)
				CheckBuffer();
		}

		// Accumulate up to next LF
		while(ntemp && (*p != net_endl[1]))
		{
			p++;
			ntemp--;
		}

		// Output the LF too
		if (ntemp && (*p == net_endl[1]))
		{
			p++;
			ntemp--;
			got_crlf = true;
		}
		else
			got_crlf = false;

		// Write out what was found
		if (p - s)
			WriteBuffer(s, p - s);

		// Adjust ptrs if more to come
		s = p;
	}

	return n;
}

std::streamsize dotstuff_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Never need to un-dot-stuff - delivery agent always does it
	WriteBuffer(s, n);
	return n;
}

#pragma mark ____________________________fromstuff_filterbuf

std::streamsize fromstuff_filterbuf::encode (const char_type* s, std::streamsize n)
{
	// From stuff contents
	const char_type* p;
	std::streamsize ntemp = n;
	while(ntemp)
	{
		// Start at beginning of next bit
		p = s;

		// Remember whether this is at the start of a line
		bool had_crlf = got_crlf;

		// Check for special case of second endl at buffer boundary
		if (mCRLast && (*p == '\n'))
		{
			// Output the \n now
			// NB This will only happen on the rare occasion that an endl pair is split across
			// the stream input buffer boundary
			*mBufferNext++ = *p;
			mBufferCount++;
			if (mBufferCount == filterbuf_size)
				CheckBuffer();
			p++;
			ntemp--;
			mCRLast = false;
			
			// Now recycle for next line
			s = p;
			continue;
		}

		// Accumulate up to next endl start
		while(ntemp && (*p != '\r')  && (*p != '\n'))
		{
			p++;
			ntemp--;
			mCRLast = false;
		}

		// Output the first endl too
		if (ntemp && ((*p == '\r') || (*p == '\n')))
		{
			mCRLast = (*p == '\r');
			p++;
			ntemp--;
			got_crlf = true;
			
			// Output second endl if possible
			if (mCRLast && ntemp && (*p == '\n'))
			{
				p++;
				ntemp--;
				mCRLast = false;
			}
		}
		else
			got_crlf = false;

		// Write out what was found
		if (p - s)
		{
			// Start and length of buffer to write/test
			//const char* start = s;
			unsigned long len = p - s;

			// Got to end of line
			if (got_crlf)
			{
				// Check for entire line in buffer and write stuff it
				if (had_crlf)
					WriteLineWithStuffing(s, p - s);
					
				// Might have partial line before that was big enough to be tested and written out
				// in which case we can just write the remainder withut stuffing
				else if (mPartialLine.empty())
					WriteBuffer(s, p - s);
				
				// Otherwise we have a partial line start so append the remainder and write stuff it
				else
				{
					mPartialLine.append(s, p - s);
					WriteLineWithStuffing(mPartialLine.c_str(), mPartialLine.length());
					mPartialLine.clear();
				}
			}
			
			// Have a partial line
			else
			{
				// Look for start of line
				if (had_crlf)
				{
					// Look for a start of a line that can be tested for stuffing and write it out
					if (len >= 5)
						WriteLineWithStuffing(s, p - s);
					
					// Cache the start of the line
					else
						mPartialLine.assign(s, p - s);
				}
				
				// Have the middle of a line
				else
				{
					// If partial already written out then write this out without test
					if (mPartialLine.empty())
						WriteBuffer(s, p - s);

					// Partial line start exists
					else
					{
						// Append to partial
						mPartialLine.append(s, p - s);
						
						// Now see if it can be written with test
						if (mPartialLine.length() >= 5)
						{
							WriteLineWithStuffing(mPartialLine.c_str(), mPartialLine.length());
							mPartialLine.clear();
						}
					}
				}
			}
		}

		// Adjust ptrs if more to come
		s = p;
	}

	return n;
}

std::streamsize fromstuff_filterbuf::decode (const char_type* s, std::streamsize n)
{
	// Never need to un-from-stuff - its a one-way conversion when writing to a mailbox file
	WriteBuffer(s, n);
	return n;
}

void fromstuff_filterbuf::WriteLineWithStuffing(const char_type* s, std::streamsize n)
{
	// Look at start for 'From '
	if (!::strncmp(s, "From ", 5))
	{
		// Stuffit with '>'
		*mBufferNext++ = '>';
		mBufferCount++;
		if (mBufferCount == filterbuf_size)
			CheckBuffer();
	}
	
	// Write the line out
	WriteBuffer(s, n);
}

void fromstuff_filterbuf::FinishLine()
{
	// Write the remainder to the buffer
	if (!mPartialLine.empty())
	{
		WriteBuffer(mPartialLine.c_str(), mPartialLine.length());
		mPartialLine.clear();
	}
}
