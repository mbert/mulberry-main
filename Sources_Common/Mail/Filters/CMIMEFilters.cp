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


// Source for CMIMEFilters classes

#include "CMIMEFilters.h"

#include "CCharSpecials.h"
#include "CGeneralException.h"
#include "CLog.h"
#include "CPreferences.h"
#include "CProgress.h"
#include "CURL.h"			// To get hex mappings

#include <string.h>

#pragma mark ____________________________CMIMEFilter

CMIMEFilter::CMIMEFilter(i18n::ECharsetCode charset, bool is_text, bool is_flowed, bool is_delsp, LStream* aStream, CProgress* progress) : CFilter(aStream, progress)
{
	// Make wrap safe
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();
	if (wrap == 0)
		wrap = 1000;

	// Need some slack in line buffer to account for possible overrun by a single QP char
	// Also base64 is forcibly wrapped to cWrapLength so need a buffer big enough for that
	mLineBufferLength = std::max(wrap + 16, 1024UL);
	mLineBuffer = new unsigned char[mLineBufferLength];
	mLinePos = mLineBuffer;
	mLineLength = 0;
	mCharset = charset;
	mLastChar = 0;
	mIsText = is_text;
	mIsFlowed = is_flowed;
	mIsDelsp = is_delsp;
	mGotCRLF = true;
	mLineQuoted = false;
	mSigDashState = eSigDashNone;
}

CMIMEFilter::~CMIMEFilter()
{
	delete[] mLineBuffer;
	mLineBuffer = NULL;
	mLinePos = NULL;
}

bool CMIMEFilter::Complete() const
{
	return CFilter::Complete() && (mLineLength == 0);
}

// Process a character into the stream - take flow into account
bool CMIMEFilter::FlowProcess(unsigned char c)
{
	// Don't allow NULL's in in-coming text attachments
	if (!c && mIsText)
		c = ' ';

	// Look for flowed and unquoted
	if (mIsText && mIsFlowed && !mLineQuoted &&
		(mLastChar == ' ') &&
#if __line_end != __crlf
		(c == lendl1) && 
#else
		((c == lendl1) || (c == lendl2)) &&
#endif
		(mSigDashState != eSigDash3))
	{
		// Bump last char once past the flow
#if __line_end != __crlf
		mLastChar = c;
#else
		if (c == lendl2)
			mLastChar = c;
#endif

		// Bump down the delsp
		// FIXME: This does not work if the space falls on a buffer boundary
		if ((c == lendl1) && mIsDelsp && (mBufferLength > 0))
		{
			mBufferLength--;
			mBufferPos--;
		}
		// Skip it
		return false;
	}
	else
	{
		// Copy a byte
		mLastChar = c;
		*mBufferPos++ = c;
		mBufferLength++;
		
		switch(mLastChar)
		{
		case lendl2:
			mGotCRLF = true;
			mSigDashState = eSigDashNone;
			break;
		default:
			// Special for sig dash processing
			if (mIsText && mIsFlowed)
			{
				// Change state machine state
				switch(mLastChar)
				{
				case '-':
					if ((mSigDashState == eSigDashNone) && mGotCRLF)
						mSigDashState = eSigDash1;
					else if (mSigDashState == eSigDash1)
						mSigDashState = eSigDash2;
					else
						mSigDashState = eSigDashNone;
					break;
				case ' ':
					if (mSigDashState == eSigDash2)
						mSigDashState = eSigDash3;
					else
						mSigDashState = eSigDashNone;
					break;
				default:
					mSigDashState = eSigDashNone;
					break;
				}
			}

			// Test for quote at start of line only
			if (mGotCRLF)
				mLineQuoted = (mLastChar == '>');
			mGotCRLF = false;
			break;
		}
		
		// Added
		return true;
	}
}

#pragma mark ____________________________C8bitFilter

C8bitFilter::C8bitFilter(i18n::ECharsetCode charset, bool is_text, bool is_flowed, bool is_delsp, LStream* aStream, CProgress* progress)
	: CMIMEFilter(charset, is_text, is_flowed, is_delsp, aStream, progress)
{
	mStatus = eLineBuild;
	mCRLast = false;
}

