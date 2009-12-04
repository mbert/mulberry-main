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


// Source for CToolbarButtonImp class

#include "CToolbarButtonImp.h"

#include "CMulberryCommon.h"
#include "CToolbarButton.h"

#include <UDrawingState.h>
#include <UDrawingUtils.h>
#include <UEventMgr.h>
#include <UGraphicUtils.h>
#include <UGAColorRamp.h>

const	UInt16	bevelButton_SmallMark	=	char_Bullet;	// Mark used for small font popups

enum {
	bevelButton_GraphicSlop 			= 	3,
	bevelButton_LargePopupGlyphSize 	= 	9,
	bevelButton_LargePopupGlyphRows 	= 	5,
	bevelButton_SmallPopupGlyphSize 	= 	5,
	bevelButton_SmallPopupGlyphRows 	= 	3,
	bevelButton_ClickAndPopupWidth 		=	12
};

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CToolbarButtonImp::CToolbarButtonImp(LStream *inStream)
	: LAMBevelButtonImp(inStream)
{
	mSmallIcon = false;
	mShowIcon = true;
	mShowText = true;
	mMouseInside = false;
	mPushed = false;
	mPrevInfo.state = kThemeStateActive;
	mPrevInfo.value = kThemeButtonOff;
	mPrevInfo.adornment = kThemeAdornmentNone;
	mMultiValueMenu = false;
	mArrowRight = false;
	mClickAndPopup = false;
	mClickOnPopup = false;
}

// Default destructor
CToolbarButtonImp::~CToolbarButtonImp()
{
}

void CToolbarButtonImp::Init (LControlPane* inControlPane, SInt16 inControlKind, ConstStringPtr inTitle, ResIDT inTextTraitsID, SInt32 inRefCon )
{
	UInt16 theBehavior = (UInt16) inControlPane->GetMinValue();
	mMultiValueMenu = (theBehavior & kControlBehaviorMultiValueMenu) != 0;
	mArrowRight = (inControlKind & kControlBevelButtonMenuOnRight) != 0;

	LAMBevelButtonImp::Init(inControlPane, inControlKind, inTitle, inTextTraitsID, inRefCon);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CToolbarButtonImp::GetDataTag(SInt16 inPartCode, FourCharCode inTag, Size inBufferSize, void* inBuffer, Size* outDataSize ) const
{
	Size dataSize = 0;
													// Handle getting the data for the various tags
													// supported  by the bevel button
	switch (inTag)
	{
	case kControlBevelButtonMenuValueTag:
		if (inBufferSize == sizeof(SInt16))
		{
			dataSize = sizeof(SInt16);
			*(SInt16*) inBuffer = GetMenuValue();
		}
	break;

	default:
		LAMBevelButtonImp::GetDataTag(inPartCode, inTag, inBufferSize, inBuffer, outDataSize);
		if (outDataSize != NULL)
			dataSize = *outDataSize;
		break;
	}
														// If we are being asked to return the data
														// size then handle that now
	if (outDataSize != NULL)
		*outDataSize = dataSize;
}

#pragma mark -
#pragma mark === MOUSE TRACKING

//	Track mouse while it is being pressed

Boolean CToolbarButtonImp::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
													// If we have a popup then we need to handle
													// things a little differently than we do just
													// for a button, if there is no popup we simply
													// let our superclass handle things
	if (!HasPopup())
		return TrackHotSpotButton(inHotSpot, inPoint, inModifiers);
	else if (!GetClickAndPopup())
		return TrackHotSpotPopup(inHotSpot, inPoint, inModifiers);
	else
	{
		// Determine which part it is in
		Rect frameRect;
		CalcLocalFrameRect(frameRect);
		frameRect.left = frameRect.right - bevelButton_ClickAndPopupWidth;
		if (::MacPtInRect(inPoint, &frameRect))
			return TrackHotSpotPopup(inHotSpot, inPoint, inModifiers);
		else
			return TrackHotSpotButton(inHotSpot, inPoint, inModifiers);
	}
}

