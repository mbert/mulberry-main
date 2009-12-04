// ===========================================================================
//	LUnicodeEditText.cp				PowerPlant 2.2.2	©1997-2001 Metrowerks Inc.
// ===========================================================================
//
//	Text Editing Control
//
//	Implementation of Password variation:
//
//		To support password text entry, we replace the low-level QD procs for
//		drawing and measuring text. Our custom procs always draw or measure
//		the special password character no matter what characters are actually
//		in the field. Before making any call that accesses the text, we
//		install our custom procs, then restore the original procs afterwards.
//
//		With this technique, the password text is actually in the TextEdit
//		record, but it always draws as the password character. So you access
//		the password text the same as normal text using SetText() and
//		GetText(). This differs from the Toolbox EditText control, where
//		there are separate accessors for password and normal text.
//
//		You can set the special password character by calling
//		SetPasswordChar(), which is a static class function. Thus all
//		EditText Panes use the same password character. The default
//		choice is the bullet (¥) character.
//
//		USE WARNING:
//			You should not attach an LUndoer directly to a LUnicodeEditText.
//			If you do, performing undo will show the password field
//			contents in clear text.
//
//			Instead, attach the LUndoer to a supercommander of the
//			LUnicodeEditText, such as the enclosing tab group or window.

#ifdef PowerPlant_PCH
	#include PowerPlant_PCH
#endif

#include <LUnicodeEditText.h>
#include <LEditField.h>
#include <LControlImp.h>
#include <LStream.h>
#include <LView.h>
#include <PP_KeyCodes.h>
#include <PP_Messages.h>
#include <UDrawingState.h>
#include <UDrawingUtils.h>
#include <UGAColorRamp.h>
#include <UTETextAction.h>
#include <UTextTraits.h>
#include <UMemoryMgr.h>
#include <UScrap.h>

#include <Sound.h>

PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------
//	¥ LUnicodeEditText								Stream Constructor		  [public]
// ---------------------------------------------------------------------------

LUnicodeEditText::LUnicodeEditText(
	LStream*	inStream,
	ClassIDT	inImpID)

	: LControlPane(inStream)
{
	SInt16		controlKind;
	ResIDT		textTraitsID;
	Str255		title;

	*inStream >> controlKind;
	if (controlKind == kControlEditTextProc)
		controlKind = kControlEditUnicodeTextProc;
	else if (controlKind == kControlEditTextPasswordProc)
		controlKind = kControlEditUnicodeTextPasswordProc;
	mIsPassword = (controlKind == kControlEditUnicodeTextPasswordProc);

	*inStream >> textTraitsID;
	inStream->ReadPString(title);

	MakeControlImp(inImpID, controlKind, title, textTraitsID);

	*inStream >> mMaxChars;

	UInt8		attributes;
	*inStream >> attributes;

	SInt8	keyFilterID;
	*inStream >> keyFilterID;
}

// ---------------------------------------------------------------------------
//	¥ LUnicodeEditText								Parameterized Constructor [public]
// ---------------------------------------------------------------------------

LUnicodeEditText::LUnicodeEditText(
	const SPaneInfo&	inPaneInfo,
	LCommander*			inSuperCommander,
	ConstStringPtr		inInitialText,
	ResIDT				inTextTraitsID,
	MessageT			inMessage,
	SInt16				inMaxChars,
	UInt8				inAttributes,
	TEKeyFilterFunc		inKeyFilter,
	bool				inPasswordField,
	ClassIDT			inImpID)

	: LControlPane(inPaneInfo, inImpID,
						inPasswordField ? kControlEditUnicodeTextPasswordProc : kControlEditUnicodeTextProc,
						inInitialText, inTextTraitsID, inMessage),
	  LCommander(inSuperCommander)
{
	mMaxChars		= inMaxChars;
}


// ---------------------------------------------------------------------------
//	¥ ~LUnicodeEditText							Destructor				  [public]
// ---------------------------------------------------------------------------

