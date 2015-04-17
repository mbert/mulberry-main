// Copyright ©2005, 2006 Freescale Semiconductor, Inc.
// Please see the License for the specific language governing rights and
// limitations under the License.
// ===========================================================================
//	LFile.cp					PowerPlant 2.2.2	©1993-2005 Metrowerks Inc.
// ===========================================================================
//
//	A wrapper class for a Macintosh file with a data and a resource fork
//
//	A FSSpec (File System Specification) record identifies a Mac file.
//	When open, the data fork has a file refNum. Likewise, when open,
//	the resource fork has a file refNum. The LFile class stores an
//	FSSpec and the refNums for the data and resource forks.
//
//	This class does not provide many functions for manipulating files. You
//	should get the FSSpec or refNum of the fork you want to manipulate
//	and make direct calls to the Mac File Manager. However, use the
//	member functions for opening and closing data and resource forks.
//
//	The only file accessing functions provided are ones for reading and
//	writing the entire data fork.

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include <LFile.h>
#include <UMemoryMgr.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ LFile									Default Constructor		  [public]
// ---------------------------------------------------------------------------

LFile::LFile()
{
	mDataForkRefNum			= refNum_Undefined;
	mResourceForkRefNum		= refNum_Undefined;
}


// ---------------------------------------------------------------------------
//	¥ LFile									Constructor				  [public]
// ---------------------------------------------------------------------------
//	Construct a File from a Toolbox File System Specification

LFile::LFile(
	const PPx::FSObject&	inFileRef)
{
	mMacFileRef			= inFileRef;
	mDataForkRefNum		= refNum_Undefined;
	mResourceForkRefNum	= refNum_Undefined;
}


// ---------------------------------------------------------------------------
//	¥ LFile									Constructor				  [public]
// ---------------------------------------------------------------------------
//	Construct a File from an Alias
//
//	outWasChanged indicates if the AliasHandle was changed during resolution
//	inFromFile is a File Specifier for the starting point for a relative
//		search. If nil, an absolute search is performed

LFile::LFile(
	AliasHandle		inAlias,
	Boolean&		outWasChanged,
	PPx::FSObject*	inFromFile)
{
	FSRef newFile;
	OSErr	err = ::FSResolveAlias(inFromFile ? &inFromFile->UseRef() : nil, inAlias, &newFile,
								&outWasChanged);

	mDataForkRefNum		= refNum_Undefined;
	mResourceForkRefNum = refNum_Undefined;

	if (err == noErr)
		mMacFileRef = newFile;

	ThrowIfOSErr_(err);
}


// ---------------------------------------------------------------------------
//	¥ ~LFile								Destructor				  [public]
// ---------------------------------------------------------------------------

LFile::~LFile()
{
	try {								// Don't throw out of a destructor
		CloseDataFork();
	}

	catch (...) { }

	try {
		CloseResourceFork();
	}

	catch (...) { }
}


// ---------------------------------------------------------------------------
//	¥ GetSpecifier
// ---------------------------------------------------------------------------
//	Return the Toolbox File System Specification for a File

void
LFile::GetSpecifier(
	PPx::FSObject&		outFileRef) const
{
	outFileRef = mMacFileRef;		// Copy FSRef struct
}


// ---------------------------------------------------------------------------
//	¥ SetSpecifier
// ---------------------------------------------------------------------------
//	Set a new Toolbox File System Specification for a File
//
//	This has the side effect of closing any open forks of the file identified
//	by the old Specifier

void
LFile::SetSpecifier(
	const PPx::FSObject&	inFileRef)
{
	CloseDataFork();
	CloseResourceFork();

	mMacFileRef = inFileRef;
}


// ---------------------------------------------------------------------------
//	¥ UsesSpecifier
// ---------------------------------------------------------------------------
//	Returns whether the File's FSRef is the same as the input FSRef

bool
LFile::UsesSpecifier(
	const PPx::FSObject&	inFileRef) const
{
	return EqualFileSpec(mMacFileRef, inFileRef);
}