Boolean CToolbarButtonImp::TrackHotSpotButton(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
										// For the initial mouse down, the
										//   mouse is currently inside the
										//   HotSpot when it was previously
										//   outside
	Boolean		currInside = true;
	Boolean		prevInside = false;
	mControlPane->HotSpotAction(inHotSpot, currInside, prevInside);

	StRegion		clipR;				// Get copy of clipping region
	::GetClip(clipR);
										// Track the mouse while it is down
	Point	currPt = inPoint;
	while (::StillDown()) {
		::GetMouse ( &currPt );			// Must keep track if mouse moves from
		prevInside = currInside;		// In-to-Out or Out-To-In
		currInside =  (mControlPane->PointInHotSpot(currPt, inHotSpot)
							&& ::PtInRgn(currPt, clipR));
		mControlPane->HotSpotAction(inHotSpot, currInside, prevInside);
	}

										// Get location from MouseUp event
	EventRecord	macEvent;
	if (UEventMgr::GetMouseUp(macEvent))
	{
		currPt = macEvent.where;
		::GlobalToLocal(&currPt);
		prevInside = currInside;
		currInside = (mControlPane->PointInHotSpot(currPt, inHotSpot) && ::PtInRgn(currPt, clipR));
		mControlPane->HotSpotAction(inHotSpot, currInside, prevInside);
	}

	mClickOnPopup = false;

										// Return if we are still inside the
	return currInside;					//   control or not
}

Boolean CToolbarButtonImp::TrackHotSpotPopup(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	MenuHandle	menuH = GetMacMenuH ();

												// We only want the popup menu to appear if the
												// mouse went down in the our hot spot which is
												// the popup portion of the control
	if ( PointInHotSpot ( inPoint, inHotSpot ) && menuH != nil ) {
												// Get things started off on the right foot
		Boolean		currInside = true;
		Boolean		prevInside = false;
		HotSpotAction ( inHotSpot, currInside, prevInside );

												// We skip the normal tracking that is done in
												// the control as the call to PopupMenuSelect
												// will take control of the tracking once the
												// menu is up.
												// Now we need to handle the display of the
												// actual popup menu we start by setting up some
												// values that we will need
		SInt16	menuID = 0;
		SInt16 menuItem = GetMenuValue();
		SInt16	currItem = 1;
		Point popLoc;
		GetPopupMenuPosition ( popLoc );

												// Call our utility function which handles the
												// display of the menu, menu is disposed of
												// inside this function
		HandlePopupMenuSelect ( popLoc, currItem, menuID, menuItem );

												// Setup the current item field with the newly
												// selected item
		if ( menuItem > 0 ) {
			SetupCurrentMenuItem (  menuH, menuItem );

												// NOTE: this was a call to SetValue but I
												// changed it to a PokeValue because it was
												// resulting in a double broadcast of the value
												// message  which occurs because the default
												// behavior of a pushbutton is to just broadcast
												// its value message when the button is pushed a
												// popup bevel button always has the pushbutton
												// or toggle behavior, this gets around the
												// problem
			//mControlPane->PokeValue ( menuItem );
			SetDataTag(0, kControlBevelButtonMenuValueTag, sizeof(SInt16), &menuItem);
		}

												// Make sure that we get the HotSpotAction
												// called one last time so that the pushed state
												// is cleared
		HotSpotAction ( inHotSpot, false, true );

		mClickOnPopup = true;

												// Return true if an item was selected
		return menuItem > 0;
	}
	
	return false;
}

void CToolbarButtonImp::HotSpotAction(SInt16 inHotSpot, Boolean inCurrInside, Boolean inPrevInside)
{
	// Turn hiliting on and off as appropriate
	SetPushedState(inCurrInside);
}

#pragma mark -
#pragma mark === POPUP MENU

SInt16 CToolbarButtonImp::GetMenuFontSize () const
{
													// Get the font size for the popup so we can
													// figure out the correct mark to apply
	StTextState			theTextState;

	ResIDT	textTID = GetTextTraitsID ();
	TextTraitsH traitsH = UTextTraits::LoadTextTraits ( textTID );
	if (traitsH)  {
		return (**traitsH).size;
	}
													// Because we should never really get here we
													// simply return zero
	return 0;
}