C8bitFilter::~C8bitFilter()
{
}

// Filter 8bits chars and insert LFs, handle lonely periods, and wrap text
ExceptionCode C8bitFilter::GetBytes(void* outBuffer, SInt32& inByteCount)
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

	while((total < inByteCount) && !done)
	{
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
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
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
				
				// Punt over \n if \r was last
				if (mCRLast && (*endLine == '\n'))
				{
					mBufferLength -= 1;
					mBufferPos += 1;
					mMarker += 1;
					mCRLast = false;
					endLine++;
				}

				// Loop while waiting for line break or exceed of wrap length
				while ((*endLine != '\r') && (*endLine != '\n') &&
							 ((count <= prefs_wrap) || (lastSpace == -1) && (count <= mLineBufferLength)) &&
							 (mBufferLength - count > 0))
				{
					if (*endLine==' ')
						lastSpace = count;
					endLine++;
					count++;
				}
				
				// Check break state
				if ((*endLine != '\r') && (*endLine != '\n') && (count > prefs_wrap))
				{
					// Exceed wrap - check for spaces
					if (lastSpace > 0)
					{
						// Adjust count (include space if flowed)
						count = lastSpace + (mIsFlowed ? 1 : 0);
						
						// Copy line to buffer without trailing SP & then add CRLF
						::memcpy(mLineBuffer, mBufferPos, count);
						
						// Adjust count past trailing space
						mLineLength = mIsFlowed ? count : count++;
					}
					else
					{
						// Copy whole line to buffer & then add CRLF
						::memcpy(mLineBuffer, mBufferPos, count);
						
						// Do not adjust count
						mLineLength = count;
					}
					
					// Add line end
					mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
					if (::get_endl_len(mOutEndl) == 2)
						mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
				}
				else
				{
					// Check whether at end of text
					if (mBufferLength - count > 0)
					{
						// Copy line to buffer
						::memcpy(mLineBuffer, mBufferPos, count);
						mLineLength = count;				// Adjust count to last char
						
						// Add line end
						mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
						if (::get_endl_len(mOutEndl) == 2)
							mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
						
						// Punt line end
						mCRLast = false;
						if (*endLine == '\r')
						{
							count++;
							if (mBufferLength - count > 0)
							{
								if (endLine[1] == '\n')
									count++;
							}
							else
								mCRLast = true;
							
						}
						else if (*endLine == '\n')
							count++;
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

// Filter 8bit chars and takeout LFs
ExceptionCode C8bitFilter::PutBytes(const void* inBuffer, SInt32& inByteCount)
{
	// Reset buffer to start
	mBufferPos = mBuffer;
	mBufferLength = 0;

	const unsigned char* p = reinterpret_cast<const unsigned char*>(inBuffer);
	SInt32 total = inByteCount;
	SInt32 total_out = 0;

	while(total)
	{
#if __line_end != __crlf
		// Ignore the unwanted CR or LF
		if (mIsText && (*p == lendl3))
		{
			p++;
			total--;
			//mGotCRLF = true;
			//mSigDashState = eSigDashNone;
		}
		else
#endif
		{
			// Process char - possibly flowed
			FlowProcess(*p);
			p++;
			total--;
		}

		// Check for full buffer
		if (mBufferLength == cMaxBuffer)
			CheckBuffer(false, total_out);
	}

	// Flush remainder to stream
	CheckBuffer(true, total_out);

	inByteCount = total_out;

	return noErr;
}

#pragma mark ____________________________CQPFilter
// __________________________________________________________________________________________________
// C L A S S __ C Q P F I L T E R
// __________________________________________________________________________________________________

CQPFilter::CQPFilter(i18n::ECharsetCode charset, bool is_text, bool is_flowed, bool is_delsp, LStream* aStream, CProgress* progress)
	: CMIMEFilter(charset, is_text, is_flowed, is_delsp, aStream, progress)
{
	// Make wrap safe
	unsigned long wrap = CPreferences::sPrefs->wrap_length.GetValue();
	if (wrap == 0)
		wrap = 1000;

	mTempBuffer = new unsigned char[wrap + 16];
	mReadStatus = eLineBuild;
	mWriteStatus = eWriteNormal;
	
	mQuotedCharLast = 0;
}

CQPFilter::~CQPFilter()
{
	delete mTempBuffer;
	mTempBuffer = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do QP filter and handle lonely periods
ExceptionCode CQPFilter::GetBytes(void* outBuffer, SInt32& inByteCount)
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

	while((total < inByteCount) && !done)
	{
		// Determine mode
		switch(mReadStatus)
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
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
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
			long tempSpace = -1;
			long count = 0;
			long temp_count = 0;
			bool hard_break = false;
			mLineLength = 0;
			const unsigned char* endLine = mBufferPos;
			unsigned char* tempLine = mTempBuffer;
			
			// Loop while waiting for line break or exceed of wrap length
			while (!hard_break &&
						 (mLineLength <= prefs_wrap - 1) &&
						 (mBufferLength - count > 0))
			{
				if (*endLine==' ')
				{
					lastSpace = count;
					tempSpace = temp_count;
				}
				
				// Never process as QP always hard
#if __line_end != __crlf
				if ((*endLine == '\r') || (*endLine == '\n'))
#else
				if (*endLine == lendl1)
#endif
				{
					endLine++;
					count++;
					hard_break = true;
				}
#if __line_end == __crlf
				// Never process as QP always hard
				else if (*endLine == lendl2)
				{
					endLine++;
					count++;
				}
				// Does char need quoting
				else if (cQPChar[(unsigned char) *endLine])
#else
				// Does char need quoting
				else if (cQPChar[(unsigned char) *endLine] && (*endLine != lendl3))
#endif
				{
					*tempLine++ = *endLine++;
					count++;
					temp_count++;
					mLineLength++;			// No need to quote
				}
				else	// Ordinary quote
				{
					*tempLine++ = *endLine++;
					count++;
					temp_count++;
					mLineLength += 3;		// Quote => 3 chars
				}
			}
			
			// Check break state
			if ((mLineLength > prefs_wrap - 1) && !hard_break)
			{
				// Exceed wrap - check for spaces
				if (lastSpace > 0)
				{
					// Adjust count to include space for soft-break
					count = lastSpace + 1;
					temp_count = tempSpace + 1;
				}
				
				// Copy line to buffer with trailing SP & then add =CRLF
				mLineLength = CopyQuoted(mLineBuffer, mTempBuffer, temp_count);
				mLineBuffer[mLineLength++] = '=';
				mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
				if (::get_endl_len(mOutEndl) == 2)
					mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
			}
			else if (hard_break)
			{
				// Line to buffer & then add CRLF
				mLineLength = CopyQuoted(mLineBuffer, mTempBuffer, temp_count);
				
				// Check for trailing space and encode it as per rfc2045 rule 3
				if (mLineLength && (mLineBuffer[mLineLength - 1] == ' '))
				{
					mLineBuffer[mLineLength - 1] = '=';
					mLineBuffer[mLineLength++] = '2';
					mLineBuffer[mLineLength++] = '0';
				}

				// Now add CRLF
				mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
				if (::get_endl_len(mOutEndl) == 2)
					mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
			}
			else
			{
				// Copy last line to buffer
				mLineLength = CopyQuoted(mLineBuffer, mTempBuffer, temp_count);
			}
			
			// Update counter & ptr
			mBufferLength -= count;
			mBufferPos += count;
			mMarker += count;
			
			// switch state after line completely built
			if (mLineLength)
			{
				mReadStatus = eLineCopy;
				mLinePos = mLineBuffer;
			}
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
					mReadStatus = eLineBuild;		// switch state if line complete
			}
			break;
		default:;
		}
	}

	// Update progress
	if (mProgress)
		mProgress->SetPercentage((GetMarker()*100)/mStream->GetLength());

	inByteCount = total;
	return noErr;
}

// Filter out quoted-printables
ExceptionCode CQPFilter::PutBytes(const void *inBuffer, SInt32& inByteCount)
{
	// Reset buffer to start
	mBufferPos = mBuffer;
	mBufferLength = 0;

	const unsigned char* p = reinterpret_cast<const unsigned char*>(inBuffer);
	SInt32 total = inByteCount;
	SInt32 total_out = 0;

	while(total)
	{
		switch(mWriteStatus)
		{
		case eWriteNormal:
		case eQuoteEndlLast:
			switch(*p)
			{
			case '=':
				// Advance past quote and change status
				p++;
				total--;
				mWriteStatus = eQuoteLast;
				//mSigDashState = eSigDashNone;
				break;

			case '\r':
			case '\n':
				if ((mWriteStatus == eQuoteEndlLast) &&
					(*p != mEndlLast))
				{
					// Skip it - its part of a soft break
					p++;
					total--;
					mWriteStatus = eWriteNormal;
					break;
				}
				
				// Fall through for default processing
			default:
#if __line_end != __crlf
				// Ignore the unwanted CR or LF
				if (mIsText && (*p == lendl3))
				{
					p++;
					total--;
					//mGotCRLF = true;
					//mSigDashState = eSigDashNone;
				}
				else
#endif
				{
					// Process char - possibly flowed
					FlowProcess(*p);
					mQuotedCharLast = 0;
					p++;
					total--;
				}
				mWriteStatus = eWriteNormal;
				break;
			}
			break;

		case eQuoteLast:
			switch(*p)
			{
			case '\r':
			case '\n':
				// Advance past softbreak CR and change status
				mEndlLast = *p++;
				total--;
				mWriteStatus = eQuoteEndlLast;
				mSigDashState = eSigDashNone;
				break;

			default:
				{
					mQuotedChar = 0;
					mQuoted1 = *p;

					// Check validity of QP character
					const unsigned char hex = (unsigned char) cFromHex[*p++];
					total--;
					if (hex == 0xFF)
					{
						// Write out the '=' followed by invalid QP character
						FlowProcess('=');
						FlowProcess(mQuoted1);
						mQuotedCharLast = 0;

						// Change status back to normal after error
						mWriteStatus = eWriteNormal;
					}
					else
					{
						// Just copy high nibble of quoted char
						mQuotedChar = hex << 4;

						// Change status
						mWriteStatus = eQuoteChar1Last;
						//mSigDashState = eSigDashNone;
					}
				}
				break;
			}
			break;

		case eQuoteChar1Last:
			{
				mQuoted2 = *p;

				// Check validity of QP character
				const unsigned char hex = (unsigned char) cFromHex[*p++];
				total--;
				if (hex == 0xFF)
				{
					// Write out the '=' followed by invalid QP characters
					FlowProcess('=');
					FlowProcess(mQuoted1);
					FlowProcess(mQuoted2);
					mQuotedCharLast = 0;

					// Change status back to normal after error
					mWriteStatus = eWriteNormal;
				}
				else
				{
					// Just copy low nibble of quoted char
					mQuotedChar |= hex;

					// Look for encoded line ends
					if (mIsText && ((mQuotedChar == '\r') || (mQuotedChar == '\n')))
					{
						// Skip \r\n
						if ((mQuotedChar != '\n') || (mQuotedCharLast != '\r'))
						{
							// Process char - possibly flowed
							FlowProcess(os_endl[0]);
							if (os_endl_len == 2)
								FlowProcess(os_endl[1]);
						}
					}
					else
					{
						// Process char - possibly flowed
						FlowProcess(mQuotedChar);
					}
					mQuotedCharLast = mQuotedChar;

					// Change status
					mWriteStatus = eWriteNormal;
				}
				break;
			}
		}

		// Check for full buffer
		if (mBufferLength == cMaxBuffer)
			CheckBuffer(false, total_out);
	}

	// Flush remainder to stream
	CheckBuffer(true, total_out);

	inByteCount = total_out;

	return noErr;
}

SInt32 CQPFilter::CopyQuoted(unsigned char* dest, unsigned char* src, long len)
{
	SInt32 ctr = 0;

	while(len--)
	{
		unsigned char c = *src++;

		// Does it need quoting (hard CRLF's must be quoted)
		if (cQPChar[(unsigned char) c] && (c != '\r') && (c != '\n'))
		{
			// No need to quote
			*dest++ = c;
			ctr++;
		}
		else
		{
			*dest++ = '=';
			ctr++;

			// Map to charset
			unsigned char d = c;

			// Do high nibble
			*dest++ = cHexChar[(d >> 4)];
			ctr++;

			// Do low nibble
			*dest++ = cHexChar[(d & 0x0F)];
			ctr++;
		}
	}

	return ctr;
}

#pragma mark ____________________________CBase64Filter

// Constants for base64
const signed char cdebase64[] =
	{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 15
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 31
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,	// 47
	  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,	// 63
	  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,	// 79
	  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,	// 95
	  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,	// 111
	  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,	// 127
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 143
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 159
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 175
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 191
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 207
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 223
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 239
	  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	// 255

//                               0         1         2         3         4         5         6
//                               0123456789012345678901234567890123456789012345678901234567890123
const unsigned char cbase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const unsigned char cbase64pad = '=';

const long cEncodeLength = (3L*cWrapLength) >> 2L;

// __________________________________________________________________________________________________
// C L A S S __ C B A S E 6 4 F I L T E R
// __________________________________________________________________________________________________

CBase64Filter::CBase64Filter(i18n::ECharsetCode charset, bool is_text, bool is_flowed, bool is_delsp, LStream* aStream, CProgress* progress)
	: CMIMEFilter(charset, is_text, is_flowed, is_delsp, aStream, progress)
{
	mReadStatus = eLineBuild;
	mAtomPos = 0;
	mCharLast = 0;
}

CBase64Filter::~CBase64Filter()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

ExceptionCode CBase64Filter::GetBytes(void* outBuffer, SInt32& inByteCount)
{
	SInt32 total = 0;
	bool	done = false;

	// Update progress at start
	if (mProgress && (GetMarker() == 0))
		mProgress->Reset();

	while((total < inByteCount) && !done)
	{
		// Determine mode
		switch(mReadStatus)
		{
		case eLineBuild:
			{
				// Read more if current is less than one line
				if (mBufferLength < cEncodeLength)
				{
					// Copy remaining to start of buffer
					if (mBufferLength)
						::memmove(mBuffer, mBufferPos, mBufferLength);
					
					// Read from input into buffer
					SInt32 read_len = cMaxBuffer - mBufferLength;
					OSErr err = mStream->GetBytes(mBuffer + mBufferLength, read_len);
					if ((err != noErr) && (err != readErr) && (err != eofErr))
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
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
				long count = (mBufferLength >= cEncodeLength) ? cEncodeLength : mBufferLength;
				mLineLength = Encode(mLineBuffer, mBufferPos, count);
				mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
				if (::get_endl_len(mOutEndl) == 2)
					mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];
				
				// Update counter & ptr
				mBufferLength -= count;
				mBufferPos += count;
				mMarker += count;
				
				// switch state after line completely built
				mReadStatus = eLineCopy;
				mLinePos = mLineBuffer;
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
					mReadStatus = eLineBuild;		// switch state if line complete
			}
			break;
		default:;
		}
	}

	// Update progress
	if (mProgress)
		mProgress->SetPercentage((GetMarker()*100)/mStream->GetLength());

	inByteCount = total;
	return noErr;
}

// Filter out base64
ExceptionCode CBase64Filter::PutBytes(const void *inBuffer, SInt32& inByteCount)
{
	SInt32 total = 0;
	SInt32 total_out = 0;
	bool got_pad2 = false;
	bool got_pad3 = false;

	// Reset buffer to start
	mBufferPos = mBuffer;
	mBufferLength = 0;

	while(total < inByteCount)
	{
		unsigned char c;
		c = *((unsigned char*) inBuffer);
		inBuffer = (unsigned char*)inBuffer + sizeof(unsigned char);
		total++;

		// Do not decode CRLF's
		if ((c != '\r') && (c != '\n'))
		{
			switch(mAtomPos)
			{
			case 0:
				mAtom.base64.b0 = cdebase64[c];
				mAtomPos++;
				break;

			case 1:
				mAtom.base64.b1 = cdebase64[c];
				mAtomPos++;

				// Now write it out to buffer
#ifdef big_endian
				PutChar(mAtom.base256[0]);
#else
				PutChar(mAtom.base256[2]);
#endif
				break;

			case 2:
				got_pad2 = (c == cbase64pad);
				mAtom.base64.b2 = cdebase64[c];
				mAtomPos++;

				// Now write it out to buffer if not pad
				if (!got_pad2)
				{
#ifdef big_endian
					PutChar(mAtom.base256[1]);
#else
					PutChar(mAtom.base256[1]);
#endif
				}
				break;

			case 3:
				got_pad3 = (c == cbase64pad);
				mAtom.base64.b3 = cdebase64[c];
				mAtomPos = 0;

				// Now write it out to buffer if not pad
				if (!got_pad3)
				{
#ifdef big_endian
					PutChar(mAtom.base256[2]);
#else
					PutChar(mAtom.base256[0]);
#endif
				}
				break;
			}
		}

		// Check for full buffer
		if (mBufferLength == cMaxBuffer)
			CheckBuffer(false, total_out);
	}

	// Flush remainder to stream
	CheckBuffer(true, total_out);

	inByteCount = total_out;

	return noErr;
}

// Copy from src to dest while quoting (len must be multiple of three)
SInt32 CBase64Filter::Encode(unsigned char* dest, unsigned char* src, long len)
{
	SInt32 ctr = 0;

	// Encode chars in multiples of three
	for(long i = 0; i < len/3; i++)
	{
		TAtom atom;

		// Fill atom with unencoded chars
#ifdef big_endian
		atom.base256[0] = *src++;
		atom.base256[1] = *src++;
		atom.base256[2] = *src++;
#else
		atom.base256[2] = *src++;
		atom.base256[1] = *src++;
		atom.base256[0] = *src++;
#endif

		// Fill destinations with encoded chars
		*dest++ = cbase64[atom.base64.b0];
		*dest++ = cbase64[atom.base64.b1];
		*dest++ = cbase64[atom.base64.b2];
		*dest++ = cbase64[atom.base64.b3];
		ctr += 4;
	}

	// Check for remainder
	if (len % 3)
	{
		TAtom atom;

		switch(len % 3)
		{
		case 1:
			// Fill atom with unencoded chars and pad
#ifdef big_endian
			atom.base256[0] = *src++;
			atom.base256[1] = '\0';
			atom.base256[2] = '\0';
#else
			atom.base256[2] = *src++;
			atom.base256[1] = '\0';
			atom.base256[0] = '\0';
#endif

			// Fill destinations with encoded chars and pad
			*dest++ = cbase64[atom.base64.b0];
			*dest++ = cbase64[atom.base64.b1];
			*dest++ = cbase64pad;
			*dest++ = cbase64pad;
			break;

		case 2:
			// Fill atom with unencoded chars and pad
#ifdef big_endian
			atom.base256[0] = *src++;
			atom.base256[1] = *src++;
			atom.base256[2] = '\0';
#else
			atom.base256[2] = *src++;
			atom.base256[1] = *src++;
			atom.base256[0] = '\0';
#endif

			// Fill destinations with encoded chars and pad
			*dest++ = cbase64[atom.base64.b0];
			*dest++ = cbase64[atom.base64.b1];
			*dest++ = cbase64[atom.base64.b2];
			*dest++ = cbase64pad;
			break;
		}
		ctr += 4;
	}
	return ctr;
}

void CBase64Filter::PutChar(unsigned char c)
{
	// If text then do possible flow processing
	if (mIsText)
	{
		if ((c == '\r') || (c == '\n'))
		{
			if ((c != '\n') || (mCharLast != '\r'))
			{
				// Process char - possibly flowed
				FlowProcess(os_endl[0]);
				if (os_endl_len == 2)
					FlowProcess(os_endl[1]);
			}
		}
		else
			// Process char - possibly flowed
			FlowProcess(c);

		mCharLast = c;
	}
	else
	{
		*mBufferPos++ = c;
		mBufferLength++;
		mCharLast = c;
	}
}