// ---------------------------------------------------------------------------
//	¥ SpecifierExists
// ---------------------------------------------------------------------------
//	Returns whether the File's FSRef corresponds to an existing disk file

bool
LFile::SpecifierExists() const
{
	return mMacFileRef.Exists();
}


// ---------------------------------------------------------------------------
//	¥ MakeAlias
// ---------------------------------------------------------------------------
//	Return a newly created Alias for a File
//
//	inFromFile is a File Specifier for the starting point for a relative
//		search. Pass nil if you don't need relative path information.

AliasHandle
LFile::MakeAlias(
	PPx::FSObject*		inFromFile)
{
	AliasHandle	theAlias;
	OSErr	err = ::FSNewAlias(&inFromFile->UseRef(), &mMacFileRef.UseRef(), &theAlias);

	return theAlias;
}


// ---------------------------------------------------------------------------
//	¥ CreateNewFile
// ---------------------------------------------------------------------------
//	Create a new disk File, with an empty data fork and a resoure map.
//	You must call OpenDataFork or OpenResourceFork (with write permission)
//	before you can store information in the File.
//
//	If the file already exists, but doesn't have a resource map, this
//	function will create a resource map.

void
LFile::CreateNewFile(
	OSType		inCreator,
	OSType		inFileType,
	ScriptCode	/* inScriptCode */)
{
	// Don't bother if already exists
	if (mMacFileRef.Exists())
		return;

	FSCatalogInfo	catInfo;
	FInfo*			finderInfo = (FInfo *) &catInfo.finderInfo;
	
	::BlockZero(finderInfo, sizeof(FInfo));
	
	finderInfo->fdType		= inFileType;
	finderInfo->fdCreator	= inCreator;

	FSRef parentRef;
	mMacFileRef.GetParent(parentRef);

	HFSUniStr255 fileName;
	mMacFileRef.GetName(fileName);

	FSRef fileRef;
	OSStatus err = ::FSCreateFileUnicode(&parentRef, fileName.length,
											fileName.unicode, kFSCatInfoFinderInfo,
											&catInfo, &fileRef, NULL);
	ThrowIfOSErr_(err);
	
	mMacFileRef = fileRef;
}


// ---------------------------------------------------------------------------
//	¥ CreateNewDataFile
// ---------------------------------------------------------------------------
//	Create a new disk File, with an empty data fork and no resource map.
//	You must call OpenDataFork (with write permission) before you can store
//	data in the File.
//
//	The resource fork is uninitialized (no resource map), so you can't call
//	OpenResourceFork for the File. You can initialize the resource fork
//	by calling CreateNewFile.

void
LFile::CreateNewDataFile(
	OSType		inCreator,
	OSType		inFileType,
	ScriptCode	inScriptCode)
{
	// Just create the file
	CreateNewFile(inCreator, inFileType, inScriptCode);
}


// ---------------------------------------------------------------------------
//	¥ OpenDataFork
// ---------------------------------------------------------------------------
//	Open the data fork of a File with the specified permissions and
//	return the reference number for the opened fork
//
//	A data fork must be Open before you can read or write data

SInt16
LFile::OpenDataFork(
	SInt16	inPrivileges)
{
	HFSUniStr255 forkName = { 0 };
	::FSGetDataForkName(&forkName);

	OSStatus err = ::FSCreateFork(&mMacFileRef.UseRef(), forkName.length, forkName.unicode);
	if ((err != noErr) && (err != errFSForkExists)) {
		ThrowIfOSErr_(err);
	}

	SInt16 forkRefNum;
	err = ::FSOpenFork(&mMacFileRef.UseRef(), forkName.length, forkName.unicode,
						inPrivileges, &forkRefNum);
	ThrowIfOSErr_(err);

	mDataForkRefNum = forkRefNum;

	err = ::FSSetForkPosition(mDataForkRefNum, fsFromStart, 0);

	return mDataForkRefNum;
}