void CToolbarButtonImp::GetPopupMenuPosition(Point& outPopupLoc )
{
													// Get the local popup button frame, this will
													// be correctly setup depending on whether the
													// button is arrow only or not
	Rect	popupRect;
	CalcLocalFrameRect ( popupRect );

	if ( !HasPopupToRight ()) {
		outPopupLoc.v = popupRect.bottom;
		outPopupLoc.h = popupRect.left;

	} else {
													// Setup the vertical value
		outPopupLoc.v = popupRect.top;

													// Setup the horizontal component which is
													// always at the right edge
		outPopupLoc.h = popupRect.right;
	}

													// Because PopMenuSelect needs the location for
													// the popup menu global coordinates we need to
													// convert the location
	mControlPane->LocalToPortPoint ( outPopupLoc );
	mControlPane->PortToGlobalPoint ( outPopupLoc );

}

void CToolbarButtonImp::SetMenuValue(SInt16 inItem)
{
	LBevelButton	*button = dynamic_cast<LBevelButton*>(mControlPane);
	if (button) {
		button->SetCurrentMenuItem(inItem);
	}
}

void CToolbarButtonImp::SetupCurrentMenuItem(MenuHandle inMenuH, SInt16 inCurrentItem)
{
													// We need to be able to handle two different
													// approaches to the marking of the menu items,
													// 1) for the multivalue case we allow the user
													// to mark and unmark all the items in the menu,
													// 2) we handle the standard behavior of only
													// one item marked at a time in the menu
	if ( inMenuH ) {
		SInt16	mark;
		if ( HasMultiValueMenu ()) {
													// We start out by checking if the item is
													// marked or not, if it is not mark then the
													// mark is set to our mark otherwise it is left
													// at 0 which will result in the mark being
													// removed
			::GetItemMark ( inMenuH, inCurrentItem, &mark );
			if ( mark == 0 ) {
				mark = (SInt16) (GetMenuFontSize () < 12 ? bevelButton_SmallMark : checkMark);
			} else {
				mark = 0;
			}

		} else {
			SInt16		oldItem = GetMenuValue();
			if ( oldItem != inCurrentItem  ) {
													// Remove the old mark
				::SetItemMark ( inMenuH, oldItem, 0 );
			}
													// Always make sure the item is marked
			mark = (SInt16) (GetMenuFontSize () < 12 ? bevelButton_SmallMark : checkMark);
		}
													// Now we can finally get the mark set, this
													// will either mark it or not based on the value
													// for the mark
		::SetItemMark ( 	inMenuH,
								inCurrentItem,
								mark );

		SetMenuValue(inCurrentItem);
	}
}

void CToolbarButtonImp::HandlePopupMenuSelect(Point inPopupLoc, SInt16 inCurrentItem, SInt16 &outMenuID, SInt16 &outMenuItem)
{
													// Load the menu from the resource
	MenuHandle	menuH = GetMacMenuH();
	if (menuH)
	{
													// Save off the current system font family and
													// size
		SInt16 saveFont = ::LMGetSysFontFam();
		SInt16 saveSize = ::LMGetSysFontSize();

													// Enclose this all in a try catch block so that
													// we can at least reset the system font if
													// something goes wrong
		try
		{
													// Handle the actual insertion into the hierarchical menubar
			::MacInsertMenu(menuH, hierMenu);

													// Reconfigure the system font so that the menu
													// will be drawn in our desired font and size
			mControlPane->FocusDraw();
			{
				ResIDT	textTID = GetTextTraitsID();
				TextTraitsH traitsH = UTextTraits::LoadTextTraits(textTID);
				if (traitsH) 
				{
					::LMSetSysFontFam((**traitsH).fontNumber);
					::LMSetSysFontSize((**traitsH).size);
					::LMSetLastSPExtra(-1L);
				}
			}
													// Before we display the menu we need to make
													// sure that we have the current item marked in
													// the menu. NOTE: we do NOT use the current
													// item that has been passed in here as that
													// always has a value of one as our menus are
													// always pulldown menus.  We only do this if
													// the menu is not a multi value menu
			if ( !HasMultiValueMenu())
			{
				SetupCurrentMenuItem(menuH, GetMenuValue());
			}
													// Then we call PopupMenuSelect and wait for it
													// to return
			SInt32 result = ::PopUpMenuSelect(menuH,
												inPopupLoc.v,
												inPopupLoc.h,
												inCurrentItem);

													// Then we extract the values from the returned
													// result these are then passed back out to the
													// caller
			outMenuID = HiWord(result);
			outMenuItem = LoWord(result);

		} catch (...)  { }
													// Restore the system font
		::LMSetSysFontFam(saveFont);
		::LMSetSysFontSize(saveSize);
		::LMSetLastSPExtra(-1L);
													// Finally get the menu removed
		::MacDeleteMenu(::GetMenuID(menuH));
	}
}

