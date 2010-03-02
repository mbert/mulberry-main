/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CAFFilter class

#include "CAFFilter.h"

#include "CGeneralException.h"
#include "CLog.h"
#include "CProgress.h"

#if __dest_os == __win32_os
#include <winsock.h>
#elif __dest_os == __linux_os
#include <jFileUtil.h>

#include <netinet/in.h>
#endif

#pragma mark const

const short refNum_Undefined	= -1;

const unsigned long cAppleFile_RealNameIndex = 0;
const unsigned long cAppleFile_FinderInfoIndex = 1;
const unsigned long cAppleFile_FileDatesIndex = 2;
const unsigned long cAppleFile_CommentIndex = 3;
const unsigned long cAppleFile_RsrcForkIndex = 4;
const unsigned long cAppleFile_DataForkIndex = 5;

const unsigned long cAppleFile_AS = 0x00051600;
const unsigned long cAppleFile_AD = 0x00051607;
const unsigned long cAppleFile_Version2 = 0x00020000;
const unsigned short cAppleFile_ASEntries = 6;
const unsigned short cAppleFile_ADEntries = 5;

const unsigned long cAppleFile_ASHeaderSize = sizeof(SAppleFileHeader);
const unsigned long cAppleFile_ADHeaderSize = sizeof(SAppleFileHeader) - sizeof(SAppleFileEntry);
const unsigned long cAppleFile_ASIndexSize = sizeof(SAppleFileIndex);
const unsigned long cAppleFile_ADIndexSize = sizeof(SAppleFileIndex) - sizeof(SAppleFileEntry);

const unsigned long cAppleFile_ASData = 1;
const unsigned long cAppleFile_ASResource = 2;
const unsigned long cAppleFile_ASRealName = 3;
const unsigned long cAppleFile_ASComment = 4;
const unsigned long cAppleFile_ASIconBW = 5;
const unsigned long cAppleFile_ASIconColour = 6;
const unsigned long cAppleFile_ASFileDates = 8;
const unsigned long cAppleFile_ASFinderInfo = 9;
const unsigned long cAppleFile_ASMacInfo = 10;
const unsigned long cAppleFile_ASProDocInfo = 11;
const unsigned long cAppleFile_ASMSDocInfo = 12;
const unsigned long cAppleFile_ASAFPName = 13;
const unsigned long cAppleFile_ASAFPInfo = 14;
const unsigned long cAppleFile_ASAFPDirID = 15;

const unsigned long cAppleFile_DateUnknown = 0x80000000;
const unsigned long cAppleFile_MacToASDate = 0xB492F401;

#if __dest_os != __mac_os && __dest_os != __mac_os_x

#if __dest_os == __win32_os
#pragma pack(push, 2)
#endif


typedef long Point;

struct FInfo {
	OSType							fdType;						/*the type of the file*/
	OSType							fdCreator;					/*file's creator*/
	unsigned short					fdFlags;					/*flags ex. hasbundle,invisible,locked, etc.*/
	Point							fdLocation;					/*file's location in folder*/
	short							fdFldr;						/*folder containing file*/
}
#if __dest_os == __linux_os
//__attribute__ ((packed))
#endif
;
typedef struct FInfo FInfo;

struct FXInfo {
	short							fdIconID;					/*Icon ID*/
	short							fdUnused[3];				/*unused but reserved 6 bytes*/
	signed char						fdScript;					/*Script flag and number*/
	signed char						fdXFlags;					/*More flag bits*/
	short							fdComment;					/*Comment ID*/
	long							fdPutAway;					/*Home Dir ID*/
}
#if __dest_os == __linux_os
//__attribute__ ((packed))
#endif
;

typedef struct FXInfo FXInfo;
#if __dest_os == __win32_os
#pragma pack(pop)
#endif
#endif

// __________________________________________________________________________________________________
// C L A S S __ C A F F I L T E R
// __________________________________________________________________________________________________

// Constructor
CAFFilter::CAFFilter(LStream* aStream, CProgress* progress) : CFilter(aStream, progress)
{
	InitAFFilter();

	// Store cast local copy
	mFileStream = (CFullFileStream*) aStream;
}

CAFFilter::~CAFFilter()
{
	mFileStream = NULL;
	delete mHeaderBuffer;
	mHeaderBuffer = NULL;
}

