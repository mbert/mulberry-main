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


// Source for CTextFieldX class

#include "CTextFieldX.h"

#include "CContextMenu.h"

#include "cdstring.h"
#include "cdustring.h"

#include <LControlImp.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T F I E L D
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CTextFieldX::CTextFieldX(LStream *inStream)
		: CTextBase(inStream)
{
	ResIDT	inTextTraitsID;
	SInt32	msg;
	Boolean broadcastall;
	Boolean password;
	Boolean numbers;

	*inStream >> inTextTraitsID;
	*inStream >> msg;
	*inStream >> broadcastall;
	*inStream >> password;
	*inStream >> numbers;

	if (password)
		SetPasswordMode();

	mValueMessage = msg;
	mBroadcastAll = broadcastall;
	mBroadcastReturn = false;
	mNumbers = numbers;

	TextTraitsPtr	traitsPtr = nil;
	
	if (inTextTraitsID != Txtr_SystemFont)
	{
		TextTraitsH	traitsH = UTextTraits::LoadTextTraits(inTextTraitsID);
		
		if (traitsH != nil) {
			StHandleLocker	lock((Handle)traitsH);
			SetTextTraits(**traitsH);
		}
	}

	ResizeFrameBy(-8, -8, false);
	MoveBy(4, 4, false);

	Rect margins = { 1, 1, 1, 1 };  /* t,l,b,r */
	SetMargins(margins);
}

// Default destructor
CTextFieldX::~CTextFieldX()
{
}

// Constructor from stream
CTextFieldMultiX::CTextFieldMultiX(LStream *inStream)
		: CTextFieldX(inStream)
{
}

// Default destructor
CTextFieldMultiX::~CTextFieldMultiX()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup D&D
void CTextFieldX::FinishCreateSelf()
{
	// Do inherited
	CTextBase::FinishCreateSelf();

	CContextMenuAttachment::AddUniqueContext(this, 2500, this, false);

}

// ---------------------------------------------------------------------------
//	¥ CalcRevealedRect
// ---------------------------------------------------------------------------
//	Calculate the portion of the Frame which is revealed through the
//	Frames of all SuperViews. RevealedRect is in Port coordinates.

void
CTextFieldX::CalcRevealedRect()
{
		// To accommodate controls which draw outside their Frame,
		// this function calculates the portion of its Structure rect
		// that is revealed
	
	if (CalcPortStructureRect(mRevealedRect)) {
									// Frame is in QD Space
		if (mSuperView != nil) {	// Intersect Frame with RevealedRect
									//   of SuperView
			Rect	superRevealed;
			mSuperView->GetRevealedRect(superRevealed);
			::SectRect(&superRevealed, &mRevealedRect, &mRevealedRect);
		}

	} else {						// Frame not in QD Space
		mRevealedRect.left =		//   so RevealedRect is empty
			mRevealedRect.top =
			mRevealedRect.right =
			mRevealedRect.bottom = 0;
	}
}


// ---------------------------------------------------------------------------
//	¥ CalcPortStructureRect											  [public]
// ---------------------------------------------------------------------------
//	Calculate the ControlView's Structure Rectangle in Port coordinates
//
//	A ControlView may draw outside of its Frame. This feature was added to
//	support Appearance Manager controls, which may draw outside their
//	control rectangles.
//
//	Returns true if the Rect is within QuickDraw space (16-bit)
//	Returns false if the Rect is outside QuickDraw space
//		and outRect is unchanged

bool
CTextFieldX::CalcPortStructureRect(
	Rect&	outRect) const
{
	bool	isInQDSpace = CalcPortFrameRect(outRect);

	if (isInQDSpace) {
	
			// Adjust Frame to account for border. On Platinum Theme, the
			// border is two pixels. For Carbon Targets, we can ask the
			// OS for the border size. We subtract one to compensate for
			// the one pixel border which PP scrollers historically used.
	
		SInt32		outset = 4;			// Two minus one
		
		::MacInsetRect(&outRect, (SInt16) -outset, (SInt16) -outset);
	}
	
	return isInQDSpace;
}


// ---------------------------------------------------------------------------
//	¥ CalcPortExposedRect											  [public]
// ---------------------------------------------------------------------------
//	Calculate the exposed rectangle of a Pane and return whether that
//	rectangle is empty. The rectangle is in Port coordinates.

Boolean
CTextFieldX::CalcPortExposedRect(
	Rect&	outRect,						// In Port coords
	bool	inOKIfHidden) const
{
	bool exposed = ((mVisible == triState_On) || inOKIfHidden)  &&
				   CalcPortStructureRect(outRect)  &&
				   (mSuperView != nil);

	if (exposed) {
		Rect	superRevealed;
		mSuperView->GetRevealedRect(superRevealed);

		exposed = ::SectRect(&outRect, &superRevealed, &outRect);
	}

	return exposed;
}

void CTextFieldX::DrawSelf()
{
	CTextWidget::DrawSelf();

	ThemeDrawState	state = kThemeStateInactive;
	
	if (IsActive() and IsEnabled()) {
		state = kThemeStateActive;
	}

	ApplyForeAndBackColors();
	::PenNormal();

	Rect	frame;
	CalcLocalFrameRect(frame);
	//::MacInsetRect(&frame, 3, 3);

	if (IsTarget())
	{
		::DrawThemeEditTextFrame(&frame, state);
		::DrawThemeFocusRect(&frame, true);
	}
	else
	{
		::DrawThemeFocusRect(&frame, false);
		::DrawThemeEditTextFrame(&frame, state);
	}
}