LUnicodeEditText::~LUnicodeEditText()
{
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ SetText														  [public]
// ---------------------------------------------------------------------------

void LUnicodeEditText::SetText(CFStringRef inText)
{
	SetDataTag(kControlEntireControl, kControlEditTextCFStringTag, sizeof(inText), &inText);
}

// ---------------------------------------------------------------------------
//	¥ GetText														  [public]
// ---------------------------------------------------------------------------

PPx::CFString LUnicodeEditText::GetText() const
{
	CFStringRef	text = NULL;

	GetDataTag(kControlEntireControl, mIsPassword ? kControlEditTextPasswordCFStringTag : kControlEditTextCFStringTag, sizeof(text), &text);
					
	return PPx::CFString(text, PPx::retain_No);
}

// ---------------------------------------------------------------------------
//	¥ SetTextTraitsID												  [public]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::SetTextTraitsID(
	ResIDT		inTextTraitsID)
{
	//UTextTraits::SetTETextTraits(inTextTraitsID, mTextEditH);
	mTextTraitsID = inTextTraitsID;
}


// ---------------------------------------------------------------------------
//	¥ GetTextTraitsID												  [public]
// ---------------------------------------------------------------------------

ResIDT
LUnicodeEditText::GetTextTraitsID() const
{
	return mTextTraitsID;
}


// ---------------------------------------------------------------------------
//	¥ SetSelection													  [public]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::SetSelection(
	ControlEditTextSelectionRec&	inSelection)
{
	SetDataTag(kControlEntireControl, kControlEditTextSelectionTag, sizeof(ControlEditTextSelectionRec), &inSelection);
}


// ---------------------------------------------------------------------------
//	¥ SelectAll														  [public]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::SelectAll()
{
	ControlEditTextSelectionRec rec;
	rec.selStart = 0;
	rec.selEnd = 0x7FFF;
	SetSelection(rec);
}


// ---------------------------------------------------------------------------
//	¥ GetSelection													  [public]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::GetSelection(
	ControlEditTextSelectionRec&	outSelection) const
{
	GetDataTag(kControlEntireControl, kControlEditTextSelectionTag, sizeof(ControlEditTextSelectionRec), &outSelection);
}


// ---------------------------------------------------------------------------
//	¥ GetSelection													  [public]
// ---------------------------------------------------------------------------
//	Passes back an AEDesc of the currently selected text

void
LUnicodeEditText::GetSelection(
	AEDesc&		outDesc) const
{
	ControlEditTextSelectionRec rec;
	GetSelection(rec);
	SInt16	selStart = rec.selStart;
	SInt16	selEnd   = rec.selEnd;

#if 0
	StHandleLocker lock((Handle) (**mTextEditH).hText);

	OSErr err = ::AECreateDesc(typeChar,
							   (*(**mTextEditH).hText + selStart),
							   (selEnd - selStart),
							   &outDesc );

	ThrowIfOSErr_(err);
#endif
}


// ---------------------------------------------------------------------------
//	¥ HasSelection													  [public]
// ---------------------------------------------------------------------------

bool
LUnicodeEditText::HasSelection() const
{										// Selection start and end are
										//   different if text is selected
	ControlEditTextSelectionRec rec;
	GetSelection(rec);
	return (rec.selStart != rec.selEnd);
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ HideSelf														  [public]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::HideSelf()
{
	if (IsOnDuty()) {					// Hidden field can't be the Target
		SwitchTarget(GetSuperCommander());
	}

	LControlPane::HideSelf();
}


// ---------------------------------------------------------------------------
//	¥ ActivateSelf												   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::ActivateSelf()
{
	if (mEnabled == triState_On) {
		Refresh();
	}
	
	LControlPane::ActivateSelf();
}



// ---------------------------------------------------------------------------
//	¥ DeactivateSelf											   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::DeactivateSelf()
{
	if (IsEnabled()) {
		Refresh();
	}

	LControlPane::DeactivateSelf();
}


// ---------------------------------------------------------------------------
//	¥ EnableSelf												   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::EnableSelf()
{
	if (mActive == triState_On) {
		Draw(nil);
		DontRefresh();
	}
	
	LControlPane::EnableSelf();
}


// ---------------------------------------------------------------------------
//	¥ DisableSelf												   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::DisableSelf()
{
	if (IsOnDuty()) {					// Disabled field can't be the Target
		SwitchTarget(GetSuperCommander());
	}

	if (mActive == triState_On) {
		Draw(nil);
		DontRefresh();
	}
	
	LControlPane::DisableSelf();
}


// ---------------------------------------------------------------------------
//	¥ ClickSelf													   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::ClickSelf(
	const SMouseDownEvent&	inMouseDown)
{
	if (!IsTarget()) {				// If not the Target, clicking in an
									//   EditField makes it the Target.
		SwitchTarget(this);
	}

	LControlPane::ClickSelf(inMouseDown);
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ HandleKeyPress												  [public]
// ---------------------------------------------------------------------------

Boolean
LUnicodeEditText::HandleKeyPress(
	const EventRecord&	inKeyEvent)
{
	Boolean		keyHandled	 = true;
	EKeyStatus	theKeyStatus = keyStatus_Input;
	UInt16		theChar		 = (UInt16) (inKeyEvent.message & charCodeMask);
	LCommander	*theTarget	 = GetTarget();

	if (inKeyEvent.modifiers & cmdKey) {	// Always pass up when the command
		theKeyStatus = keyStatus_PassUp;	//   key is down

	}

	mControlImp->DoKeyPress(inKeyEvent);	// Let imp handle keypress

	return LCommander::HandleKeyPress(inKeyEvent);
}


// ---------------------------------------------------------------------------
//	¥ ObeyCommand													  [public]
// ---------------------------------------------------------------------------

Boolean
LUnicodeEditText::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand) {

		case msg_TabSelect:
			if (!IsEnabled()) {
				cmdHandled = false;
				break;
			} // else FALL THRU to SelectAll()

		case cmd_SelectAll:
			SelectAll();
			break;

		default:
			cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}