void CAFFilter::InitAFFilter()
{
	// Init params
	mFileStream = nil;
	mSingle = true;
	mHeaderBuffer = nil;

	mStatus1 = eStart1;
	mStatus2 = eStart2;

	// Force finite length, marker at start
	// These must be different so that when called from another filter it is not immediately 'complete'
	mLength = 0x7FFFFFFF;
	mMarker = 0;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

bool CAFFilter::Complete() const
{
	return ((mStatus1 == eEnd1) || (mStatus2 == eEnd2));
}

// Encode data from source into AppleSingle for output
ExceptionCode CAFFilter::GetBytes(void* outBuffer, long& inByteCount)
{
	long total = 0;
	bool done = false;
	char* p = (char*) outBuffer;

	unsigned long len;
	long max;

	// Update progress at start
	if (mProgress && (GetMarker() == 0))
		mProgress->Reset();

	while((total < inByteCount) && !done)
	{
		switch(mStatus1)
		{
		case eStart1:
			mStatus1 = eCreateHeader;
			mLength = 0;

			// Fall through

		case eCreateHeader:
			// Create buffer from file
			CreateHeader();
			mStatus1 = eHeaderCopy;

			// Fall through

		case eHeaderCopy:
			// Fill output with as much as possible
			max = (mHeaderBufferLen >= (inByteCount - total)) ? (inByteCount - total) : mHeaderBufferLen;
			::memcpy(p, mHeaderBuffer, max);
			p += max;
			total += max;

			// Adjust header if not all copied
			if (max < mHeaderBufferLen)
			{
				::memmove(mHeaderBuffer, mHeaderBuffer + max, mHeaderBufferLen - max);
				mHeaderBufferLen -= max;
			}
			else
			{
				// All copied so change state
				mStatus1 = eRsrcForkCopy;
				
				// Reset to start of stream in case marker has moved for some reason
				mFileStream->SetRsrcMarker(0, streamFrom_Start);
			}

			break;

		case eRsrcForkCopy:
		{
			// Fill output with as much as possible from resource fork
			len = ntohl(mIndex.entries[cAppleFile_RsrcForkIndex].entryLen);
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;
			OSErr err = mFileStream->GetResourceBytes(p, max);
			if ((err != noErr) && (err != readErr) && (err != eofErr))
			{
				CLOG_LOGTHROW(CGeneralException, err);
				throw CGeneralException(err);
			}
			p += max;
			total += max;
			mMarker += max;
			mIndex.entries[cAppleFile_RsrcForkIndex].entryLen = htonl(ntohl(mIndex.entries[cAppleFile_RsrcForkIndex].entryLen) - max);

			// All copied so change state
			if (!mIndex.entries[cAppleFile_RsrcForkIndex].entryLen)
			{
				// All copied so change state
				mStatus1 = (mSingle ? eDataForkCopy : eEnd1);
				
				// Reset to start of stream in case marker has moved for some reason
				if (mStatus1 == eDataForkCopy)
					mFileStream->SetMarker(0, streamFrom_Start);
			}
			break;
		}

		case eDataForkCopy:
			// Fill output with as much as possible from data fork
			len = ntohl(mIndex.entries[cAppleFile_DataForkIndex].entryLen);
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;
			mFileStream->GetBytes(p, max);
			p += max;
			total += max;
			mMarker += max;
			mIndex.entries[cAppleFile_DataForkIndex].entryLen = htonl(ntohl(mIndex.entries[cAppleFile_DataForkIndex].entryLen) - max);

			// All copied so change state
			if (!mIndex.entries[cAppleFile_DataForkIndex].entryLen)
				mStatus1 = eEnd1;

			break;

		case eEnd1:
			done = true;

		default:;
		}
	}

	// Update progress
	if (mProgress)
		mProgress->SetPercentage((GetMarker()*100)/mStream->GetLength());

	inByteCount = total;
	return noErr;
}

// Decode data from AppleSingle for output
ExceptionCode CAFFilter::PutBytes(const void *inBuffer, SInt32& inByteCount)
{
	long total = 0;
	bool	done = false;
	char* p = (char*) inBuffer;

	while((total < inByteCount) && !done)
	{
		long len;
		long max;
		switch(mStatus2)
		{
		case eStart2:
		{
			// Init offset counter
			mGetOffset = 0;
			mGetTotal = 0;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Get output file details
			PPx::FSObject fspec;
			mFileStream->GetSpecifier(fspec);
			FSSpec spec;
			fspec.GetFSSpec(spec);

			// Get existing file info
			mGetInfo.hFileInfo.ioCompletion = nil;
			mGetInfo.hFileInfo.ioNamePtr = spec.name;
			mGetInfo.hFileInfo.ioVRefNum = spec.vRefNum;
			mGetInfo.hFileInfo.ioFDirIndex = 0;
			mGetInfo.hFileInfo.ioDirID = spec.parID;
			ThrowIfOSErr_(::PBGetCatInfoSync(&mGetInfo));

			*mGetComment = 0;
#else
			// Other OSes will not handle file info
#endif
			mStatus2 = eGetHeader;

		}
			// Fall through

		case eGetHeader:
			{			
			// Add as much to file as possible
			len = sizeof(SAppleFileHeader);
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;

			// Must have full data now
			if (max != len)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Copy header
			::memcpy(&mIndex.header, p, max);
			p += max;
			total += max;
			mGetOffset += max;

			// Convert elements in header into host order
			mIndex.header.magicNum = ntohl(mIndex.header.magicNum);
			mIndex.header.versionNum = ntohl(mIndex.header.versionNum);
			mIndex.header.numEntries = ntohs(mIndex.header.numEntries);

			// All copied so change state
			mStatus2 = eGetEntries;

			break;

			// Fall through
			}
		case eGetEntries:
			while(mIndex.header.numEntries--)
			{
				SAppleFileEntry entry;

				// Add as much to file as possible
				len = sizeof(SAppleFileEntry);
				max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;

				// Must have full data now
				if (max != len)
				{
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}

				// Copy entry if equal to one we want
				::memcpy(&entry, p, max);

				// Convert elements in entry into host order
				entry.entryID = ntohl(entry.entryID);
				entry.entryOffset = ntohl(entry.entryOffset);
				entry.entryLen = ntohl(entry.entryLen);

				switch(entry.entryID)
				{
					case cAppleFile_ASRealName:
						mIndex.entries[cAppleFile_RealNameIndex] = entry;
						break;

					case cAppleFile_ASFinderInfo:
						mIndex.entries[cAppleFile_FinderInfoIndex] = entry;
						break;

					case cAppleFile_ASFileDates:
						mIndex.entries[cAppleFile_FileDatesIndex] = entry;
						break;

					case cAppleFile_ASComment:
						mIndex.entries[cAppleFile_CommentIndex] = entry;
						break;

					case cAppleFile_ASResource:
						mIndex.entries[cAppleFile_RsrcForkIndex] = entry;
						break;

					case cAppleFile_ASData:
						mIndex.entries[cAppleFile_DataForkIndex] = entry;
						break;

					// Ignore all others
					default:;
				}
				p += max;
				total += max;
				mGetOffset += max;

				// Get max total
				if (mGetTotal < entry.entryOffset + entry.entryLen)
					mGetTotal = entry.entryOffset + entry.entryLen;
			}

			// All copied so change state
			mStatus2 = eProcessEntries;

			break;

		case eProcessEntries:
			// Check for completion
			if (mGetOffset == mGetTotal)
				mStatus2 = eEnd2;

			// Check real name
			else if (mGetOffset == mIndex.entries[cAppleFile_RealNameIndex].entryOffset)
			{
				mIndex.entries[cAppleFile_RealNameIndex].entryOffset = 0;
				mStatus2 = eGetRealName;
			}

			// Check finder info
			else if (mGetOffset == mIndex.entries[cAppleFile_FinderInfoIndex].entryOffset)
			{
				mIndex.entries[cAppleFile_FinderInfoIndex].entryOffset = 0;
				mStatus2 = eGetFinderInfo;
			}

			// Check file dates info
			else if (mGetOffset == mIndex.entries[cAppleFile_FileDatesIndex].entryOffset)
			{
				mIndex.entries[cAppleFile_FileDatesIndex].entryOffset = 0;
				mStatus2 = eGetFileDates;
			}

			// Check comments
			else if (mGetOffset == mIndex.entries[cAppleFile_CommentIndex].entryOffset)
			{
				mIndex.entries[cAppleFile_CommentIndex].entryOffset = 0;
				mStatus2 = eGetComment;
			}

			// Check resource entry
			else if (mGetOffset == mIndex.entries[cAppleFile_RsrcForkIndex].entryOffset)
			{
				mIndex.entries[cAppleFile_RsrcForkIndex].entryOffset = 0;
				mStatus2 = eGetRsrc;
			}

			// Check data entry
			else if (mGetOffset == mIndex.entries[cAppleFile_DataForkIndex].entryOffset)
			{
				mIndex.entries[cAppleFile_DataForkIndex].entryOffset = 0;
				mStatus2 = eGetData;
			}

			// Unknown offset
			else
			{
				// Increment by one to ignore
				p++;
				total++;
				mGetOffset++;
			}
			break;

		case eGetRealName:
			// Add as much to file as possible
			len = mIndex.entries[cAppleFile_RealNameIndex].entryLen;
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;

			// Must have full data now
			if (max != len)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Copy name
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			::memcpy(&mGetRealName[1], p, max);
			mGetRealName[0] = max;
#else
			mGetRealName.assign(p, max);
#endif
			p += max;
			total += max;
			mIndex.entries[cAppleFile_RealNameIndex].entryLen -= max;
			mGetOffset += max;

			// All copied so change state
			if (!mIndex.entries[cAppleFile_RealNameIndex].entryLen)
				mStatus2 = eProcessEntries;

			break;

		case eGetFinderInfo:
			// Add as much to file as possible
			len = mIndex.entries[cAppleFile_FinderInfoIndex].entryLen;
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;

			// Must have full data now
			if (max != len)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			// Cope with bug where some clients send partial data
			else if (max == sizeof(FInfo) + sizeof(FXInfo))
			{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
				// Copy FInfo
				::memcpy(&mGetInfo.hFileInfo.ioFlFndrInfo, p, sizeof(FInfo));
				p += sizeof(FInfo);
				::memcpy(&mGetInfo.hFileInfo.ioFlXFndrInfo, p, sizeof(FXInfo));
				p += sizeof(FXInfo);
#else
				p += sizeof(FInfo);
				p += sizeof(FXInfo);
#endif
			}
			else
				p += max;

			total += max;
			mIndex.entries[cAppleFile_FinderInfoIndex].entryLen -= max;
			mGetOffset += max;

			// All copied so change state
			if (!mIndex.entries[cAppleFile_FinderInfoIndex].entryLen)
				mStatus2 = eProcessEntries;

			break;

		case eGetFileDates:
			// Add as much to file as possible
			len = mIndex.entries[cAppleFile_FileDatesIndex].entryLen;
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;

			// Must have full data now
			if (max != 4*sizeof(unsigned long))
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Copy file dates
			::memcpy(&mGetInfo.hFileInfo.ioFlCrDat, p, sizeof(unsigned long));
			mGetInfo.hFileInfo.ioFlCrDat += cAppleFile_MacToASDate;
			p += sizeof(unsigned long);
			::memcpy(&mGetInfo.hFileInfo.ioFlMdDat, p, sizeof(unsigned long));
			mGetInfo.hFileInfo.ioFlMdDat += cAppleFile_MacToASDate;
			p += sizeof(unsigned long);
			::memcpy(&mGetInfo.hFileInfo.ioFlBkDat, p, sizeof(unsigned long));
			mGetInfo.hFileInfo.ioFlBkDat += cAppleFile_MacToASDate;
			p += sizeof(unsigned long);
			p += sizeof(unsigned long);		// Ignore last access date
#else
			p += 4*sizeof(unsigned long);		// Ignore all dates
#endif
			total += max;
			mIndex.entries[cAppleFile_FileDatesIndex].entryLen -= max;
			mGetOffset += max;

			// All copied so change state
			if (!mIndex.entries[cAppleFile_FileDatesIndex].entryLen)
				mStatus2 = eProcessEntries;

			break;

		case eGetComment:
			// Add as much to file as possible
			len = mIndex.entries[cAppleFile_CommentIndex].entryLen;
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;

			// Must have full data now
			if (max != mIndex.entries[cAppleFile_CommentIndex].entryLen)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Copy file comment
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			::memcpy(&mGetComment[1], p, max);
			mGetComment[0] = max;
#endif
			p += max;
			total += max;
			mIndex.entries[cAppleFile_CommentIndex].entryLen -= max;
			mGetOffset += max;

			// All copied so change state
			if (!mIndex.entries[cAppleFile_CommentIndex].entryLen)
				mStatus2 = eProcessEntries;

			break;

		case eGetRsrc:
			// Add as much to file as possible
			len = mIndex.entries[cAppleFile_RsrcForkIndex].entryLen;
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;
			mFileStream->PutResourceBytes(p, max);
			p += max;
			total += max;
			mIndex.entries[cAppleFile_RsrcForkIndex].entryLen -= max;
			mGetOffset += max;

			// All copied so change state
			if (!mIndex.entries[cAppleFile_RsrcForkIndex].entryLen)
				mStatus2 = eProcessEntries;

			break;

		case eGetData:
			// Add as much to file as possible
			len = mIndex.entries[cAppleFile_DataForkIndex].entryLen;
			max = (len >= (inByteCount - total)) ? (inByteCount - total) : len;
			mFileStream->PutBytes(p, max);
			p += max;
			total += max;
			mIndex.entries[cAppleFile_DataForkIndex].entryLen -= max;
			mGetOffset += max;

			// All copied so change state
			if (!mIndex.entries[cAppleFile_DataForkIndex].entryLen)
				mStatus2 = eProcessEntries;

			break;

		case eEnd2:

			// Mark as done
			done = true;

		default:;
		}

		// Check for done
		if (mGetOffset == mGetTotal)
			done = true;
	}

	// Check for completion
	if (done)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Get output file details
		PPx::FSObject fspec;
		mFileStream->GetSpecifier(fspec);
		FSSpec spec;
		fspec.GetFSSpec(spec);

		mGetInfo.hFileInfo.ioNamePtr = spec.name;
		mGetInfo.hFileInfo.ioVRefNum = spec.vRefNum;
		mGetInfo.hFileInfo.ioDirID = spec.parID;

		// Now write out file info
		ThrowIfOSErr_(::PBSetCatInfoSync(&mGetInfo));

		// Now add comment
		if (*mGetComment)
		{
			// Get Desktop database refnum
			DTPBRec dtpb;

			dtpb.ioNamePtr = nil;
			dtpb.ioVRefNum = spec.vRefNum;
			ThrowIfOSErr_(::PBDTGetPath(&dtpb));

			// Set comment
			dtpb.ioCompletion = false;
			dtpb.ioNamePtr = spec.name;
			dtpb.ioDTBuffer = (char*) &mGetComment[1];
			dtpb.ioDTReqCount = mGetComment[0];
			dtpb.ioDirID = spec.parID;
			ThrowIfOSErr_(::PBDTSetCommentSync(&dtpb));
		}

		// Now change file name if required
#if 0	// Always use user's choosen name regardless of embedded name
		if (::PLstrcmp(spec.name, mGetRealName))
		{
			HParamBlockRec hpb;
			hpb.fileParam.ioCompletion = nil;
			hpb.fileParam.ioNamePtr = spec.name;
			hpb.fileParam.ioVRefNum = spec.vRefNum;
			hpb.fileParam.ioDirID = spec.parID;
			hpb.ioParam.ioMisc = (char*) mGetRealName;

			OSErr err = ::PBHRenameSync(&hpb);

			if (err && (err != dupFNErr))
				ThrowOSErr_(err);

			// Must update spec in file
			if (!err)
			{
				*spec.name = 0;
				::PLstrncpy(spec.name, mGetRealName, 31);	// Protect against buffer overflow
				mFileStream->SetSpecifier(spec);
			}
		}
#endif
		// Flush volume to ensure changes set in stone
		::FlushVol(nil, spec.vRefNum);
#elif __dest_os == __win32_os

		// Now change file name if required
#if 0	// Always use user's choosen name regardless of embedded name
		if (cdstring(mFileStream->GetFileName()) != mGetRealName)
		{
			CString old_path = mFileStream->GetFilePath();
			CString new_path = old_path.Left(old_path.GetLength() - mFileStream->GetFileName().GetLength()) + mGetRealName.win_str();
			mFileStream->Rename(old_path, new_path);
		}
#endif
#elif __dest_os == __linux_os

		// Now change file name if required
#if 0	// Always use user's choosen name regardless of embedded name
		if (mFileStream->GetFileName() != mGetRealName)
		{
			string old_path = mFileStream->GetFilePath().c_str();
			string new_path = old_path.substr(0, old_path.length() - mFileStream->GetFileName().length()) + mGetRealName.c_str(); 
			JRenameFile(old_path.c_str(), new_path.c_str());
		}
#endif
#else
#error __dest_os
#endif

		mStatus2 = eEnd2;
	}

	inByteCount = total;
	return noErr;
}

