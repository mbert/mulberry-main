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


// Source for CBinHexFilter class

#include "CBinHexFilter.h"

#include "CGeneralException.h"
#include "CLog.h"
#include "CProgress.h"
#include "CErrorHandler.h"
#include "CUnicodeStdLib.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MyCFString.h"
#endif					

#if __dest_os == __win32_os
#include <winsock.h>
#endif

#if __dest_os == __linux_os
#include <jFileUtil.h>

#include <netinet/in.h>
#endif

const SInt16		refNum_Undefined	= -1;

// Constants

const char cBinHexTitle[] = "(This file must be converted with BinHex 4.0)";

const signed char cdebinhex[] =
	{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 15
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 31
	 -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, -1, -1,	// 47
	 13, 14, 15, 16, 17, 18, 19, -1, 20, 21, -1, -1, -1, -1, -1, -1,	// 63
	 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, -1,	// 79
	 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, -1, -1, -1, -1,	// 95
	 48, 49, 50, 51, 52, 53, 54, -1, 55, 56, 57, 58, 59, 60, -1, -1,	// 111
	 61, 62, 63, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 127
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 143
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 159
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 175
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 191
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 207
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 223
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,	// 239
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };	// 255

//                               0           1         2         3         4         5         6
//                               0 12345 6789012345678901234567890123456789012345678901234567890123
const unsigned char cbinhex[] = "!\"#$%&\'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

// __________________________________________________________________________________________________
// C L A S S __ C B I N H E X F I L T E R
// __________________________________________________________________________________________________

// Constructor
CBinHexFilter::CBinHexFilter(LStream* aStream, CProgress* progress) : CCreateFileFilter(aStream, progress)
{
	InitBinHexFilter();

	mFileOutBuffer = new unsigned char[cMaxBuffer];
}

CBinHexFilter::~CBinHexFilter()
{
	delete mFileOutBuffer;
	mFileOutBuffer = NULL;
}

