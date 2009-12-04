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


// Source for CFilter class

#include "CFilter.h"
#include "CGeneralException.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CProgress.h"

#include <string.h>

//#define DOT_STUFF

// __________________________________________________________________________________________________
// C L A S S __ C F I L T E R
// __________________________________________________________________________________________________

CFilter::CFilter(LStream* aStream, CProgress* progress)
{
	mBuffer = NULL;
	mBuffer = new unsigned char[cMaxBuffer];
	mBufferPos = mBuffer;
	mBufferLength = 0;
	mStream = aStream;
	mProgress = progress;
	mOutEndl = eEndl_Auto;
	mAutoCreate = true;
}

CFilter::~CFilter()
{
	delete mBuffer;
	mBuffer = NULL;
	mBufferPos = NULL;
	mStream = NULL;
	mProgress = NULL;
}

bool CFilter::Complete() const
{
	return (GetMarker() == mStream->GetLength());
}

bool CFilter::CheckBuffer(bool force, SInt32& written)
{
	if ((mBufferLength == cMaxBuffer) || force && mBufferLength)
{
		// Flush buffer to stream and reset to start
		SInt32 write_len = mBufferLength;
		OSErr err = mStream->PutBytes(mBuffer, write_len);
		if (err)
		{
			CLOG_LOGTHROW(CGeneralException, err);
			throw CGeneralException(err);
		}
		written += write_len;
		mBufferPos = mBuffer;
		mBufferLength = 0;
		return true;
	}
	else
		return false;
}

// No filter but do progress
ExceptionCode CFilter::GetBytes(void* outBuffer, SInt32& inByteCount)
{
	// Update progress at start
	if (mProgress && (GetMarker() == 0))
		mProgress->Reset();

	// Copy direct from buffer
	ExceptionCode ex = mStream->GetBytes(outBuffer, inByteCount);
	mMarker += inByteCount;

	// Update progress
	if (!ex && mProgress)
		mProgress->SetPercentage((GetMarker()*100)/mStream->GetLength());

	return ex;
}

// No filter
ExceptionCode CFilter::PutBytes(const void *inBuffer, SInt32& inByteCount)
{
	// Copy direct to stream
	return mStream->PutBytes(inBuffer, inByteCount);
}

#pragma mark -
// __________________________________________________________________________________________________
// C L A S S __ C F I L T E R L F S
// __________________________________________________________________________________________________

CFilterEndls::CFilterEndls(LStream* aStream, CProgress* progress) : CFilter(aStream, progress)
{
	// Make wrap safe
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();
	if (wrap == 0)
		wrap = 1000;

	mLineBuffer = NULL;
	mLineBuffer = new unsigned char[wrap + 2];
	mLinePos = mLineBuffer;
	mLineLength = 0;
	mStatus = eLineBuild;
}