// Fill header structure from file
void CAFFilter::CreateHeader()
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	OSErr err = noErr;
	unsigned long offset = 0;
	CInfoPBRec cpb;

	// Get input file details
	PPx::FSObject fspec;
	mFileStream->GetSpecifier(fspec);
	FSSpec spec;
	fspec.GetFSSpec(spec);

	// Get file info
	cpb.hFileInfo.ioCompletion = nil;
	cpb.hFileInfo.ioNamePtr = spec.name;
	cpb.hFileInfo.ioVRefNum = spec.vRefNum;
	cpb.hFileInfo.ioFDirIndex = 0;
	cpb.hFileInfo.ioDirID = spec.parID;
	ThrowIfOSErr_(::PBGetCatInfoSync(&cpb));

	// Get Desktop database refnum
	DTPBRec dtpb;
	short DTRefNum;

	dtpb.ioNamePtr = nil;
	dtpb.ioVRefNum = spec.vRefNum;
	err = ::PBDTGetPath(&dtpb);
	if (err)
		DTRefNum = 0;
	else
		DTRefNum = dtpb.ioDTRefNum;

	// Header
	mIndex.header.magicNum = htonl(mSingle ? cAppleFile_AS : cAppleFile_AD);
	mIndex.header.versionNum = htonl(cAppleFile_Version2);
	for(short i = 0; i < 16; i++)
		mIndex.header.filler[i] = 0;
	mIndex.header.numEntries = htonl(mSingle ? cAppleFile_ASEntries : cAppleFile_ADEntries);
	offset = mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize;

	// Real name
	mIndex.entries[cAppleFile_RealNameIndex].entryID = htonl(cAppleFile_ASRealName);
	mIndex.entries[cAppleFile_RealNameIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_RealNameIndex].entryLen = htonl(spec.name[0]);
	offset += ntohl(mIndex.entries[0].entryLen);

	// Finder info
	mIndex.entries[cAppleFile_FinderInfoIndex].entryID = htonl(cAppleFile_ASFinderInfo);
	mIndex.entries[cAppleFile_FinderInfoIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_FinderInfoIndex].entryLen = htonl(sizeof(FInfo) + sizeof(FXInfo));
	offset += ntohl(mIndex.entries[1].entryLen);

	// File dates
	mIndex.entries[cAppleFile_FileDatesIndex].entryID = htonl(cAppleFile_ASFileDates);
	mIndex.entries[cAppleFile_FileDatesIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_FileDatesIndex].entryLen = htonl(16);
	offset += ntohl(mIndex.entries[2].entryLen);

	// Get comment
	Str255 comment;
	if (DTRefNum)
	{
		dtpb.ioCompletion = false;
		dtpb.ioNamePtr = spec.name;
		dtpb.ioDTBuffer = (char*) &comment[1];
		dtpb.ioDirID = spec.parID;
		if (::PBDTGetCommentSync(&dtpb))
			*comment = 0;
		else
			*comment = dtpb.ioDTActCount;
	}
	else
		*comment = 0;

	// Set comment
	mIndex.entries[cAppleFile_CommentIndex].entryID = htonl(cAppleFile_ASComment);
	mIndex.entries[cAppleFile_CommentIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_CommentIndex].entryLen = htonl(*comment);
	offset += ntohl(mIndex.entries[3].entryLen);

	mHeaderBufferLen = offset;

	// Resource fork
	mIndex.entries[cAppleFile_RsrcForkIndex].entryID = htonl(cAppleFile_ASResource);
	mIndex.entries[cAppleFile_RsrcForkIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_RsrcForkIndex].entryLen = htonl(cpb.hFileInfo.ioFlRLgLen);
	offset += ntohl(mIndex.entries[4].entryLen);

	// Only add data for AppleSingle
	if (mSingle)
	{
		// Data fork
		mIndex.entries[cAppleFile_DataForkIndex].entryID = htonl(cAppleFile_ASData);
		mIndex.entries[cAppleFile_DataForkIndex].entryOffset = htonl(offset);
		mIndex.entries[cAppleFile_DataForkIndex].entryLen = htonl(cpb.hFileInfo.ioFlLgLen);
		offset += ntohl(mIndex.entries[5].entryLen);
	}

	// Total number of encoded bytes to be sent
	mLength = offset;

	// Create and fill in header details
	mHeaderBuffer = new char[mHeaderBufferLen + 1];
	char* p = mHeaderBuffer;

	// Add header
	::memcpy(p, &mIndex.header, (mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize));
	p += mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize;

	// Add real name
	::memcpy(p, &spec.name[1], spec.name[0]);
	p += spec.name[0];

	// Add FInfo
	::memcpy(p, &cpb.hFileInfo.ioFlFndrInfo, sizeof(FInfo));
	p += sizeof(FInfo);
	::memcpy(p, &cpb.hFileInfo.ioFlXFndrInfo, sizeof(FXInfo));
	p += sizeof(FXInfo);

	// Add dates
	*((long*) p) = cpb.hFileInfo.ioFlCrDat - cAppleFile_MacToASDate;
	p += sizeof(long);
	*((long*) p) = cpb.hFileInfo.ioFlMdDat - cAppleFile_MacToASDate;
	p += sizeof(long);
	*((long*) p) = cpb.hFileInfo.ioFlBkDat - cAppleFile_MacToASDate;
	p += sizeof(long);
	*((long*) p) = cAppleFile_DateUnknown;
	p += sizeof(long);

	// Addcomment
	::memcpy(p, &comment[1], *comment);
	p += *comment;
