// Copyright ©2005, 2006 Freescale Semiconductor, Inc.
// Please see the License for the specific language governing rights and
// limitations under the License.
// ===========================================================================
//	UNavServicesDialogs.cp		PowerPlant 2.2.2	©1998-2005 Metrowerks Inc.
// ===========================================================================
//
//	Classes which use Navigation Services dialogs to prompt the user to
//	open and save files

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include <UNavServicesDialogs.h>

#include <LFileTypeList.h>
#include <LString.h>
#include <PP_Resources.h>
#include <UDesktop.h>
#include <UExtractFromAEDesc.h>
#include <UMemoryMgr.h>
#include <UModalDialogs.h>

#include <Script.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ StNavReplyRecord						Default Constructor		  [public]
// ---------------------------------------------------------------------------

UNavServicesDialogs::StNavReplyRecord::StNavReplyRecord()
{
	mNavDialog = NULL;
	mNavReply.validRecord = false;
	mNavResult = kNavUserActionNone;

	SetDefaultValues();
}


// ---------------------------------------------------------------------------
//	¥ ~StNavReplyRecord						Destructor				  [public]
// ---------------------------------------------------------------------------

UNavServicesDialogs::StNavReplyRecord::~StNavReplyRecord()
{
	if (mNavReply.validRecord) {
		::NavDisposeReply(&mNavReply);
	}
}


// ---------------------------------------------------------------------------
//	¥ SetDefaultValues												  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::StNavReplyRecord::SetDefaultValues()
{
									// Clean up record if it is valid
	if (mNavReply.validRecord) {
		::NavDisposeReply(&mNavReply);
	}

		// The Nav Services documentation states that the other fields
		// are invalid if validRecord is false. However, we put sensible
		// defaults in each field so that we can access them without
		// having to throw an error if validRecord is false.

	mNavReply.validRecord				= false;
	mNavReply.replacing					= false;
	mNavReply.isStationery				= false;
	mNavReply.translationNeeded			= false;
	mNavReply.selection.descriptorType = typeNull;
	mNavReply.selection.dataHandle		= nil;
	mNavReply.keyScript					= smSystemScript;
	mNavReply.fileTranslation			= nil;
}


// ---------------------------------------------------------------------------
//	¥ GetFileSpec													  [public]
// ---------------------------------------------------------------------------
//	Pass back the FSSpec for the underlying file. Accesses the first file
//	if there is more than one file.

