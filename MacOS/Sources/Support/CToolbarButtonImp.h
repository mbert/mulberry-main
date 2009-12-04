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


// Header for CToolbarButtonImp class

#ifndef __CTOOLBARBUTTONIMP__MULBERRY__
#define __CTOOLBARBUTTONIMP__MULBERRY__

#include <LAMBevelButtonImp.h>

// Constants

// Classes
class CToolbarButtonImp : public LAMBevelButtonImp
{
public:
	enum { class_ID = 'TBit' };
	
						CToolbarButtonImp(LStream *inStream);
	virtual				~CToolbarButtonImp();

	virtual	void		Init(	LControlPane*	inControlPane,
								SInt16			inControlKind,
								ConstStringPtr	inTitle = Str_Empty,
								ResIDT			inTextTraitsID = 0,
								SInt32			inRefCon = 0);

	virtual	void		GetDataTag(
								SInt16				inPartCode,
								FourCharCode		inTag,
								Size				inBufferSize,
								void*				inBuffer,
								Size				*outDataSize = nil) const;

	void	SetMouseInside(bool inside);

	void SetSmallIcon(bool small_icon)
		{ mSmallIcon = small_icon; }
	void SetShowIcon(bool icon)
		{ mShowIcon = icon; }
	void SetShowCaption(bool caption)
		{ mShowText = caption; }

	bool GetClickOnPopup() const
		{ return mClickOnPopup; }
	bool GetClickAndPopup() const
		{ return mClickAndPopup; }
	void SetClickAndPopup(bool clickpopup)
		{ mClickAndPopup = clickpopup; }

	bool				IsPushed() const
		{ return mPushed; }
	virtual	void		SetPushedState(bool	inPushedState);

	virtual	void		SetupCurrentMenuItem(
								MenuHandle		inMenuH,
								SInt16			inCurrentItem);


	virtual Boolean		TrackHotSpot(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);
	virtual void		HotSpotAction(
								SInt16		inHotSpot,
								Boolean		inCurrInside,
								Boolean		inPrevInside);
	virtual	void		HandlePopupMenuSelect(
								Point		inPopupLoc,
								SInt16		inCurrentItem,
								SInt16&		outMenuID,
								SInt16&		outMenuItem);

	virtual	SInt16		GetMenuFontSize () const;

	virtual void		SetMenuValue( SInt16 inItem );

	virtual	void		GetPopupMenuPosition( Point& outPopupLoc );

	bool				HasPopupToRight() const
									{
										return 	mArrowRight;
									}

	bool				HasPopup() const
									{
										return	GetMacMenuH() != nil;
									}

	bool				HasMultiValueMenu	() const
									{
										return	mMultiValueMenu;
									}
	virtual void		DrawSelf();

protected:
	bool				mSmallIcon;
	bool				mShowIcon;
	bool				mShowText;
	bool				mMouseInside;
	bool				mPushed;		// Is mouse down inside control?
	ThemeButtonDrawInfo	mPrevInfo;
	bool				mMultiValueMenu;
	bool				mArrowRight;
	bool				mClickAndPopup;
	bool				mClickOnPopup;

	Boolean		TrackHotSpotButton(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);

	Boolean		TrackHotSpotPopup(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);

	void		DrawFrame();
	void		DrawIcon();
	void		DrawText();
	void		DrawPopupGlyph();
};

#endif