#elif __dest_os == __win32_os

	unsigned long offset = 0;

	// Header
	mIndex.header.magicNum = htonl(mSingle ? cAppleFile_AS : cAppleFile_AD);
	mIndex.header.versionNum = htonl(cAppleFile_Version2);
	for(short i = 0; i < 16; i++)
		mIndex.header.filler[i] = 0;
	mIndex.header.numEntries = htons(mSingle ? cAppleFile_ASEntries : cAppleFile_ADEntries);
	offset = mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize;

	// Real name
	mIndex.entries[cAppleFile_RealNameIndex].entryID = htonl(cAppleFile_ASRealName);
	mIndex.entries[cAppleFile_RealNameIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_RealNameIndex].entryLen = htonl(mFileStream->GetFileName().GetLength());
	offset += ntohl(mIndex.entries[0].entryLen);

	// Finder info
	mIndex.entries[cAppleFile_FinderInfoIndex].entryID = htonl(cAppleFile_ASFinderInfo);
	mIndex.entries[cAppleFile_FinderInfoIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_FinderInfoIndex].entryLen = htonl(sizeof(FInfo) + sizeof(FXInfo));
	offset += ntohl(mIndex.entries[1].entryLen);

	// File dates
	mIndex.entries[cAppleFile_FileDatesIndex].entryID = htonl(cAppleFile_ASFileDates);
	mIndex.entries[cAppleFile_FileDatesIndex].entryOffset =htonl(offset);
	mIndex.entries[cAppleFile_FileDatesIndex].entryLen = htonl(16);
	offset += ntohl(mIndex.entries[2].entryLen);

	// Get comment
	Str255 comment;
	*comment = 0;

	// Set comment
	mIndex.entries[cAppleFile_CommentIndex].entryID = htonl(cAppleFile_ASComment);
	mIndex.entries[cAppleFile_CommentIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_CommentIndex].entryLen = htonl(*comment);
	offset += ntohl(mIndex.entries[3].entryLen);

	mHeaderBufferLen = offset;

	// Resource fork
	mIndex.entries[cAppleFile_RsrcForkIndex].entryID = htonl(cAppleFile_ASResource);
	mIndex.entries[cAppleFile_RsrcForkIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_RsrcForkIndex].entryLen = 0;
	offset += ntohl(mIndex.entries[4].entryLen);

	// Only add data for AppleSingle
	if (mSingle)
	{
		// Data fork
		mIndex.entries[cAppleFile_DataForkIndex].entryID = htonl(cAppleFile_ASData);
		mIndex.entries[cAppleFile_DataForkIndex].entryOffset = htonl(offset);
		mIndex.entries[cAppleFile_DataForkIndex].entryLen = htonl(mFileStream->GetLength());
		offset += ntohl(mIndex.entries[5].entryLen);
	}

	// Total number of encoded bytes to be sent
	mLength = offset;

	// Create and fill in header details
	mHeaderBuffer = new char[mHeaderBufferLen + 1];
	char* p = mHeaderBuffer;

	// Add header
	::memcpy(p, &mIndex.header, (mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize));
	p += mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize;

	// Add real name
	::memcpy(p, mFileStream->GetFileName(), mFileStream->GetFileName().GetLength());
	p += mFileStream->GetFileName().GetLength();

	// Add FInfo
	FInfo fake_finfo;
	fake_finfo.fdType = htonl('TEXT');
	fake_finfo.fdCreator = htonl('ttxt');
	fake_finfo.fdFlags = 0;
	fake_finfo.fdLocation = 0;
	fake_finfo.fdFldr = 0;
	::memcpy(p, &fake_finfo, sizeof(FInfo));
	p += sizeof(FInfo);
	FXInfo fake_fxinfo;
	::memset(&fake_fxinfo, 0, sizeof(FXInfo));
	::memcpy(p, &fake_fxinfo, sizeof(FXInfo));
	p += sizeof(FXInfo);

	// Add dates
	CFileStatus file_status;
	mFileStream->GetStatus(file_status);
	CTime reference(2000, 1, 1, 0, 0, 0);	// AppleSingle/AppleDouble reference time
	*((long*) p) = htonl((file_status.m_ctime - reference).GetTotalSeconds());
	p += sizeof(long*);
	*((long*) p) = htonl((file_status.m_mtime - reference).GetTotalSeconds());
	p += sizeof(long*);
	*((long*) p) = htonl(cAppleFile_DateUnknown);
	p += sizeof(long*);
	*((long*) p) = htonl(cAppleFile_DateUnknown);
	p += sizeof(long*);

	// Add comment (none for Windows)