CFilterEndls::~CFilterEndls()
{
	delete mLineBuffer;
	mLineBuffer = NULL;
	mLinePos = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Filter in LFs, handle lonely periods, and wrap text
ExceptionCode CFilterEndls::GetBytes(void* outBuffer, SInt32& inByteCount)
{
	SInt32 total = 0;
	bool	done = false;

	// Update progress at start
	if (mProgress && (GetMarker() == 0))
		mProgress->Reset();

	unsigned long prefs_wrap = CPreferences::sPrefs->wrap_length.GetValue();

	// Make wrap safe
	if (prefs_wrap == 0)
		prefs_wrap = 1000;

	while((total < inByteCount) && !done) {

		// Determine mode
		switch(mStatus)
		{
		case eLineBuild:
			{
				// Read more if current is less than one line
				if (mBufferLength <= prefs_wrap)
					{
						// Copy remaining to start of buffer
						if (mBufferLength)
							::memmove(mBuffer, mBufferPos, mBufferLength);
						
						// Read from input into buffer
						SInt32 read_len = cMaxBuffer - mBufferLength;
						OSErr err = mStream->GetBytes(mBuffer + mBufferLength, read_len);
						if ((err != noErr) && (err != readErr) && (err != eofErr))
						{
							CLOG_LOGTHROW(CGeneralException, err);
							throw CGeneralException(err);
						}
						mBufferLength += read_len;
						mBufferPos = mBuffer;
					}
				
				// Is any data left to process
				if (mBufferLength < 1)
					{
						done = true;
						break;
					}
				
				// Now fill a line
				long lastSpace = -1;
				long count = 0;
				const unsigned char* endLine = mBufferPos;
				
				// Loop while waiting for line break or exceed of wrap length
				while ((*endLine!=lendl1) &&
							 (count <= prefs_wrap) &&
							 (mBufferLength - count > 0))
					{
						if (*endLine==' ')
							lastSpace = count;
						endLine++;
						count++;
					}
				
				// Check break state
				if ((*endLine != lendl1) && (count > prefs_wrap))
					{
						// Exceed wrap - check for spaces
						if (lastSpace > 0)
							{
								// Adjust count
								count = lastSpace;
								
								// Copy line to buffer without trailing SP & then add CRLF
								::memcpy(mLineBuffer, mBufferPos, count);
								mLineLength = count++;				// Adjust count past trailing space
							}
						else
							{
								// Copy whole line to buffer & then add CRLF
								::memcpy(mLineBuffer, mBufferPos, --count);
								mLineLength = count;				// Do not adjust count
							}
						
						// Add line end
						mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
						if (::get_endl_len(mOutEndl) == 2)
							mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
					}
				else
					{
						// Check for CRLF and bump past pair
						// Check whether at end of text
						if (mBufferLength - count > 0)
							{
								// Copy line to buffer without line end
								::memcpy(mLineBuffer, mBufferPos, count);
								mLineLength = count;				// Adjust count to last char
								
								// Add line terminator
								mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
								if (::get_endl_len(mOutEndl) == 2)
									mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
								
								// Punt line end
#if __line_end != __crlf
								count++;
#else
								count += 2;
#endif
							}
						else
							{
								// Copy last line to buffer
								::memcpy(mLineBuffer, mBufferPos, count);
								mLineLength = count;				// Do not adjust count
							}
					}
				
				// Update counter & ptr
				mBufferLength -= count;
				mBufferPos += count;
				mMarker += count;
				
				// switch state after line completely built
				mStatus = eLineCopy;
				mLinePos = mLineBuffer;
				
#ifdef DOT_STUFF
				// Check for lonely period
				if ((mLineLength == 1 + ::get_endl_len(mOutEndl)) && (*mLineBuffer == '.'))
					{
						// Add an extra period
						mLineLength = 1;
						mLineBuffer[mLineLength++] = '.';
						mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
						if (::get_endl_len(mOutEndl) == 2)
							mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
					}
#endif
				break;
			}
		case eLineCopy:
			// Loop while there is something in the line and while space to copy to
			while(mLineLength && (total < inByteCount))
			{
				// Copy byte and adjust ctrs
				*((unsigned char*) outBuffer) = *mLinePos++;
				outBuffer = (unsigned char*)outBuffer + sizeof(unsigned char);
				total++;
				if (!--mLineLength)
					mStatus = eLineBuild;		// switch state if line complete
			}
			break;
		}
	}

	// Update progress
	if (mProgress)
		mProgress->SetPercentage((GetMarker()*100)/mStream->GetLength());

	inByteCount = total;
	return noErr;
}

// Filter out LFs
ExceptionCode CFilterEndls::PutBytes(const void* inBuffer, SInt32& inByteCount)
{
	SInt32 total = 0;
	SInt32 total_out = 0;

	// Reset buffer to start
	mBufferPos = mBuffer;
	mBufferLength = 0;

	while(total < inByteCount)
	{
		// Copy a byte
#if __line_end != __crlf
		if (*((unsigned char*) inBuffer) != lendl3)
#endif
		{
			*mBufferPos++ = *((unsigned char*) inBuffer);
			inBuffer = (unsigned char*)inBuffer + sizeof(unsigned char);
			total++;
			mBufferLength++;
		}
#if __line_end != __crlf
		else
		{
			inBuffer = (unsigned char*)inBuffer + sizeof(unsigned char);
			total++;
		}
#endif

		// Check for full buffer
		if (mBufferLength == cMaxBuffer)
			CheckBuffer(false, total_out);
	}

	// Flush remainder to stream
	CheckBuffer(true, total_out);

	inByteCount = total_out;

	return noErr;
}