void
UNavServicesDialogs::StNavReplyRecord::GetFileSpec(
	PPx::FSObject&	outFileSpec) const
{
	// Get parent FSRef
	FSRef parent;
	UExtractFromAEDesc::TheFSRef(Selection(), parent);
	
	// Create parent+name spec
	if (mNavReply.saveFileName != NULL)
		outFileSpec = PPx::FSObject(parent, mNavReply.saveFileName);
	else
		outFileSpec = parent;
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ Load															  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::Load()
{
	::NavLoad();
}


// ---------------------------------------------------------------------------
//	¥ Unload														  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::Unload()
{
	::NavUnload();
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ AskSaveChanges												  [public]
// ---------------------------------------------------------------------------

SInt16
UNavServicesDialogs::AskSaveChanges(
	CFStringRef		inDocumentName,
	CFStringRef		inAppName,
	bool			inQuitting)
{
	NavDialogCreationOptions	options;
	::NavGetDefaultDialogCreationOptions(&options);

	options.saveFileName = inDocumentName;
	options.clientName = inAppName;

	StNavEventUPP				eventUPP(NavEventProc);
	StNavReplyRecord			reply;

	NavAskSaveChangesAction		action = kNavSaveChangesClosingDocument;
	if (inQuitting) {
		action = kNavSaveChangesQuittingApplication;
	}
	options.modality = kWindowModalityAppModal;
	options.parentWindow = NULL;

	UDesktop::Deactivate();

	OSErr	err = ::NavCreateAskSaveChangesDialog(&options, action, eventUPP, &reply, &reply.GetDialog());
	err = ::NavDialogRun(reply.GetDialog());
	if (err == noErr)
		reply.Result() = ::NavDialogGetUserAction( reply.GetDialog() );
	::NavDialogDispose(reply.GetDialog());

	UDesktop::Activate();

	ThrowIfOSErr_(err);

	switch(reply.Result())
	{
	case kNavUserActionSaveChanges:
		return answer_Save;
	case kNavUserActionDontSaveChanges:
		return answer_DontSave;
	case kNavUserActionCancel:
	default:
		return answer_Cancel;
	}
}


// ---------------------------------------------------------------------------
//	¥ AskConfirmRevert												  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::AskConfirmRevert(
	CFStringRef		inDocumentName)
{
	NavDialogCreationOptions	options;
	::NavGetDefaultDialogCreationOptions(&options);

	options.saveFileName = inDocumentName;

	StNavEventUPP				eventUPP(NavEventProc);
	StNavReplyRecord			reply;

	UDesktop::Deactivate();

	OSErr	err = ::NavCreateAskDiscardChangesDialog(&options, eventUPP, &reply, &reply.GetDialog());
	err = ::NavDialogRun(reply.GetDialog());
	if (err == noErr)
		reply.Result() = ::NavDialogGetUserAction( reply.GetDialog() );
	::NavDialogDispose(reply.GetDialog());

	UDesktop::Activate();

	ThrowIfOSErr_(err);

	return (reply.Result() == kNavAskDiscardChanges);
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ AskOpenOneFile												  [public]
// ---------------------------------------------------------------------------
//	Simple wrapper for NavGetFile that lets a user select one file using
//	the default options

bool
UNavServicesDialogs::AskOpenOneFile(
	OSType					inFileType,
	PPx::FSObject&			outFileSpec,
	NavDialogOptionFlags	inFlags)
{
	LFileTypeList	fileTypes(inFileType);
	LFileChooser	chooser;

	inFlags &= ~kNavAllowMultipleFiles;		// Can't select multiple files
	NavDialogCreationOptions*	options = chooser.GetDialogOptions();
	options->optionFlags = inFlags;

	bool	askOK = chooser.AskOpenFile(fileTypes);

	if (askOK) {
		chooser.GetFileSpec(1, outFileSpec);
	}

	return askOK;
}


// ---------------------------------------------------------------------------
//	¥ AskChooseOneFile												  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::AskChooseOneFile(
	OSType					inFileType,
	PPx::FSObject&			outFileSpec,
	NavDialogOptionFlags	inFlags)
{
	if (inFileType)
{
	LFileTypeList	fileTypes(inFileType);
	LFileChooser	chooser;

		NavDialogCreationOptions*	options = chooser.GetDialogOptions();
		options->optionFlags = inFlags;

		return chooser.AskChooseOneFile(fileTypes, outFileSpec);
	}
	else
	{
		LFileTypeList	fileTypes(fileTypes_All);
		LFileChooser	chooser;

		NavDialogCreationOptions*	options = chooser.GetDialogOptions();
		options->optionFlags = inFlags;

	return chooser.AskChooseOneFile(fileTypes, outFileSpec);
	}
}


// ---------------------------------------------------------------------------
//	¥ AskChooseFolder												  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::AskChooseFolder(
	PPx::FSObject&			outFileSpec,
	SInt32&					outFolderDirID)
{
	LFileChooser	chooser;

	return chooser.AskChooseFolder(outFileSpec, outFolderDirID);
}


// ---------------------------------------------------------------------------
//	¥ AskChooseVolume												  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::AskChooseVolume(
	PPx::FSObject&			outFileSpec)
{
	LFileChooser	chooser;

	return chooser.AskChooseVolume(outFileSpec);
}

// ---------------------------------------------------------------------------
//	¥ AskSaveFile													  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::AskSaveFile(
	CFStringRef				inDefaultName,
	OSType					inFileType,
	PPx::FSObject&			outFileSpec,
	bool&					outReplacing,
	NavDialogOptionFlags	inFlags)
{
	LFileDesignator		designator;

	designator.SetFileType(inFileType);

	if (not (inFlags & kNavAllowStationery)) {
									// Turn off type popup. The popup
									//   contains file translation and
									//   stationery options. By design
									//   this function doesn't support
									//   file translation.
		inFlags |= kNavNoTypePopup;
	}

	NavDialogCreationOptions*	options = designator.GetDialogOptions();
	options->optionFlags = inFlags;

	bool	askOK = designator.AskDesignateFile(inDefaultName);

	if (askOK) {
		designator.GetFileSpec(outFileSpec);
		outReplacing = designator.IsReplacing();
	}

	return askOK;
}

#pragma mark -

// ===========================================================================
//	LFileChooser Class
// ===========================================================================
//	Uses Navigation Services to ask user to open a file