#elif __dest_os == __linux_os

	unsigned long offset = 0;

	// Header
	mIndex.header.magicNum = htonl(mSingle ? cAppleFile_AS : cAppleFile_AD);
	mIndex.header.versionNum = htonl(cAppleFile_Version2);
	for(short i = 0; i < 16; i++)
		mIndex.header.filler[i] = 0;
	mIndex.header.numEntries = htons(mSingle ? cAppleFile_ASEntries : cAppleFile_ADEntries);
	offset = mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize;

	// Real name
	mIndex.entries[cAppleFile_RealNameIndex].entryID = htonl(cAppleFile_ASRealName);
	mIndex.entries[cAppleFile_RealNameIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_RealNameIndex].entryLen = htonl(mFileStream->GetFileName().length());
	offset += ntohl(mIndex.entries[0].entryLen);

	// Finder info
	mIndex.entries[cAppleFile_FinderInfoIndex].entryID = htonl(cAppleFile_ASFinderInfo);
	mIndex.entries[cAppleFile_FinderInfoIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_FinderInfoIndex].entryLen = htonl(sizeof(FInfo) + sizeof(FXInfo));
	offset += ntohl(mIndex.entries[1].entryLen);

	// File dates
	mIndex.entries[cAppleFile_FileDatesIndex].entryID = htonl(cAppleFile_ASFileDates);
	mIndex.entries[cAppleFile_FileDatesIndex].entryOffset =htonl(offset);
	mIndex.entries[cAppleFile_FileDatesIndex].entryLen = htonl(16);
	offset += ntohl(mIndex.entries[2].entryLen);

	// Get comment
	char comment[256];
	*comment = 0;

	// Set comment
	mIndex.entries[cAppleFile_CommentIndex].entryID = htonl(cAppleFile_ASComment);
	mIndex.entries[cAppleFile_CommentIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_CommentIndex].entryLen = htonl(*comment);
	offset += ntohl(mIndex.entries[3].entryLen);

	mHeaderBufferLen = offset;

	// Resource fork
	mIndex.entries[cAppleFile_RsrcForkIndex].entryID = htonl(cAppleFile_ASResource);
	mIndex.entries[cAppleFile_RsrcForkIndex].entryOffset = htonl(offset);
	mIndex.entries[cAppleFile_RsrcForkIndex].entryLen = 0;
	offset += ntohl(mIndex.entries[4].entryLen);

	// Only add data for AppleSingle
	if (mSingle)
	{
		// Data fork
		mIndex.entries[cAppleFile_DataForkIndex].entryID = htonl(cAppleFile_ASData);
		mIndex.entries[cAppleFile_DataForkIndex].entryOffset = htonl(offset);
		mIndex.entries[cAppleFile_DataForkIndex].entryLen = htonl(mFileStream->GetLength());
		offset += ntohl(mIndex.entries[5].entryLen);
	}

	// Total number of encoded bytes to be sent
	mLength = offset;

	// Create and fill in header details
	mHeaderBuffer = new char[mHeaderBufferLen + 1];
	char* p = mHeaderBuffer;

	// Add header
	::memcpy(p, &mIndex.header, (mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize));
	p += mSingle ? cAppleFile_ASIndexSize : cAppleFile_ADIndexSize;

	// Add real name
	::memcpy(p, mFileStream->GetFileName(), mFileStream->GetFileName().length());
	p += mFileStream->GetFileName().length();

	// Add FInfo
	FInfo fake_finfo;
	fake_finfo.fdType = htonl('TEXT');
	fake_finfo.fdCreator = htonl('ttxt');
	fake_finfo.fdFlags = 0;
	fake_finfo.fdLocation = 0;
	fake_finfo.fdFldr = 0;
	::memcpy(p, &fake_finfo, sizeof(FInfo));
	p += sizeof(FInfo);
	FXInfo fake_fxinfo;
	::memset(&fake_fxinfo, 0, sizeof(FXInfo));
	::memcpy(p, &fake_fxinfo, sizeof(FXInfo));
	p += sizeof(FXInfo);

	// Add dates
	struct stat file_status;
	mFileStream->GetStatus(file_status);
	//0,0,0, 1, Jan, 2000, dummy, dummy, No DST
	tm appleRef = {0, 0, 0, 1, 0, 100, 0,0,0};
	time_t reference = mktime(&appleRef);
	*((long*) p) = htonl(file_status.st_ctime - reference);
	p += sizeof(long);
	*((long*) p) = htonl(file_status.st_mtime - reference);
	p += sizeof(long);
	*((long*) p) = htonl(cAppleFile_DateUnknown);
	p += sizeof(long);
	*((long*) p) = htonl(cAppleFile_DateUnknown);
	p += sizeof(long);

	// Add comment (none for Windows)

#else
#error __dest_os
#endif
}