void CBinHexFilter::InitBinHexFilter()
{
	// Init params
	mStatus1 = eStart1;
	mStatus2 = eStart2;
	mDecodedOne = false;

	mFileOutBuffer = NULL;

	// Force finite length, marker at start
	mLength = 1;
	mMarker = 0;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

bool CBinHexFilter::Complete() const
{
	return ((mStatus1 == eEnd1) || (mStatus2 == eEnd2));
}

// Encode data from source into BinHex for output
ExceptionCode CBinHexFilter::GetBytes(void* outBuffer, SInt32& inByteCount)
{
	unsigned char* p;
	unsigned char* q = (unsigned char*) outBuffer;
	SInt32 total = 0;
	bool nomore = false;

	// Update progress at start
	if (mProgress && (mStream->GetMarker() == 0))
		mProgress->Reset();

	// Loop to fill buffer or no more input
	while((total < inByteCount) && !nomore) {
		switch (mStatus1) {

			case eStart1:
				// Do file init
				InputFileInit();

				// Copy BinHex title details into buffer
				::strcpy((char*) mBuffer, ::get_endl(mOutEndl));
				::strcat((char*) mBuffer, cBinHexTitle);
				::strcat((char*) mBuffer, ::get_endl(mOutEndl));
				::strcat((char*) mBuffer, ":");
				mBufferLength = ::strlen((char*) mBuffer);
				mStatus1 = eCopyStart;

				// Fall through to next case

			case eCopyStart:
				// Fill output with header text
				p = mBuffer;
				while(mBufferLength && !(total >= inByteCount)) {

					// Copy the char
					*q++ = *p++;
					mBufferLength--;
					total++;
				}

				// Copy remaining header to start of buffer for next output
				if (mBufferLength && (total >= inByteCount))
					::memmove(mBuffer, p, mBufferLength);

				// Otherwise move on to next state
				else {
					mStatus1 = eHeader;
					mLineCount = 1;
				}
				break;

			case eHeader:
				// Copy header into buffer
				CreateHeader();

				// Init state variables and counters
				mProcess = eFirstChar;
				mAtomPos = 0;
				mLastChar = 0x00;
				mRunLength = 0;
				mCRC = 0;
				mSubStatus1 = eNormal;
				mStatus1 = eCopyHeader;

				// Fall through to next case

			case eCopyHeader:
				Encode(&q, &total, inByteCount);

				// Have required bytes been encoded and sent to output?
				if (!mBufferLength) {
					// All encoding in this state done

					// Calc final CRC bytes
					IncrementCRC(0);
					IncrementCRC(0);

					// Convert to network order
					mCRC = htons(mCRC);

					// Send CRC
					*((unsigned short*) mBuffer) = mCRC;
					mBufferLength = 2;
					mProcess = eFirstChar;
					mStatus1 = ePutHeaderCRC;
				}
				break;

			case ePutHeaderCRC:
				Encode(&q, &total, inByteCount);

				// Have required bytes been encoded and sent to output?
				if (!mBufferLength) {
					// All encoding in this state done

					// Fill buffer from data fork
					mFileStream->SetMarker(0, streamFrom_Start);
					mBufferLength = cMaxBuffer;
					OSErr err = mFileStream->GetBytes(mBuffer, mBufferLength);
					if ((err != noErr) && (err != readErr) && (err != eofErr))
					{
						CLOG_LOGTHROW(CGeneralException, err);
						throw CGeneralException(err);
					}
					mMarker += mBufferLength;

					// Set flags
					mProcess = eFirstChar;
					mRunLength = 0;
					mCRC = 0;
					if (!mBufferLength) {
						*((unsigned short*) mBuffer) = mCRC;
						mBufferLength = 2;
						mStatus1 = ePutDataCRC;
					}
					else
						mStatus1 = eDataForkCopy;
				}
				break;

			case eDataForkCopy:
				Encode(&q, &total, inByteCount);

				// Has input buffer been used up?
				if (!mBufferLength) {
					// Try to read some more
					mBufferLength = cMaxBuffer;
					OSErr err = mFileStream->GetBytes(mBuffer, mBufferLength);
					if ((err != noErr) && (err != readErr) && (err != eofErr))
					{
						CLOG_LOGTHROW(CGeneralException, err);
						throw CGeneralException(err);
					}
					mMarker += mBufferLength;

					// If still zero then data done
					if (!mBufferLength) {
						// Do CRC bytes
						IncrementCRC(0);
						IncrementCRC(0);

						// Convert to network order
						mCRC = htons(mCRC);

						*((unsigned short*) mBuffer) = mCRC;
						mBufferLength = 2;
						mProcess = eFirstChar;
						mStatus1 = ePutDataCRC;
					}
				}
				break;

			case ePutDataCRC:
				Encode(&q, &total, inByteCount);

				// Has input buffer been used up?
				if (!mBufferLength) {
					// Fill buffer from resource fork
					mFileStream->SetRsrcMarker(0, streamFrom_Start);
					mBufferLength = cMaxBuffer;
					OSErr err = mFileStream->GetResourceBytes(mBuffer, mBufferLength);
					if ((err != noErr) && (err != readErr) && (err != eofErr))
					{
						CLOG_LOGTHROW(CGeneralException, err);
						throw CGeneralException(err);
					}
					mMarker += mBufferLength;

					// Set flags
					mProcess = eFirstChar;
					mRunLength = 0;
					mCRC = 0;
					if (!mBufferLength) {
						*((unsigned short*) mBuffer) = mCRC;
						mBufferLength = 2;
						mStatus1 = ePutRsrcCRC;
					}
					else
						mStatus1 = eRsrcForkCopy;
				}
				break;

			case eRsrcForkCopy:
				Encode(&q, &total, inByteCount);

				// Has input buffer been used up?
				if (!mBufferLength) {
					// Try to read some more
					mBufferLength = cMaxBuffer;
					OSErr err = mFileStream->GetResourceBytes(mBuffer, mBufferLength);
					if ((err != noErr) && (err != readErr) && (err != eofErr))
					{
						CLOG_LOGTHROW(CGeneralException, err);
						throw CGeneralException(err);
					}
					mMarker += mBufferLength;

					// If still zero then done
					if (!mBufferLength) {
						// Do CRC bytes
						IncrementCRC(0);
						IncrementCRC(0);

						// Convert to network order
						mCRC = htons(mCRC);

						*((unsigned short*) mBuffer) = mCRC;
						mBufferLength = 2;
						mProcess = eFirstChar;
						mStatus1 = ePutRsrcCRC;
					}
				}
				break;

			case ePutRsrcCRC:
				Encode(&q, &total, inByteCount);

				// Has input buffer been used up?
				if (!mBufferLength) {
					// Set flags
					mProcess = eFirstChar;
					mStatus1 = eTerminate;

					// Finish off encoding
					switch (mAtomPos) {

						case 0:
						case 3:
							mBufferLength = 0;
							break;

						case 1:
						case 2:
							mBuffer[0] = 0x00;
							mBufferLength = 1;
							break;

					}
				}
				break;

			case eTerminate:
				Encode(&q, &total, inByteCount);

				if (mBufferLength==0) {
					// Copy in last chars
					::strcpy((char*) mBuffer, ":");
					::strcat((char*) mBuffer, ::get_endl(mOutEndl));
					mBufferLength = 1 + ::get_endl_len(mOutEndl);
					mStatus1 = ePutLast;
				}
				break;

			case ePutLast:
				p = mBuffer;
				while(mBufferLength && !(total >= inByteCount)) {

					// Copy the char
					*q++ = *p++;
					mBufferLength--;
					total++;
				}

				// Copy remaining to start of buffer
				if (mBufferLength && (total >= inByteCount))
					::memmove(mBuffer, p, mBufferLength);

				// Otherwise move on to last state
				else {
					mStatus1 = eEnd1;
					nomore = true;
				}
				break;

			case eEnd1:
				total = 0;
				nomore = true;
				break;

		}
	}

	// Update progress
	if (mProgress && (mStream->GetLength() > 0))
		mProgress->SetPercentage((mStream->GetMarker()*100)/mStream->GetLength());

	inByteCount = total;
	return noErr;
}

// Decode data from BinHex for output
ExceptionCode CBinHexFilter::PutBytes(const void *inBuffer, SInt32& inByteCount)
{
	OSErr err = noErr;

	try {
		//SInt32 total = 0;

		// Count down bytes in buffer
		mBufferLength = inByteCount;
		unsigned char* p = (unsigned char*) inBuffer;

		while(mBufferLength > 0)
		{
			switch (mStatus2)
			{

			case eStart2:
				// Find BinHex title and colon
				mFileCreated = false;
				while (::strncmp((char*) p, cBinHexTitle, ::strlen(cBinHexTitle)) != 0)
				{
					p++;
					p = (unsigned char*) ::memchr(p, cBinHexTitle[0], mBufferLength);
					if (!p)
						return mDecodedOne ? (ExceptionCode)noErr : (ExceptionCode)writErr;
					mBufferLength = inByteCount - (p - reinterpret_cast<const unsigned char*>(inBuffer));
					if (mBufferLength < 1)
						return mDecodedOne ? (ExceptionCode)noErr : (ExceptionCode)writErr;
				}

				// Step over everything until colon
				while ((*p != ':') && (mBufferLength > 0))
				{
					p++;
					mBufferLength--;
					if (mBufferLength < 1) return writErr;
				};

				// Step over colon
				p++;
				mBufferLength--;

				// Reset state
				mLength = 1;				// Get single byte
				mAtomPos = 0;				// First of atom
				mCRC = 0;					// Reset CRC
				mBufferPos = &mDecodedNameLen;					// Decode into file name length
				mStatus2 = eGetStrLen;
				mSubStatus2 = eNormalChar;
				
				// Set flag to indicate we have found at least one item
				mDecodedOne = true;
				break;

				case eGetStrLen:
					DecodeIt(&p, false, false);		// Get one decoded byte

					// Got string length
					if (!mLength) {
						if (mDecodedNameLen > 240)
						{
							CLOG_LOGTHROW(CGeneralException, eStrToLong);
							throw CGeneralException(eStrToLong);
						}
						mLength = mDecodedNameLen;					// Get length of name string
						mDecodedName.reserve(mDecodedNameLen);		// Reserve space
						mBufferPos = (unsigned char*) (const char*) mDecodedName;		// Decode into body of file name
						mStatus2 = eGetStr;
					}
					break;

				case eGetStr:
				{
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						mLength = 1;						// Get null at end of string
						mBufferPos = &mLastChar;			// Throw it away
						mStatus2 = eGetStrNull;
					}
					break;
				}

				case eGetStrNull:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						mLength = 4;				// Get an OSType
#if __dest_os == __mac_os || __dest_os == __mac_os_x
						mBufferPos = (unsigned char*) &mDecodedInfo.file.fileType;	// Decode into file type
#else
						mBufferPos = (unsigned char*) &mDummy;				// Throw away file type
#endif
						mStatus2 = eGetType;
					}
					break;

				case eGetType:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						mLength = 4;				// Get an OSType
#if __dest_os == __mac_os || __dest_os == __mac_os_x
						mBufferPos = (unsigned char*) &mDecodedInfo.file.fileCreator;	// Decode into creator
#else
						mBufferPos = (unsigned char*) &mDummy;					// Throw away creator
#endif
						mStatus2 = eGetCreator;
					}
					break;

				case eGetCreator:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						mLength = 2;				// Get flags
#if __dest_os == __mac_os || __dest_os == __mac_os_x
						mBufferPos = (unsigned char*) &mDecodedInfo.file.finderFlags;		// Decode to Finder flags
#else
						mBufferPos = (unsigned char*) &mDummy;						// Throw away Finder flags
#endif
						mStatus2 = eGetFlags;
					}
					break;

				case eGetFlags:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						mLength = 4;				// Get length
						mBufferPos = (unsigned char*) &mDataLen;		// Decode into data length
						mStatus2 = eGetDataLen;
					}
					break;

				case eGetDataLen:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						// Convert to host order
						mDataLen = ntohl(mDataLen);

						mLength = 4;				// Get length
						mBufferPos = (unsigned char*) &mRsrcLen;		// Decode into resource length
						mStatus2 = eGetRsrcLen;
					}
					break;

				case eGetRsrcLen:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						// Convert to host order
						mRsrcLen = ntohl(mRsrcLen);
						mUseRsrc = (mRsrcLen != 0);

						// Do final CRC calc and store value
						IncrementCRC(0);
						IncrementCRC(0);
						mCRCCalc = mCRC;					// Cache calculated CRC
						mLength = 2;						// Get stored CRC
						mBufferPos = (unsigned char*) &mCRCFound;	// Decode into CRC value
						mStatus2 = eGetHeadCRC;
					}
					break;

				case eGetHeadCRC:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						// Convert to host order
						mCRCFound = ntohs(mCRCFound);

						// Check CRC's
						if (mCRCCalc != mCRCFound)
						{
							CLOG_LOGTHROW(CGeneralException, eWrongHeadCRC);
							throw CGeneralException(eWrongHeadCRC);
						}

						// Create file and refresh any progress pane
						CreateFile();
						if (mProgress) mProgress->Redraw();

						mCRC = 0;				// Restart CRC
						mLength = mDataLen;		// Get all the data
						mBufferPos = NULL;		// Will store int file
						if (mFileCreated && mLength) {
#if __dest_os == __mac_os || __dest_os == __mac_os_x
							mFileStream->OpenDataFork(fsWrPerm);
							::SetFPos(mFileStream->GetDataForkRefNum(), fsFromStart, 0L);
#elif __dest_os == __win32_os || __dest_os == __linux_os
							// Assume opened in CreateFile
#else
#error __dest_os
#endif
						}

						mStatus2 = eGetData;
					}
					break;

				case eGetData:
					DecodeIt(&p, true, false);

					// Got string
					if (!mLength) {
						// Do final CRC calc and store value
						IncrementCRC(0);
						IncrementCRC(0);
						mCRCCalc = mCRC;					// Cache actual CRC
						mLength = 2;						// Get stored CRC
						mBufferPos = (unsigned char*) &mCRCFound;	// Decode into CRC
						mStatus2 = eGetDataCRC;
					}
					break;

				case eGetDataCRC:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						// Convert to host order
						mCRCFound = ntohs(mCRCFound);

						// Check CRC's
						if (mCRCCalc != mCRCFound)
						{
							CLOG_LOGTHROW(CGeneralException, eWrongDataCRC);
							throw CGeneralException(eWrongDataCRC);
						}

						mCRC = 0;							// Restart CRC
						mLength = mRsrcLen;					// Get all the resource data
						mBufferPos = NULL;					// Will store in file
						if (mFileCreated && mLength) {
#if __dest_os == __mac_os || __dest_os == __mac_os_x
							mFileStream->OpenResourceFork(fsWrPerm);
							::SetFPos(mFileStream->GetResourceForkRefNum(), fsFromStart, 0L);
#elif __dest_os == __win32_os || __dest_os == __linux_os
							// Windows has no resource fork
#else
#error __dest_os
#endif
						}

						mStatus2 = eGetRsrc;
					}
					break;

				case eGetRsrc:
					DecodeIt(&p, false, true);

					// Got string
					if (!mLength) {
						// Do final CRC calc and store value
						IncrementCRC(0);
						IncrementCRC(0);
						mCRCCalc = mCRC;					// Cache actual CRC
						mLength = 2;						// Get stored CRC
						mBufferPos = (unsigned char*) &mCRCFound;	// Decode to CRC
						mStatus2 = eGetRsrcCRC;
					}
					break;

				case eGetRsrcCRC:
					DecodeIt(&p, false, false);

					// Got string
					if (!mLength) {
						// Convert to host order
						mCRCFound = ntohs(mCRCFound);

						// Check CRC's
						if (mCRCCalc != mCRCFound)
						{
							CLOG_LOGTHROW(CGeneralException, eWrongRsrcCRC);
							throw CGeneralException(eWrongRsrcCRC);
						}

						mLength = 2;			// Look for colon and line end
						mBufferPos = &mLastChar;			// Throw away

						mStatus2 = eGetEndColon;

					}
					break;

				case eGetEndColon:
					// Bump over CRLFs
					while((*p == '\r') || (*p == '\n'))
					{
						p++;
						mBufferLength--;
					}

					// Need to skip incomplete atom (2 or 3)
					if ((mAtomPos == 2) && (*p != ':'))
					{
						p++;
						mBufferLength--;
						mAtomPos++;
					}

					// Bump over CRLFs
					while((*p == '\r') || (*p == '\n'))
					{
						p++;
						mBufferLength--;
					}

					if ((mAtomPos == 3) && (*p != ':'))
					{
						p++;
						mBufferLength--;
					}

					// Now check for end

					// Bump over CRLFs
					while((*p == '\r') || (*p == '\n'))
					{
						p++;
						mBufferLength--;
					}

					// Look for colon
					if (*p == ':')
					{
						p++;
						mBufferLength--;
						if (mBufferLength && ((*p == '\n') || (*p == '\r')))
						{
							p++;
							mBufferLength--;
							if (mBufferLength && ((*p == '\n') || (*p == '\r')))
							{
								p++;
								mBufferLength--;
							}
							else if (mBufferLength)
							{
								CLOG_LOGTHROW(CGeneralException, eNoTerminator);
								throw CGeneralException(eNoTerminator);
							}
						}
						else if (mBufferLength)
						{
							CLOG_LOGTHROW(CGeneralException, eNoTerminator);
							throw CGeneralException(eNoTerminator);
						}

						// Completed file - close
						if (mFileCreated)
						{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
							mFileStream->CloseDataFork();
							mFileStream->CloseResourceFork();
#elif __dest_os == __win32_os
							// mFileStream->Close(); <- Don't do this as file name is wiped
#elif __dest_os == __linux_os
							mFileStream->Close(); //File name won't be wiped
#else
#error __dest_os
#endif
						}
						mFileCreated = false;

						// Update progress
						if (mProgress)
							mProgress->SetPercentage(100);

						mStatus2 = eStart2;
					}
					else
					{
						CLOG_LOGTHROW(CGeneralException, eNoTerminator);
						throw CGeneralException(eNoTerminator);
					}
					break;

				default:;
				}

		}

		inByteCount = inByteCount - mBufferLength;
	}

	catch (CGeneralException& ex)
	{
		CLOG_LOGCATCH(CGeneralException);

		// If file exists delete it
		if (mFileCreated)
		{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			mFileStream->CloseDataFork();
			mFileStream->CloseResourceFork();

			PPx::FSObject aSpec;
			mFileStream->GetSpecifier(aSpec);
			if (aSpec.Exists())
				aSpec.Delete();
#else
			cdstring fpath = mFileStream->GetFilePath();	// Cache path as Abort empties it
			mFileStream->Abort();
			::remove_utf8(fpath);
#endif
			mFileCreated = false;

			// Determine error string and display alert
			const char* rsrcid = NULL;
			switch (ex.GetErrorCode())
			{
				case eIllegalCode:
					rsrcid = "Alerts::Attachments::BinHexIllegalCode";
					break;

				case eStrToLong:
					rsrcid = "Alerts::Attachments::BinHexStrTooLong";
					break;

				case eNoStrNull:
					rsrcid = "Alerts::Attachments::BinHexNoStrNull";
					break;

				case eWrongHeadCRC:
					rsrcid = "Alerts::Attachments::BinHexWrongHeadCRC";
					break;

				case eWrongDataCRC:
					rsrcid = "Alerts::Attachments::BinHexWrongDataCRC";
					break;

				case eWrongRsrcCRC:
					rsrcid = "Alerts::Attachments::BinHexWrongRsrcCRC";
					break;

				case eNoTerminator:
					rsrcid = "Alerts::Attachments::BinHexNoTerminator";
					break;

				case eNoMore:
					rsrcid = "Alerts::Attachments::BinHexNoMore";
					break;

				default:
					rsrcid = "Alerts::Attachments::BinHexNoDecode";

			}

			CErrorHandler::PutStopAlertRsrc(rsrcid);
		}

		err = writErr;
	}

	return err;
}