// ---------------------------------------------------------------------------
//	¥ FindCommandStatus
// ---------------------------------------------------------------------------
//	Pass back the status of a Command

void
LUnicodeEditText::FindCommandStatus(
	CommandT	inCommand,
	Boolean&	outEnabled,
	Boolean&	outUsesMark,
	UInt16&		outMark,
	Str255		outName)
{
	switch (inCommand) {

		case cmd_Cut:				// Cut and Copy disabled for Password
		case cmd_Copy:				// Otherwise, enable if text is selected
			outEnabled = not mIsPassword && HasSelection();
			break;

		case cmd_Clear:				// Check if selection is not empty
			outEnabled = HasSelection();
			break;

		case cmd_Paste:				// Check if TEXT is in the Scrap
			outEnabled = UScrap::HasData(ResType_Text);
			break;

		case cmd_SelectAll:			// Check if any characters are present
		{
			PPx::CFString txt = GetText();
			outEnabled = txt.GetLength() > 0;
			break;
		}

		default:
			LCommander::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}


// ---------------------------------------------------------------------------
//	¥ UserChangedText
// ---------------------------------------------------------------------------
//	Text of EditText has changed as a result of user action
//
//	Broadcasts value message with a pointer to "this" object as the parameter.

void
LUnicodeEditText::UserChangedText()
{
	if (mValueMessage != msg_Nothing) {
		BroadcastMessage(mValueMessage, this);
	}
}


// ---------------------------------------------------------------------------
//	¥ BeTarget													   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::BeTarget()
{
	mControlImp->BeTarget(kControlFocusNextPart);
}


// ---------------------------------------------------------------------------
//	¥ DontBeTarget												   [protected]
// ---------------------------------------------------------------------------

void
LUnicodeEditText::DontBeTarget()
{
	if (UEnvironment::IsRunningOSX()) {
	
		// On Mac OS X 10.1, things changed so that the focus
		// hiliting does not automatically erase. So, we force
		// a redraw on X (any version). This won't cause a flash
		// since windows on X are buffered.
		
		Refresh();
	}

	mControlImp->DontBeTarget();
}


#pragma mark -

// ---------------------------------------------------------------------------
//	¥ TooManyCharacters
// ---------------------------------------------------------------------------
//	Return whether adding the specified number of characters will exceed
//	the maximum allowed. This function assumes that the characters being
//	added will replace the current selection.

Boolean
LUnicodeEditText::TooManyCharacters(
	SInt32	inCharsToAdd)
{
	PPx::CFString txt = GetText();
	ControlEditTextSelectionRec rec;
	GetSelection(rec);
	return ( (txt.GetLength() +
			  inCharsToAdd -
			  (rec.selEnd - rec.selStart))
			 > mMaxChars );
}

#pragma mark -

PP_End_Namespace_PowerPlant