// ---------------------------------------------------------------------------
//	¥ CloseDataFork
// ---------------------------------------------------------------------------
//	Close the data fork of a File

void
LFile::CloseDataFork()
{
	if (mDataForkRefNum != refNum_Undefined) {
		OSErr	err = ::FSCloseFork(mDataForkRefNum);
		mDataForkRefNum = refNum_Undefined;
		ThrowIfOSErr_(err);

		::FlushVol(nil, mMacFileRef.GetVolume());
	}
}


// ---------------------------------------------------------------------------
//	¥ ReadDataFork
// ---------------------------------------------------------------------------
//	Read the entire contents of a File's data fork into a newly created
//	Handle. The caller is responsible for disposing of the Handle.

Handle
LFile::ReadDataFork()
{
	SInt64		forkSize;
	OSStatus	err = ::FSGetForkSize(mDataForkRefNum, &forkSize);
	ThrowIfOSErr_(err);

	SInt32 		fileLength = forkSize;
	StHandleBlock	dataHandle(fileLength);

	ByteCount bytesRead;
	err = ::FSReadFork(mDataForkRefNum, fsFromStart, 0,
								fileLength, *dataHandle, &bytesRead);
	ThrowIfOSErr_(err);

	return dataHandle.Release();
}


// ---------------------------------------------------------------------------
//	¥ WriteDataFork
// ---------------------------------------------------------------------------
//	Write to the data fork of a File from a buffer
//
//	The buffer contents completely replace any existing data

SInt32
LFile::WriteDataFork(
	const void*		inBuffer,
	SInt32			inByteCount)
{
	ByteCount bytesWritten = inByteCount;
	OSStatus err = ::FSWriteFork(mDataForkRefNum, fsFromStart, 0,
									inByteCount, inBuffer, &bytesWritten);
	ThrowIfOSErr_(err);

	return bytesWritten;
}


// ---------------------------------------------------------------------------
//	¥ OpenResourceFork
// ---------------------------------------------------------------------------
//	Open the resource fork of a File with the specified permissions and
//	return the reference number for the opened fork
//
//	A resource fork must be Open before you can read or write resources

SInt16
LFile::OpenResourceFork(
	SInt16	inPrivileges)
{
	HFSUniStr255 forkName = { 0 };
	::FSGetResourceForkName(&forkName);

	OSStatus err = ::FSCreateResourceFork(&mMacFileRef.UseRef(), forkName.length, forkName.unicode, 0);
	if ((err != noErr) && (err != errFSForkExists)) {
		ThrowIfOSErr_(err);
	}

	mResourceForkRefNum = ::FSOpenResFile(&mMacFileRef.UseRef(), (SInt8) inPrivileges);

	if (mResourceForkRefNum == -1) {	// Open failed
		ThrowIfResError_();
	}

	err = ::FSSetForkPosition(mResourceForkRefNum, fsFromStart, 0);

	return mResourceForkRefNum;
}


// ---------------------------------------------------------------------------
//	¥ CloseResourceFork
// ---------------------------------------------------------------------------
//	Close the resource fork of a File

void
LFile::CloseResourceFork()
{
	if (mResourceForkRefNum != refNum_Undefined) {
		::CloseResFile(mResourceForkRefNum);
		mResourceForkRefNum = refNum_Undefined;
		::FlushVol(nil, mMacFileRef.GetVolume());
	}
}


// ---------------------------------------------------------------------------
//	¥ EqualFileSpec													  [static]
// ---------------------------------------------------------------------------
//	Compare two FSRef structs for equality

bool
LFile::EqualFileSpec(
	const PPx::FSObject&	inFileSpecA,
	const PPx::FSObject&	inFileSpecB)
{
		// Compare each field in FSRef struct
		//   EqualString() [case insensitive, diacritical sensitive] is
		//   the same comparison used by the File System

	return inFileSpecA.IsEqualTo(inFileSpecB);
}


PP_End_Namespace_PowerPlant