#pragma mark -
#pragma mark === DRAWING STATE

void CToolbarButtonImp::SetMouseInside(bool inside)
{
	if (mMouseInside ^ inside)
	{
		mMouseInside = inside;
		
		// Only bother if active to prevent flashing
		if (IsActive() && IsEnabled())
			mControlPane->Draw(NULL);
	}
}

void CToolbarButtonImp::SetPushedState(bool inPushedState)
{
	if (mPushed ^ inPushedState)
	{
		mPushed = inPushedState;

		mControlPane->Draw(NULL);
	}
}

void CToolbarButtonImp::DrawSelf()
{
	{
		StClipOriginState	saveClipOrigin(Point_00);
		Rect	frame;
		CalcLocalFrameRect(frame);
		mControlPane->GetContainer()->ApplyForeAndBackColors();
		::EraseRect(&frame);
	}

	// Policy:
	//  Draw plain icon + text if not tracking and mouse is outside, or disabled/inactive
	//  Draw button frame if tracking or mouse inside
	
	if ((mMouseInside || mControlPane->GetValue()) && IsActive() && IsEnabled())
		DrawFrame();
	
	if (mShowIcon)
		DrawIcon();
	
	if (mShowText)
		DrawText();

	if (HasPopup())
		DrawPopupGlyph();
}

void CToolbarButtonImp::DrawFrame()
{
	Rect	frame;
	CalcLocalFrameRect(frame);
		
	ThemeButtonDrawInfo newInfo;
	newInfo.state = mControlPane->GetValue() ^ mPushed ? kThemeStatePressed : kThemeStateActive;
	newInfo.value = (mControlPane->GetValue() ? kThemeButtonOn : kThemeButtonOff);
	newInfo.adornment = kThemeAdornmentNone;

	::DrawThemeButton(&frame, kThemeSmallBevelButton, &newInfo, &mPrevInfo, NULL, NULL, 0);

	// Special line for click-and-popup
	if (GetClickAndPopup())
	{
		frame.left = frame.right - bevelButton_ClickAndPopupWidth;
		frame.right = frame.left + 3;
		frame.top += 2;
		frame.bottom -= 2;
		::DrawThemeSeparator(&frame, kThemeStateActive);
	}

	mPrevInfo = newInfo;
}


void CToolbarButtonImp::DrawIcon()
{
	Rect	frame;
	CalcLocalFrameRect(frame);
	if (GetClickAndPopup())
		frame.right -= bevelButton_ClickAndPopupWidth;
	frame.top += (mSmallIcon ? 2 : 0);
	frame.bottom = frame.top + (mSmallIcon ? 16 : 32);
	frame.left = (frame.right + frame.left - (mSmallIcon ? 16 : 32)) / 2;
	if (HasPopup() && !mShowText)
		frame.left -= 2;
	frame.right = frame.left + (mSmallIcon ? 16 : 32);

	short resid = static_cast<CToolbarButton*>(mControlPane)->Sticns::GetResID();
	if (static_cast<CToolbarButton*>(mControlPane)->GetIconToggle() && mControlPane->GetValue())
		resid++;

	IconTransformType transform = kTransformNone;
	if (IsActive() && IsEnabled())
		transform = mPushed ? kTransformSelected : kTransformNone;
	else
		transform = kTransformDisabled;
	
	::Ploticns(&frame, kAlignNone, transform, resid);
}

