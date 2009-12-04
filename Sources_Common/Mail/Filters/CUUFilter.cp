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


// Source for CUUFilter classes

#include "CUUFilter.h"

#include "CAttachment.h"
#include "CGeneralException.h"
#include "CProgress.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CICSupport.h"
#include "MyCFString.h"
#endif

const SInt16		refNum_Undefined	= -1;

const char cUUDBegin[] = "begin";
const char cUUDBase64[] = "-base64";
const char cUUBegin[] = "begin 644 ";
const char cUUEnd[] = "end";
const char cUUEnd64[] = "====";

// Constants for base64
const signed char cdeUU[] =
	{	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 15
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 31
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,	// 47
		16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,	// 63
		32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,	// 79
		48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,	// 95
		0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 111
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 127
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 143
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 159
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 175
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 191
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 207
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 223
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 239
		-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	// 255

//                               0           1         2         3         4         5          6
//                               01 23456 78901234567890123456789012345678901234567890123456789 0123
const unsigned char cUU[] =     " !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";

// Constants for base64
const signed char cdeUUbase64[] =
	{	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 15
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

//                                 0         1         2         3         4         5         6
//                                 0123456789012345678901234567890123456789012345678901234567890123
const unsigned char cUUbase64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const long cUUEncodeLength = 45;
const long cUUDecodeLength = 60;

// __________________________________________________________________________________________________
// C L A S S __ C B A S E 6 4 F I L T E R
// __________________________________________________________________________________________________

CUUFilter::CUUFilter(LStream* aStream, CProgress* progress) : CCreateFileFilter(aStream, progress)
{
	mLineBuffer = NULL;
	mLineBuffer = new unsigned char[cUUDecodeLength + 3];	// Add count byte & CRLF
	mLinePos = mLineBuffer;
	mLineLength = 0;
	mEncodeStatus = eStart;
	mAtomPos = 0;
	mDecodeStatus = eDecodeStart;
	mDecodedOne = false;
	mUUBase64 = false;
	mDecodeLookup = cdeUU;
}

CUUFilter::~CUUFilter()
{
	delete mLineBuffer;
	mLineBuffer = NULL;
	mLinePos = NULL;
	mDecodeLookup = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

ExceptionCode CUUFilter::GetBytes(void* outBuffer, SInt32& inByteCount)
{
	SInt32 total = 0;
	bool	done = false;
	unsigned char* p;

	// Update progress at start
	if (mProgress && (GetMarker() == 0))
		mProgress->Reset();

	while((total < inByteCount) && !done) {

		// Determine mode
		switch(mEncodeStatus)
		{
		case eStart:
			// Create header text
			InputFileInit();
			CreateHeader();
			mEncodeStatus = eCopyStart;

			// Fall through to next case

		case eCopyStart:
			// Fill output with header text
			p = mBuffer;
			while(mBufferLength && !(total >= inByteCount))
			{
				// Copy the char
				*((unsigned char*) outBuffer) = *p++;
				outBuffer = (unsigned char*)outBuffer + sizeof(unsigned char);
				mBufferLength--;
				total++;
			}

			// Copy remaining header to start of buffer for next output
			if (mBufferLength && (total >= inByteCount))
				::memmove(mBuffer, p, mBufferLength);

			// Otherwise move on to next state
			else
			{
				mEncodeStatus = eLineBuild;
				mFileStream->SetMarker(0, streamFrom_Start);
			}
			break;

		case eLineBuild:
			{
			// Read more if current is less than one line
			if (mBufferLength <= cUUEncodeLength)
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
				mEncodeStatus = eEnd;
				break;
			}

			// Now fill a line
			long count = (mBufferLength >= cUUEncodeLength) ? cUUEncodeLength : mBufferLength;
			mLineLength = Encode(mLineBuffer, mBufferPos, count);
			mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[0];
			if (::get_endl_len(mOutEndl) == 2)
				mLineBuffer[mLineLength++] = ::get_endl(mOutEndl)[1];

			// Update counter & ptr
			mBufferLength -= count;
			mBufferPos += count;
			mMarker += count;

			// switch state after line completely built
			mEncodeStatus = eLineCopy;
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
					mEncodeStatus = eLineBuild;		// switch state if line complete
			}
			break;

		case eEnd:
			// Create header text
			::strcpy((char*) mBuffer, " ");
			::strcat((char*) mBuffer, ::get_endl(mOutEndl));
			::strcat((char*) mBuffer, cUUEnd);
			::strcat((char*) mBuffer, ::get_endl(mOutEndl));
			mBufferLength = ::strlen((char*) mBuffer);
			mEncodeStatus = eCopyEnd;

			// Fall through to next case

		case eCopyEnd:
			// Fill output with header text
			p = mBuffer;
			while(mBufferLength && !(total >= inByteCount))
			{
				// Copy the char
				*((unsigned char*) outBuffer) = *p++;
				outBuffer = (unsigned char*)outBuffer + sizeof(unsigned char);
				mBufferLength--;
				total++;
			}

			// Copy remaining header to start of buffer for next output
			if (mBufferLength && (total >= inByteCount))
				::memmove(mBuffer, p, mBufferLength);

			// Otherwise all done
			else
			{
				done = true;
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

// Filter out base64
ExceptionCode CUUFilter::PutBytes(const void *inBuffer, SInt32& inByteCount)
{
	SInt32 total = 0;
	SInt32 total_out = 0;

	// Reset buffer to start
	mBufferPos = mBuffer;
	mBufferLength = 0;

	// Starting position for decode
	unsigned char* p = (unsigned char*) inBuffer;

	while((total < inByteCount) && (mDecodeStatus != eDecodeEnd))
	{
		switch(mDecodeStatus)
		{
		case eDecodeStart:
			{
			// Look for start
			while(true)
			{
				while (::strncmp((char*) p, cUUDBegin, ::strlen(cUUDBegin)) != 0)
				{
					p++;
					p = (unsigned char*) ::memchr(p, cUUDBegin[0], inByteCount - total);
					if (!p)
						return mDecodedOne ? (ExceptionCode)noErr : (ExceptionCode)writErr;
					total = p - reinterpret_cast<const unsigned char*>(inBuffer);
					if (total > inByteCount)
						return mDecodedOne ? (ExceptionCode)noErr : (ExceptionCode)writErr;
				}

				// Step over first bit and look for base64
				p += ::strlen(cUUDBegin);
				if (::strncmp((char*) p, cUUDBase64, ::strlen(cUUDBase64)) == 0)
				{
					mUUBase64 = true;
					mDecodeLookup = cdeUUbase64;
					p += ::strlen(cUUDBase64);
				}

				// Must have a space, three digits, space
				if ((*p++ == ' ') &&
					isdigit(*p++) &&
					isdigit(*p++) &&
					isdigit(*p++) &&
					(*p++ == ' '))
					break;
			}

			char* fname = (char*) p;
			while((*p != '\r') && (*p != '\n'))
				p++;
			*p++ = 0;
			while((*p == '\r') || (*p == '\n'))
				p++;
			total = p - reinterpret_cast<const unsigned char*>(inBuffer);

			// Set up file info for create
			mDecodedName = fname;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Try to map file extension to creator/type
			ICMapEntry entry;
			CICSupport::ICMapFileName(fname, entry);
			mDecodedInfo.file.fileCreator = entry.fileCreator;
			mDecodedInfo.file.fileType = entry.fileType;
#endif
			CreateFile();
			if (mProgress) mProgress->Redraw();

			if (mFileCreated)
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				mFileStream->OpenDataFork(fsWrPerm);
				::SetFPos(mFileStream->GetDataForkRefNum(), fsFromStart, 0L);
#elif __dest_os == __win32_os || __dest_os == __linux_os
				// Assume opened in CreateFile
#else
#error __dest_os
#endif
			}

			mDecodeStatus = eGetCount;
			break;
			}
		case eGetCount:
			// Get count byte
			mLineLength = mDecodeLookup[*p++];
			total++;
			if (mLineLength > 0)
				mDecodeStatus = eDecode;
			else
				mDecodeStatus = eDecodeClose;
			break;

		case eDecode:
			char c;
			c = *p++;
			total++;

			// Do not decode CRLF's
			if ((c != '\r') && (c != '\n'))
			{
				switch(mAtomPos)
				{
				case 0:
					mAtom.base64.b0 = mDecodeLookup[(unsigned char) c];
					mAtomPos++;
					break;

				case 1:
					mAtom.base64.b1 = mDecodeLookup[(unsigned char) c];
					mAtomPos++;

					// Now write it out to buffer
#ifdef big_endian
					*mBufferPos++ = mAtom.base256[0];
#else
					*mBufferPos++ = mAtom.base256[2];
#endif
					mLineLength--;
					mBufferLength++;
					break;

				case 2:
					mAtom.base64.b2 = mDecodeLookup[(unsigned char) c];
					mAtomPos++;

					// Now write it out to buffer if not end of line
					if (mLineLength > 0)
					{
#ifdef big_endian
						*mBufferPos++ = mAtom.base256[1];
#else
						*mBufferPos++ = mAtom.base256[1];
#endif
						mLineLength--;
						mBufferLength++;
					}
					break;

				case 3:
					mAtom.base64.b3 = mDecodeLookup[(unsigned char) c];
					mAtomPos = 0;

					// Now write it out to buffer if not end of line
					if (mLineLength > 0)
					{
#ifdef big_endian
						*mBufferPos++ = mAtom.base256[2];
#else
						*mBufferPos++ = mAtom.base256[0];
#endif
						mLineLength--;
						mBufferLength++;
					}
					break;
				}
			}
			else if (c == '\r')
			{
#ifdef big_endian
				// Reset to line count
				mDecodeStatus = eGetCount;
#else
				// Punt past
#endif
			}
			else if (c == '\n')
			{
				// Reset to line count
				mDecodeStatus = eGetCount;
			}
			break;

		case eDecodeClose:
			// Flush remainder to stream
			if (mBufferLength)
			{
				SInt32 write_len = mBufferLength;
				OSErr err = mFileStream->PutBytes(mBuffer, write_len);
				if (err)
				{
					CLOG_LOGTHROW(CGeneralException, err);
					throw CGeneralException(err);
				}
				total_out += write_len;
				mBufferPos = mBuffer;
				mBufferLength = 0;
			}

			// Completed file - close
			if (mFileCreated)
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				mFileStream->CloseDataFork();
				mFileCreated = false;
#elif __dest_os == __win32_os
				// mFileStream->Close();	<- Don't do this as file name is lost
				//mFileCreated = false;
#elif __dest_os == __linux_os
				mFileStream->Close(); //filename not lost
				mFileCreated = false;
#else
#error __dest_os
#endif
			}
			mDecodeStatus = eDecodeEnd;

			// Reset decode state engine so that another part in the stream can be processed
			mAtomPos = 0;
			mDecodeStatus = eDecodeStart;
			mUUBase64 = false;
			mDecodeLookup = cdeUU;
			break;

			default:;
		}

		// Check for full buffer
		if (mBufferLength == cMaxBuffer)
		{
			// Flush buffer to stream and reset to start
			SInt32 write_len = mBufferLength;
			if (mFileCreated)
			{
				OSErr err = mFileStream->PutBytes(mBuffer, write_len);
				if (err)
				{
					CLOG_LOGTHROW(CGeneralException, err);
					throw CGeneralException(err);
				}
			}
			total_out += write_len;
			mBufferPos = mBuffer;
			mBufferLength = 0;
		}
	}


	// Flush remainder to stream
	if (mBufferLength)
	{
		SInt32 write_len = mBufferLength;
		OSErr err = mFileStream->PutBytes(mBuffer, write_len);
		if (err)
		{
			CLOG_LOGTHROW(CGeneralException, err);
			throw CGeneralException(err);
		}
		total_out += write_len;
	}

	inByteCount = total_out;

	return noErr;
}

// Copy from src to dest while quoting (len must be multiple of three)
SInt32 CUUFilter::Encode(unsigned char* dest, unsigned char* src, long len)
{
	SInt32 ctr = 0;

	// Copy in length byte
	*dest++ = cUU[len];
	ctr++;

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
		*dest++ = cUU[atom.base64.b0];
		*dest++ = cUU[atom.base64.b1];
		*dest++ = cUU[atom.base64.b2];
		*dest++ = cUU[atom.base64.b3];
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
			atom.base256[1] = ' ';
			atom.base256[2] = ' ';
#else
			atom.base256[2] = *src++;
			atom.base256[1] = ' ';
			atom.base256[0] = ' ';
#endif
			break;

		case 2:
			// Fill atom with unencoded chars and pad
#ifdef big_endian
			atom.base256[0] = *src++;
			atom.base256[1] = *src++;
			atom.base256[2] = ' ';
#else
			atom.base256[2] = *src++;
			atom.base256[1] = *src++;
			atom.base256[0] = ' ';
#endif
			break;
		}

		// Fill destinations with encoded chars
		*dest++ = cUU[atom.base64.b0];
		*dest++ = cUU[atom.base64.b1];
		*dest++ = cUU[atom.base64.b2];
		*dest++ = cUU[atom.base64.b3];
		ctr += 4;
	}
	return ctr;
}

// Create header
void CUUFilter::CreateHeader()
{
	::strcpy((char*) mBuffer, cUUBegin);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject	spec;
	mFileStream->GetSpecifier(spec);

	// Copy file name
	 cdstring name(spec.GetName());
	::strcat((char*) mBuffer, name);

#elif __dest_os == __win32_os
	CString fname = mFileStream->GetFileName();

	// Copy file name
	::strcat((char*) mBuffer, cdstring(fname));
#elif __dest_os == __linux_os
	::strcat((char*)mBuffer, mFileStream->GetFileName());
#else
#error __dest_os
#endif

	::strcat((char*) mBuffer, ::get_endl(mOutEndl));

	// Update buffer length
	mBufferLength = ::strlen((char*) mBuffer);

}

// Initialise file to encode
void CUUFilter::InputFileInit()
{
	mLength = 0;

	// Start from beginning

	// There may not be a resource fork
	try {
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (mFileStream->GetDataForkRefNum() != refNum_Undefined)
		{
			ThrowIfOSErr_(::SetFPos(mFileStream->GetDataForkRefNum(), fsFromStart, 0L));
		}
#elif __dest_os == __win32_os || __dest_os == __linux_os
		// In the begining
		mFileStream->SeekToBegin();
#else
#error __dest_os
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do nothing if no resource fork
	}

	// Add length of both forks
	mLength = mFileStream->GetLength();

}