// ---------------------------------------------------------------------------
//	¥ ActivateSelf												   [protected]
// ---------------------------------------------------------------------------

void
CTextFieldX::ActivateSelf()
{
	if (mEnabled == triState_On) {
		Refresh();
	}
	CTextBase::ActivateSelf();
}



// ---------------------------------------------------------------------------
//	¥ DeactivateSelf											   [protected]
// ---------------------------------------------------------------------------

void
CTextFieldX::DeactivateSelf()
{
	if (IsEnabled()) {
		Refresh();
	}
	CTextBase::DeactivateSelf();
}


// ---------------------------------------------------------------------------
//	¥ EnableSelf												   [protected]
// ---------------------------------------------------------------------------

void
CTextFieldX::EnableSelf()
{
	if (mActive == triState_On) {
		Draw(nil);
		DontRefresh();
	}
}


// ---------------------------------------------------------------------------
//	¥ DisableSelf												   [protected]
// ---------------------------------------------------------------------------

void
CTextFieldX::DisableSelf()
{
	if (IsOnDuty()) {					// Disabled field can't be the Target
		SwitchTarget(GetSuperCommander());
	}

	if (mActive == triState_On) {
		Draw(nil);
		DontRefresh();
	}
}


// ---------------------------------------------------------------------------
//	¥ BeTarget
// ---------------------------------------------------------------------------
//	EditField is becoming the Target

void
CTextFieldX::BeTarget()
{
	// Do inherited
	CTextBase::BeTarget();
	Refresh();
}


// ---------------------------------------------------------------------------
//	¥ DontBeTarget
// ---------------------------------------------------------------------------
//	EditField is no longer the Target

void
CTextFieldX::DontBeTarget()
{
	// Do inherited
	CTextBase::DontBeTarget();
	Refresh();
}

// Handle non-editable case
Boolean CTextFieldX::HandleKeyPress(const EventRecord& inKeyEvent)
{
	bool		keyProcess = true;
	bool		keyHandled = false;
	EKeyStatus	theKeyStatus = keyStatus_Input;
	UInt16		theKey = inKeyEvent.message & charCodeMask;

	if (inKeyEvent.modifiers & cmdKey) {	// Always pass up when the command
		theKeyStatus = keyStatus_PassUp;	//   key is down

	} else {
		theKeyStatus = mNumbers ? UKeyFilters::IntegerField(inKeyEvent) : UKeyFilters::PrintingCharField(inKeyEvent);
	}

	// Arrow and navigation keys can respond to command key
	switch(theKey)
	{
	case char_LeftArrow:
	case char_RightArrow:
	case char_UpArrow:
	case char_DownArrow:
	case char_Home:
	case char_End:
	case char_PageUp:
	case char_PageDown:
		return keyHandled = CTextWidget::HandleKeyPress(inKeyEvent);
	}

	switch (theKeyStatus)
	{
	case keyStatus_PassUp:
		keyHandled = LCommander::HandleKeyPress(inKeyEvent);
		break;

	// Pass ordinary keys up to super-commander if text is not editable
	case keyStatus_Input:
	case keyStatus_TEDelete:
		// Only do if edits allowed
		if (IsReadOnly())
		{
			keyHandled = LCommander::HandleKeyPress(inKeyEvent);
			break;
		}

		// Fall through for key processing

	default:
		keyHandled = CTextBase::HandleKeyPress(inKeyEvent);
		break;
	}

	// Broadcast if required
	switch (theKey)
	{
	case char_Return:
	case char_Enter:
		if (mBroadcastReturn)
		{
			if (mValueMessage != msg_Nothing) {
				BroadcastMessage(mValueMessage, this);
			}
			SelectAll();
			keyHandled = true;
		}
		break;
	default:
		// This is now handled by userChangedText
#if 0
		if (mBroadcastAll && !UKeyFilters::IsActionKey(inKeyEvent.message))
		{
			if (mValueMessage != msg_Nothing) {
				BroadcastMessage(mValueMessage, this);
			}
		}
#endif
		break;
	}

	return keyHandled;
}

Boolean CTextFieldX::ObeyCommand(
	CommandT	inCommand,
	void*		ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand)
	{
	case msg_TabSelect:
		if (!IsEnabled()) {
			cmdHandled = false;
			break;
		} // else FALL THRU to SelectAll()

	case cmd_SelectAll:
		SelectAll();
		break;

	default:
		cmdHandled = CTextBase::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

void CTextFieldX::UserChangedText()
{
	if (mBroadcastAll)
	{
		if (mValueMessage != msg_Nothing) {
			BroadcastMessage(mValueMessage, this);
		}
	}
}

void CTextFieldX::SetTextTraits(const TextTraitsRecord& aTextTrait)
{
	// Store new traits and invalidate traits id
	mTextTraits = aTextTrait;
	if (mTextTraits.fontName[0] == 0)
		::GetFontName(mTextTraits.fontNumber, mTextTraits.fontName);

	// Update TE with new traits
	{
		SetSelection(0, LONG_MAX);
		SetFontName(mTextTraits.fontName);
		SetFontSize(::Long2Fix(mTextTraits.size));
		SetFontStyle(mTextTraits.style);
		SetFontColor(mTextTraits.color);
	}

	Refresh();
}

// Set number
void CTextFieldX::SetNumber(long num)
{
	cdstring txt(num);
	SetText(txt);
}

// Get number
long CTextFieldX::GetNumber() const
{
	cdstring result;
	GetText(result);
	return ::atol(result.c_str());
}
