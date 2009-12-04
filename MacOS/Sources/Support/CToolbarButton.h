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


// Header for CToolbarButton class

#ifndef __CTOOLBARBUTTON__MULBERRY__
#define __CTOOLBARBUTTON__MULBERRY__

#include <LBevelButton.h>

// Constants

// Classes
class CToolbarItem;

class CToolbarButton : public LBevelButton
{
public:
	enum { class_ID = 'TBut',
			imp_class_ID = 'TBit' };
	
						CToolbarButton(LStream *inStream);
						CToolbarButton(
								const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								SInt16			inBevelProc,
								SInt16			inBehavior,
								SInt16			inContentType,
								SInt16			inContentResID,
								ResIDT			inTextTraits,
								ConstStringPtr	inTitle,
								SInt16			inInitialValue,
								SInt16			inTitlePlacement,
								SInt16			inTitleAlignment,
								SInt16			inTitleOffset,
								SInt16			inGraphicAlignment,
								Point			inGraphicOffset);

						CToolbarButton(
								const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								SInt16			inBevelProc,
								ResIDT			inMenuID,
								SInt16			inMenuPlacement,
								SInt16			inContentType,
								SInt16			inContentResID,
								ResIDT			inTextTraits,
								ConstStringPtr	inTitle,
								SInt16			inInitialValue,
								SInt16			inTitlePlacement,
								SInt16			inTitleAlignment,
								SInt16			inTitleOffset,
								SInt16			inGraphicAlignment,
								Point			inGraphicOffset,
								Boolean			inCenterPopupGlyph);
	virtual				~CToolbarButton();

			void		SetDetails(const CToolbarItem*	item);

	virtual	void		SetValue(SInt32 inValue);		// Prevent crash when deleting
	virtual	void		SetMenuValue(SInt32 inValue, bool silent = false);
	virtual void		SetDescriptor(ConstStringPtr inDescriptor);

	virtual void		MouseEnter(Point inPortPt, const EventRecord&	inMacEvent);
	virtual void		MouseLeave();

	void SetSmallIcon(bool small_icon);
	void SetShowIcon(bool icon);
	void SetShowCaption(bool caption);

	bool GetIconToggle() const
		{ return mIconToggle; }
	void SetIconToggle(bool toggle)
		{ mIconToggle = toggle; }

	void SetDragMode(bool mode)
	{
		mDragMode = mode;
		MouseLeave();
	}

	bool HasPopup() const
		{ return GetMacMenuH() != NULL; }
	bool GetClickAndPopup() const;
	void SetClickAndPopup(bool clickpopup);

	virtual Boolean		TrackHotSpot(
								SInt16			inHotSpot,
								Point			inPoint,
								SInt16			inModifiers);
	
protected:
	bool 	mSmallIcon;
	bool 	mShowIcon;
	bool 	mShowCaption;
	bool 	mIconToggle;
	bool	mHasTooltip;
	bool	mDragMode;
	bool	mValueMenu;

	virtual void DeactivateSelf();
	virtual void ClickSelf(const SMouseDownEvent& inMouseDown);

	virtual void		HotSpotResult( SInt16 inHotSpot );
};

#endif