// Initialise file to encode
void CBinHexFilter::InputFileInit()
{
	mLength = 0;

	// Start from beginning

	// There may not be a resource fork
	try {
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (mFileStream->GetDataForkRefNum() != refNum_Undefined)
			ThrowIfOSErr_(::SetFPos(mFileStream->GetDataForkRefNum(), fsFromStart, 0L));
		if (mFileStream->GetResourceForkRefNum() != refNum_Undefined)
			ThrowIfOSErr_(::SetFPos(mFileStream->GetResourceForkRefNum(), fsFromStart, 0L));
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

// Create unencoded binhex header in
void CBinHexFilter::CreateHeader()
{
	unsigned char* p = mBuffer;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	PPx::FSObject	spec;
	mFileStream->GetSpecifier(spec);

	// Copy file name length
	cdstring name(spec.GetName());
	*p++ = name.length();

	// Copy file name
	::strcpy((char*) p, name.c_str());
	p += name.length();

	// Copy delimiter
	*p++ = '\0';

	// Copy type, creator, flags
	FinderInfo finfo;
	spec.GetFinderInfo(&finfo, NULL, NULL);

	*((OSType*) p) = finfo.file.fileType;
	p += sizeof(OSType);
	*((OSType*) p) = finfo.file.fileCreator;
	p += sizeof(OSType);
	*((short*) p) = finfo.file.finderFlags  & ~(kIsOnDesk | kIsInvisible | kHasBeenInited);
	p += sizeof(short);

	// Ddata & resource fork lengths
	FSCatalogInfo catInfo;
	spec.GetCatalogInfo(kFSCatInfoDataSizes | kFSCatInfoRsrcSizes, catInfo);

	*((long*) p) = catInfo.dataLogicalSize;
	p += sizeof(long);
	*((long*) p) = catInfo.rsrcLogicalSize;
	p += sizeof(long);
#else
	cdstring fname = mFileStream->GetFileName();
	int length = ::strlen(fname);

	// Copy file name length
	*p++ = length;

	// Copy file name
	::strcpy((char*) p, fname);
	p += length;

	// Copy delimiter
	*p++ = '\0';

	// Copy type, creator, flags, data & resource fork lengths
	*((OSType*) p) = htonl('TEXT');
	p += sizeof(OSType);
	*((OSType*) p) = htonl('ttxt');
	p += sizeof(OSType);
	*((short*) p) = 0;
	p += sizeof(short);
	*((long*) p) = htonl(mFileStream->GetLength());
	p += sizeof(long);
	*((long*) p) = 0;
	p += sizeof(long);
#endif
	// Update buffer length
	mBufferLength = p - mBuffer;

}

// Encode into binhex
void CBinHexFilter::Encode(unsigned char** outbuffer, long* total, long maxbuffer)
{
	unsigned char* p = mBuffer;
	unsigned char** q = outbuffer;

	while((mBufferLength ||
			((mAtomPos == 3) && (mStatus1 != eTerminate)) ||
			(mProcess != eNextChar) && (mStatus1 != eTerminate)) &&
			!(*total >= maxbuffer)) {

		switch (mSubStatus1) {

			case eNormal:
				// Process for atom pos 0,1,2
				if (mAtomPos < 3) {
					switch (mProcess) {

						case eFirstChar:

							mLastChar = *p++;
							mBufferLength--;
							IncrementCRC(mLastChar);
#ifdef big_endian
							mCurrentAtom.base256[mAtomPos] = mLastChar;
#else
							mCurrentAtom.base256[2 - mAtomPos] = mLastChar;
#endif
							mProcess = (mLastChar != 0x90) ? eNextChar : eRepeater;
							break;

						case eNextChar:

							if (mLastChar != *p) {
								mLastChar = *p++;
								mBufferLength--;
								IncrementCRC(mLastChar);
#ifdef big_endian
								mCurrentAtom.base256[mAtomPos] = mLastChar;
#else
								mCurrentAtom.base256[2 - mAtomPos] = mLastChar;
#endif
								mProcess = (mLastChar != 0x90) ? eNextChar : eRepeater;
							}
							else {
								// Do repeating
								mRunLength = 1;
								while(mBufferLength && (mLastChar == *p) && (mRunLength < 0xFF)) {
									p++;
									mBufferLength--;
									mRunLength++;
									IncrementCRC(mLastChar);
								}
								if (mRunLength > 2) {
#ifdef big_endian
									mCurrentAtom.base256[mAtomPos] = 0x90;
#else
									mCurrentAtom.base256[2 - mAtomPos] = 0x90;
#endif
									mProcess = eRunNext;
								}
								else {
#ifdef big_endian
									mCurrentAtom.base256[mAtomPos] = mLastChar;
#else
									mCurrentAtom.base256[2 - mAtomPos] = mLastChar;
#endif
									mProcess = (mLastChar != 0x90) ? eNextChar : eRepeater;
								}
							}
							break;

						case eRunNext:

#ifdef big_endian
							mCurrentAtom.base256[mAtomPos] = mRunLength;
#else
							mCurrentAtom.base256[2 - mAtomPos] = mRunLength;
#endif
							mProcess = eNextChar;
							break;

						case eRepeater:

#ifdef big_endian
							mCurrentAtom.base256[mAtomPos] = 0x00;
#else
							mCurrentAtom.base256[2 - mAtomPos] = 0x00;
#endif
							mProcess = eNextChar;
							break;
					}
				}

				switch (mAtomPos) {

					case 0:
						// Copy next char
						*(*q)++ = cbinhex[mCurrentAtom.base64.b0];
						break;

					case 1:
						// Copy next char
						*(*q)++ = cbinhex[mCurrentAtom.base64.b1];
						break;

					case 2:
						// Copy next char
						*(*q)++ = cbinhex[mCurrentAtom.base64.b2];
						break;

					case 3:
						// Copy next char
						*(*q)++ = cbinhex[mCurrentAtom.base64.b3];
						break;
				}

				(mAtomPos < 3) ? mAtomPos++ : mAtomPos = 0;
				(*total)++;
				mLineCount++;
				if (mLineCount > 63) mSubStatus1 = eCRNext;
				break;

			case eCRNext:
				*(*q)++ = ::get_endl(mOutEndl)[0];
				(*total)++;
				mLineCount = 0;
				mSubStatus1 = (::get_endl_len(mOutEndl) == 2 ? eLFNext : eNormal);
				break;

			case eLFNext:
				*(*q)++ = ::get_endl(mOutEndl)[1];
				(*total)++;
				mLineCount = 0;
				mSubStatus1 = eNormal;
				break;

		}
	}

	// Copy remaining to start of buffer
	if (mBufferLength && (*total >= maxbuffer))
		::memmove(mBuffer, p, mBufferLength);

}

// Decode binhex into plain text
void CBinHexFilter::DecodeIt(unsigned char** inbuffer,
								bool to_data,
								bool to_rsrc)
{
	// Buffer output to file in this routine
	unsigned char* file_buf = mFileOutBuffer;
	SInt32 file_pos = 0;

	// Loop while input/output are not empty/full
	while (mLength && mBufferLength) {

		switch (mSubStatus2) {

			// Normal character or start of run
			case eNormalChar:
			case eStartRun:
			{
				// Read char from input
				unsigned char got_char;
				char read_byte = *(*inbuffer)++;
				mBufferLength--;

				// Do not decode CRLF
				if ((read_byte != '\r') && (read_byte != '\n')) {

					// Decode byte => 6-bit
					signed char decoded_6bit = cdebinhex[(unsigned char) read_byte];
					if (decoded_6bit == -1)
					{
						CLOG_LOGTHROW(CGeneralException, eIllegalCode);
						throw CGeneralException(eIllegalCode);
					}

					// Put decoded bits into proper atom pos
					switch (mAtomPos) {

						case 0:
							// Read a byte into atom
							mCurrentAtom.base64.b0 = decoded_6bit & 0x3F;
							mAtomPos++;
							break;

						case 1:
							// Read next byte, do decode with CRC
							mCurrentAtom.base64.b1 = decoded_6bit & 0x3F;
							mAtomPos++;
#ifdef big_endian
							got_char = mCurrentAtom.base256[0];
#else
							got_char = mCurrentAtom.base256[2];
#endif
							break;

						case 2:
							// Read next byte, do decode with CRC
							mCurrentAtom.base64.b2 = decoded_6bit & 0x3F;
							mAtomPos++;
#ifdef big_endian
							got_char = mCurrentAtom.base256[1];
#else
							got_char = mCurrentAtom.base256[1];
#endif
							break;

						case 3:
							// Read next byte, do decode with CRC
							mCurrentAtom.base64.b3 = decoded_6bit & 0x3F;
							mAtomPos = 0;
#ifdef big_endian
							got_char = mCurrentAtom.base256[2];
#else
							got_char = mCurrentAtom.base256[0];
#endif
							break;
					}

					// Process atom
					switch (mSubStatus2) {

						case eNormalChar:
							if (mAtomPos != 1) {
								// Check for start of run
								if (got_char != 0x90) {
									// No run => add to output
									mLastChar = got_char;
									if (to_data || to_rsrc) {
										*file_buf++ = mLastChar;
										file_pos++;
									}
									else
										*mBufferPos++ = mLastChar;
									IncrementCRC(mLastChar);
									mLength--;
								}
								else
									// Signal start of run
									mSubStatus2 = eStartRun;
							}
							break;

						case eStartRun:
							if (mAtomPos != 1) {
								// Check for run or 0x90 char
								if (got_char == 0x00) {
									// No run => add 0x90 to output
									mLastChar = 0x90;
									if (to_data || to_rsrc) {
										*file_buf++ = mLastChar;
										file_pos++;
									}
									else
										*mBufferPos++ = mLastChar;
									IncrementCRC(mLastChar);
									mLength--;
									mSubStatus2 = eNormalChar;
								}
								else {
									// Start run
									mRunLength = got_char - 1;
									mSubStatus2 = eDoingRun;
								}
							}
							break;

						default:;
					}
				}
				break;
			}

			// Carry on with run
			case eDoingRun:
				if (to_data || to_rsrc) {
					*file_buf++ = mLastChar;
					file_pos++;
				}
				else
					*mBufferPos++ = mLastChar;
				IncrementCRC(mLastChar);
				mLength--;
				mRunLength--;
				if (!mRunLength)
					mSubStatus2 = eNormalChar;
				break;
		}

		// Write out file buffer if full and reset
		if (file_pos >= cMaxBuffer) {
			if (mFileCreated) {
				if (to_data)
					mFileStream->WriteBlock(mFileOutBuffer, file_pos);
				else if (to_rsrc)
					mFileStream->PutResourceBytes(mFileOutBuffer, file_pos);
			}

			file_buf = mFileOutBuffer;
			file_pos = 0;

			// Update progress
			if (mProgress && (mDataLen + mRsrcLen != 0))
			{
				if (to_data)
					mProgress->SetPercentage(((mDataLen - mLength)*100)/(mDataLen + mRsrcLen));
				else if (to_rsrc)
					mProgress->SetPercentage(((mDataLen + mRsrcLen - mLength)*100)/(mDataLen + mRsrcLen));
			}
		}
	}

	// Always write out file buffer
	if (mFileCreated && file_pos) {
		if (to_data)
			mFileStream->WriteBlock(mFileOutBuffer, file_pos);
		else if (to_rsrc)
			mFileStream->PutResourceBytes(mFileOutBuffer, file_pos);
	}
}

// Increment CRC

#if 1

void CBinHexFilter::IncrementCRC(unsigned char c)
{
	register unsigned short v = (0 | c);
	register unsigned short crc = mCRC;
	register bool temp;
	register short i;

	for(i = 1; i <= 8; i++) {
		temp = ((crc & 0x8000) != 0);
		crc = (crc << 1) | (v >> 7);
		if (temp) crc ^= 0x1021;
		v = (v << 1) & 0x00FF;
	}

	mCRC = crc;
}

#elif __powerc

asm void CBinHexFilter::IncrementCRC(unsigned char c)
{
	// v = (c | 0)
	// Already in r4

	// crc = mCRC
	lhz			r5, CBinHexFilter.mCRC(r3)

	// i = 8
	li			r6, 8
	mtctr		r6

	// do {
	// temp = ((crc & 0x8000) != 0);
@1:	andi.		r0, r5, 0x8000

	// crc = (crc << 1);
	rlwinm		r5, r5, 1, 16, 30

	// crc =| (v >> 7)
	rlwinm		r7, r4, 25, 31, 31
	or			r5, r5, r7

	// if (temp) crc ^= 0x1021
	beq			@2
	xori		r5, r5, 0x1021

	// v << 1
@2:	rlwinm		r4, r4, 1, 24, 30
	bdnz+		@1
	// } while (i!=0)

	// mCRC = crc
	sth			r5, CBinHexFilter.mCRC(r3)

	blr
}

#elif __MC68K__

asm void CBinHexFilter::IncrementCRC(unsigned char c)
{
	// v = (c | 0)
	moveq		#0, D0
	move.b		0x0008(A7), D0

	// crc = mCRC
	move.l		0x0004(A7), A0
	move.w		struct(CBinHexFilter.mCRC)(A0), D1

	// i = 7
	moveq		#7, D2

	// do {
@1:
	lsl.b		#1, D0
	roxl.w		#1, D1
	bcc.s		@2
	eori		#0x1021, D1
@2:
	// } while (i!=-1)
	dbra		D2, @1

	// mCRC = crc
	move.w		D1, struct(CBinHexFilter.mCRC)(A0)

	rts
}

#endif