void CToolbarButtonImp::DrawText()
{
	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

	// Set to required text
	UTextTraits::SetPortTextTraits(mControlPane->GetTextTraitsID());

	// Set text color using theme
	ThemeTextColor color;
	if (IsActive() && IsEnabled())
		color = mControlPane->GetValue() ^ mPushed ? kThemeTextColorBevelButtonPressed : kThemeTextColorBevelButtonActive;
	else
		color = kThemeTextColorBevelButtonInactive;
	::SetThemeTextColor(color, mControlPane->GetBitDepth(), mControlPane->GetBitDepth() > 1);

	// Get the text
	LStr255 title;
	mControlPane->GetDescriptor(title);
	
	// Get rect to draw inside
	Rect	frame;
	CalcLocalFrameRect(frame);
	if (GetClickAndPopup())
		frame.right -= bevelButton_ClickAndPopupWidth;
	::InsetRect(&frame, -2, 0);

	// Draw text - center it unless this is a popup without an icon
	::MoveTo(frame.left, frame.bottom - 4);
	::DrawClippedString(title, frame.right - frame.left, (HasPopup() && !mShowIcon) ? eDrawString_Left : eDrawString_Center);
}

void CToolbarButtonImp::DrawPopupGlyph()
{

													// Get the frame for the control
	Rect	localFrame;
	CalcLocalFrameRect (localFrame);
	if (mShowIcon)
		localFrame.bottom = localFrame.top + (mSmallIcon ? 20 : 32);
	SInt16	height = UGraphicUtils::RectHeight ( localFrame );

	RGBColor			indicatorColor = Color_Black;

	SInt16	bitDepth;
	bool	hasColor;
	mControlPane->GetDeviceInfo(bitDepth, hasColor);

	if (!hasColor || (bitDepth < 4)) {			// BLACK & WHITE
												// We only draw the indicator when we are not
												// hilited as the entire button including the
												// indicator is inverted for the hilited state
												// We draw the indicator in black when normal,
												// white when hilited and with a gray pattern
												// when the button is dimmed
		indicatorColor = (IsPushed () && IsEnabled ()) ? Color_White : Color_Black;
		if ( !IsEnabled ()) {
			StColorPenState::SetGrayPattern();
		}

	} else {									// COLOR
												// We draw the indicator in black when normal
												// gray 8 when dimmed and white when hilited
		indicatorColor = Color_Black;
		if ( !IsEnabled () || !IsActive ()) {
			 indicatorColor = UGAColorRamp::GetColor(8);
		}

		if ( IsPushed ()) {
			indicatorColor = UGAColorRamp::GetColor(8);
			if (IsEnabled()) {
				indicatorColor = Color_White;
			}
		}
	}

												// Set the foreground color for our drawing
	::RGBForeColor ( &indicatorColor );

												// Setup a bunch of variables that are going to
												// be used to draw the glyph that take into
												// account the size and the orientation
	SInt16	startVPosition, startHPosition, endHPosition, endVPosition;
	SInt16	size, rows;
	size = bevelButton_SmallPopupGlyphSize;
	rows = bevelButton_SmallPopupGlyphRows;

	if ( !HasPopupToRight ()) {		// DOWN

		startVPosition = (SInt16) (localFrame.bottom - (3 + rows));
		startHPosition = (SInt16) (localFrame.right - (3 + size));
		endHPosition = (SInt16) (startHPosition + size);

	} else {									// RIGHT

		startVPosition = (SInt16) (localFrame.bottom - (3 + size));
		startHPosition = (SInt16) (localFrame.right - (3 + rows));
		endVPosition = (SInt16) (startVPosition + size);
	}
												// The glyph is drawn in the bottom right
												// corner of the button
	SInt16 index;
	for ( index = 0; index < rows; index++ ) {
		::MoveTo ( 	(SInt16) (startHPosition + index),
						(SInt16) (startVPosition + index));
		if ( !HasPopupToRight ()) {
			::Line ( (SInt16) (size - (1 + (2 * index))), 0 );
		} else {
			::Line ( 0, (SInt16) (size - (1 + (2 * index))));
		}
	}
}