// ---------------------------------------------------------------------------
//	¥ LFileChooser::LFileChooser			Constructor				  [public]
// ---------------------------------------------------------------------------

UNavServicesDialogs::LFileChooser::LFileChooser()
{
	::NavGetDefaultDialogCreationOptions(&mNavCreateOptions);

	Str255 wTitle;
	::GetIndString( wTitle,
					STRx_Standards, str_SaveDialogTitle);
	if (*wTitle)
	{
		mNavCreateOptions.windowTitle = ::CFStringCreateWithPascalString(kCFAllocatorDefault, wTitle, ::CFStringGetSystemEncoding());
	}
	else
		mNavCreateOptions.windowTitle = NULL;

	mNavFilterProc	= nil;
	mNavPreviewProc	= nil;
	mSelectDefault	= false;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::~LFileChooser			Destructor				  [public]
// ---------------------------------------------------------------------------

UNavServicesDialogs::LFileChooser::~LFileChooser()
{
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::GetDialogOptions								  [public]
// ---------------------------------------------------------------------------

NavDialogCreationOptions*
UNavServicesDialogs::LFileChooser::GetDialogOptions()
{
	return &mNavCreateOptions;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::SetDefaultLocation								  [public]
// ---------------------------------------------------------------------------
//	Specify the item that is selected when the "choose" dialog is displayed
//
//	If the inFileSpec refers to a directory, the inSelectIt parameter
//	specifies whether to select the directory itself (true), or to select
//	the first item within the directory (false).

void
UNavServicesDialogs::LFileChooser::SetDefaultLocation(
	const PPx::FSObject&	inFileSpec,
	bool					inSelectIt)
{
	mDefaultLocation = inFileSpec.UseRef();
	mSelectDefault	 = inSelectIt;
}

// ---------------------------------------------------------------------------
//	¥ LFileChooser::SetObjectFilterProc								  [public]
// ---------------------------------------------------------------------------
//	Specify the object filter callback function
//
//	See the comments for UClassicDialogs::LFileChooser::SetObjectFilterProc()
//	if you wish to use the same callback function for both NavServices
//	and StandardFile.

void
UNavServicesDialogs::LFileChooser::SetObjectFilterProc(
	NavObjectFilterProcPtr	inFilterProc)
{
	mNavFilterProc = inFilterProc;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::SetPreviewProc									  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileChooser::SetPreviewProc(
	NavPreviewProcPtr	inPreviewProc)
{
	mNavPreviewProc = inPreviewProc;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::AskOpenFile										  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileChooser::AskOpenFile(
	const LFileTypeList&	inFileTypes)
{
									// Create UPPs for callback functions
	StNavEventUPP			eventUPP(NavEventProc);
	StNavObjectFilterUPP	objectFilterUPP(mNavFilterProc);
	StNavPreviewUPP			previewUPP(mNavPreviewProc);

	mNavReply.SetDefaultValues();	// Initialize Reply record

									// Set default location, the location
									//   that's displayed when the dialog
									//   first appears
	AEDesc*		defaultLocationDesc = nil;
	if (not mDefaultLocation.IsNull()) {
		defaultLocationDesc = mDefaultLocation;

		if (mSelectDefault) {
			mNavCreateOptions.optionFlags |= kNavSelectDefaultLocation;
		} else {
			mNavCreateOptions.optionFlags &= ~kNavSelectDefaultLocation;
		}
	}

	UDesktop::Deactivate();

	OSErr err = ::NavCreateGetFileDialog(
						&mNavCreateOptions,
						inFileTypes.TypeListHandle(),
						eventUPP,
						previewUPP,
						objectFilterUPP,
						this,
						&mNavReply.GetDialog());
	err = ::NavDialogRun(mNavReply.GetDialog());
	::NavDialogDispose(mNavReply.GetDialog());

	UDesktop::Activate();

	if ( (err != noErr) && (err != userCanceledErr) ) {
		Throw_(err);
	}

	return mNavReply.Result() == kNavUserActionOpen;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::AskChooseOneFile								  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileChooser::AskChooseOneFile(
	const LFileTypeList&	inFileTypes,
	PPx::FSObject&			outFileSpec)
{
									// Create UPPs for callback functions
	StNavEventUPP			eventUPP(NavEventProc);
	StNavObjectFilterUPP	objectFilterUPP(mNavFilterProc);
	StNavPreviewUPP			previewUPP(mNavPreviewProc);

	mNavReply.SetDefaultValues();

									// Can choose only one file
	mNavCreateOptions.optionFlags &= ~kNavAllowMultipleFiles;

									// Set default location, the location
									//   that's displayed when the dialog
									//   first appears
	AEDesc*		defaultLocationDesc = nil;
	if (not mDefaultLocation.IsNull()) {
		defaultLocationDesc = mDefaultLocation;

		if (mSelectDefault) {
			mNavCreateOptions.optionFlags |= kNavSelectDefaultLocation;
		} else {
			mNavCreateOptions.optionFlags &= ~kNavSelectDefaultLocation;
		}
	}

	UDesktop::Deactivate();

	OSErr err = ::NavCreateChooseFileDialog(
						&mNavCreateOptions,
						inFileTypes.TypeListHandle(),
						eventUPP,
						previewUPP,
						objectFilterUPP,
						this,
						&mNavReply.GetDialog());
	err = ::NavDialogRun(mNavReply.GetDialog());
	::NavDialogDispose(mNavReply.GetDialog());

	UDesktop::Activate();

	if ( (err != noErr) && (err != userCanceledErr) ) {
		Throw_(err);
	}

	if (mNavReply.Result() == kNavUserActionChoose) {
		mNavReply.GetFileSpec(outFileSpec);
	}

	return mNavReply.Result() == kNavUserActionChoose;
}

// ---------------------------------------------------------------------------
//	¥ LFileChooser::AskChooseFolder									  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileChooser::AskChooseFolder(
	PPx::FSObject&	outFileSpec,
	SInt32&			outFolderDirID)
{
									// Create UPPs for callback functions
	StNavEventUPP			eventUPP(NavEventProc);
	StNavObjectFilterUPP	objectFilterUPP(mNavFilterProc);

	mNavReply.SetDefaultValues();

									// Set default location, the location
									//   that's displayed when the dialog
									//   first appears
	AEDesc*		defaultLocationDesc = nil;
	if (not mDefaultLocation.IsNull()) {
		defaultLocationDesc = mDefaultLocation;

		if (mSelectDefault) {
			mNavCreateOptions.optionFlags |= kNavSelectDefaultLocation;
		} else {
			mNavCreateOptions.optionFlags &= ~kNavSelectDefaultLocation;
		}
	}

	UDesktop::Deactivate();

	OSErr err = ::NavCreateChooseFolderDialog(
						&mNavCreateOptions,
						eventUPP,
						objectFilterUPP,
						this,
						&mNavReply.GetDialog());
	err = ::NavDialogRun(mNavReply.GetDialog());
	::NavDialogDispose(mNavReply.GetDialog());

	UDesktop::Activate();

	if ( (err != noErr) && (err != userCanceledErr) ) {
		Throw_(err);
	}

	if (mNavReply.Result() == kNavUserActionChoose) {
		mNavReply.GetFileSpec(outFileSpec);
		outFolderDirID = 0;
	}

	return mNavReply.Result() == kNavUserActionChoose;
}

// ---------------------------------------------------------------------------
//	¥ LFileChooser::AskChooseVolume									  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileChooser::AskChooseVolume(
	PPx::FSObject&	outFileSpec)
{
									// Create UPPs for callback functions
	StNavEventUPP			eventUPP(NavEventProc);
	StNavObjectFilterUPP	objectFilterUPP(mNavFilterProc);

	mNavReply.SetDefaultValues();

	AEDesc*		defaultLocationDesc = nil;
	if (not mDefaultLocation.IsNull()) {
		defaultLocationDesc = mDefaultLocation;

			// mSelectDefault should always be true when selecting
			// volumes since we can't navigate into anything

		mNavCreateOptions.optionFlags |= kNavSelectDefaultLocation;
	}

	UDesktop::Deactivate();

	OSErr err = ::NavCreateChooseVolumeDialog(
						&mNavCreateOptions,
						eventUPP,
						objectFilterUPP,
						this,
						&mNavReply.GetDialog());
	err = ::NavDialogRun(mNavReply.GetDialog());
	::NavDialogDispose(mNavReply.GetDialog());

	UDesktop::Activate();

	if (mNavReply.Result() == kNavUserActionChoose) {
		mNavReply.GetFileSpec(outFileSpec);
	}

	return mNavReply.Result() == kNavUserActionChoose;
}

// ---------------------------------------------------------------------------
//	¥ LFileChooser::IsValid											  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileChooser::IsValid() const
{
	return mNavReply.IsValid();
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::GetNumberOfFiles								  [public]
// ---------------------------------------------------------------------------

SInt32
UNavServicesDialogs::LFileChooser::GetNumberOfFiles() const
{
	SInt32	numFiles = 0;

	if (mNavReply.IsValid()) {
		AEDescList	selectedItems = mNavReply.Selection();
		OSErr	err = ::AECountItems(&selectedItems, &numFiles);

		ThrowIfOSErr_(err);
	}

	return numFiles;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::GetFileSpec										  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileChooser::GetFileSpec(
	SInt32			inIndex,
	PPx::FSObject&	outFileSpec) const
{
	FSRef		temp;
	AEKeyword	theKey;
	DescType	theType;
	Size		theSize;

	AEDescList	selectedItems = mNavReply.Selection();
	OSErr err = ::AEGetNthPtr(&selectedItems, inIndex, typeFSRef,
						&theKey, &theType, (Ptr) &temp,
						sizeof(FSRef), &theSize);

	ThrowIfOSErr_(err);
	
	outFileSpec = temp;
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::GetFileDescList									  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileChooser::GetFileDescList(
	AEDescList&		outDescList) const
{
	outDescList = mNavReply.Selection();
}


// ---------------------------------------------------------------------------
//	¥ LFileChooser::GetScriptCode									  [public]
// ---------------------------------------------------------------------------

ScriptCode
UNavServicesDialogs::LFileChooser::GetScriptCode() const
{
	return mNavReply.KeyScript();
}

#pragma mark -

// ===========================================================================
//	LFileDesignator Class
// ===========================================================================
//	Uses Navigation Services to ask user to save a file

// ---------------------------------------------------------------------------
//	¥ LFileDesignator::LFileDesignator								  [public]
// ---------------------------------------------------------------------------

UNavServicesDialogs::LFileDesignator::LFileDesignator()
{
	::NavGetDefaultDialogCreationOptions(&mNavCreateOptions);

	Str255 wTitle;
	::GetIndString( wTitle,
					STRx_Standards, str_SaveDialogTitle);
	if (*wTitle)
	{
		mNavCreateOptions.windowTitle = ::CFStringCreateWithPascalString(kCFAllocatorDefault, wTitle, ::CFStringGetSystemEncoding());
	}
	else
		mNavCreateOptions.windowTitle = NULL;

	mFileType	 = FOUR_CHAR_CODE('????');
	mFileCreator = LFileTypeList::GetProcessSignature();
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::~LFileDesignator								  [public]
// ---------------------------------------------------------------------------

UNavServicesDialogs::LFileDesignator::~LFileDesignator()
{
	if (mNavCreateOptions.windowTitle != NULL)
		::CFRelease(mNavCreateOptions.windowTitle);
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::SetFileType									  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileDesignator::SetFileType(
	OSType		inFileType)
{
	mFileType = inFileType;
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::SetFileCreator								  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileDesignator::SetFileCreator(
	OSType		inFileCreator)
{
	mFileCreator = inFileCreator;
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::GetDialogOptions								  [public]
// ---------------------------------------------------------------------------

NavDialogCreationOptions*
UNavServicesDialogs::LFileDesignator::GetDialogOptions()
{
	return &mNavCreateOptions;
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::SetDefaultLocation							  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileDesignator::SetDefaultLocation(
	const PPx::FSObject&	inFileSpec,
	bool					inSelectIt)
{
	mDefaultLocation = inFileSpec.UseRef();
	mSelectDefault	 = inSelectIt;
}

// ---------------------------------------------------------------------------
//	¥ LFileDesignator::AskDesignateFile								  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileDesignator::AskDesignateFile(
	CFStringRef	inDefaultName)
{
	StNavEventUPP		eventUPP(NavEventProc);

	mNavCreateOptions.saveFileName = inDefaultName;

	mNavReply.SetDefaultValues();

	AEDesc*		defaultLocationDesc = nil;
	if (not mDefaultLocation.IsNull()) {
		defaultLocationDesc = mDefaultLocation;

		if (mSelectDefault) {
			mNavCreateOptions.optionFlags |= kNavSelectDefaultLocation;
		} else {
			mNavCreateOptions.optionFlags &= ~kNavSelectDefaultLocation;
		}
	}

	UDesktop::Deactivate();

	OSErr err = ::NavCreatePutFileDialog(
						&mNavCreateOptions,
						mFileType,
						mFileCreator,
						eventUPP,
						this,
						&mNavReply.GetDialog());					// User Data
	err = ::NavDialogRun(mNavReply.GetDialog());
	::NavDialogDispose(mNavReply.GetDialog());

	UDesktop::Activate();

	if ( (err != noErr) && (err != userCanceledErr) ) {
		Throw_(err);
	}

	return mNavReply.Result() == kNavUserActionSaveAs;
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::IsValid										  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileDesignator::IsValid() const
{
	return mNavReply.IsValid();
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::GetFileSpec									  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileDesignator::GetFileSpec(
	PPx::FSObject&		outFileSpec) const
{
	mNavReply.GetFileSpec(outFileSpec);
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::IsReplacing									  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileDesignator::IsReplacing() const
{
	return mNavReply.IsReplacing();
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::IsStationery									  [public]
// ---------------------------------------------------------------------------

bool
UNavServicesDialogs::LFileDesignator::IsStationery() const
{
	return mNavReply.IsStationery();
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::GetScriptCode								  [public]
// ---------------------------------------------------------------------------

ScriptCode
UNavServicesDialogs::LFileDesignator::GetScriptCode() const
{
	return mNavReply.KeyScript();
}


// ---------------------------------------------------------------------------
//	¥ LFileDesignator::CompleteSave									  [public]
// ---------------------------------------------------------------------------

void
UNavServicesDialogs::LFileDesignator::CompleteSave(
	NavTranslationOptions	inOption)
{
	if (mNavReply.IsValid()) {
		::NavCompleteSave(mNavReply, inOption);
	}
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ NavEventProc													  [static]
// ---------------------------------------------------------------------------
//	Event filter callback routine for Navigation Services

pascal void
UNavServicesDialogs::NavEventProc(
	NavEventCallbackMessage		inSelector,
	NavCBRecPtr					ioParams,
	NavCallBackUserData			ioUserData)
{
	if (inSelector == kNavCBEvent) {
		try {
			UModalAlerts::ProcessModalEvent(*(ioParams->eventData.eventDataParms.event));
		}

		catch (...) { }			// Can't throw back through the Toolbox
	}
	else if (inSelector == kNavCBUserAction)
	{
		// Get callback data
		UNavServicesDialogs::StNavReplyRecord* d = (UNavServicesDialogs::StNavReplyRecord*)ioUserData;
		
		// Get reply recors
		OSErr err = ::NavDialogGetReply(d->GetDialog(), &d->Get());
		
		// Get the result
		d->Result() = ioParams->userAction;
	}
}


// ===========================================================================
//	StNavEventUPP Class
// ===========================================================================

UNavServicesDialogs::StNavEventUPP::StNavEventUPP(
	NavEventProcPtr		inProcPtr)
{
	mNavEventUPP = nil;

	if (inProcPtr != nil) {
		mNavEventUPP = NewNavEventUPP(inProcPtr);
	}
}


UNavServicesDialogs::StNavEventUPP::~StNavEventUPP()
{
	if (mNavEventUPP != nil) {
		DisposeNavEventUPP(mNavEventUPP);
	}
}

#pragma mark -
// ===========================================================================
//	StNavObjectFilterUPP Class
// ===========================================================================

UNavServicesDialogs::StNavObjectFilterUPP::StNavObjectFilterUPP(
	NavObjectFilterProcPtr		inProcPtr)
{
	mNavObjectFilterUPP = nil;

	if (inProcPtr != nil) {
		mNavObjectFilterUPP = NewNavObjectFilterUPP(inProcPtr);
	}
}


UNavServicesDialogs::StNavObjectFilterUPP::~StNavObjectFilterUPP()
{
	if (mNavObjectFilterUPP != nil) {
		DisposeNavObjectFilterUPP(mNavObjectFilterUPP);
	}
}

#pragma mark -
// ===========================================================================
//	StNavPreviewUPP Class
// ===========================================================================

UNavServicesDialogs::StNavPreviewUPP::StNavPreviewUPP(
	NavPreviewProcPtr	inProcPtr)
{
	mNavPreviewUPP = nil;

	if (inProcPtr != nil) {
		mNavPreviewUPP = NewNavPreviewUPP(inProcPtr);
	}
}


UNavServicesDialogs::StNavPreviewUPP::~StNavPreviewUPP()
{
	if (mNavPreviewUPP != nil) {
		DisposeNavPreviewUPP(mNavPreviewUPP);
	}
}


PP_End_Namespace_PowerPlant
