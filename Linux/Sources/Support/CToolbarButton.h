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


// CToolbarButton.h : header file
//

#ifndef __CTOOLBARBUTTON__MULBERRY__
#define __CTOOLBARBUTTON__MULBERRY__

#include <JXTextButton.h>

/////////////////////////////////////////////////////////////////////////////
// CToolbarButton window

class CToolbarButton : public JXTextButton
{
	friend class CToolbar;

// Construction
public:
	CToolbarButton(const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual ~CToolbarButton();

			void SetImage(ResIDT image, ResIDT imagesel)
				{ mImageID = image; mImageSelID = imagesel; }

	virtual bool HasPopup() const;
	virtual bool GetClickAndPopup() const
		{ return mClickAndPopup; }
	virtual void SetClickAndPopup(bool clickpopup)
		{ mClickAndPopup = clickpopup; }

			void SetSmallIcon(bool small_icon);
			void SetShowIcon(bool icon);
			void SetShowCaption(bool caption);

			void SetDragMode(bool mode)
			{
				mDragMode = mode;
				HandleMouseLeave();
			}

	virtual JBoolean	IsChecked() const
		{ return itsIsCheckedFlag; }
	virtual void		SetState(const JBoolean on);
	virtual void		ToggleState()
		{ SetState(JNegate(itsIsCheckedFlag)); }

protected:
	JBoolean itsIsCheckedFlag;
	ResIDT			mImageID;
	ResIDT			mImageSelID;
	bool mTracking;
	bool mSmallIcon;
	bool mShowIcon;
	bool mShowCaption;
	bool mClickAndPopup;
	bool mDragMode;

	virtual void HandleMouseEnter();
	virtual void HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers);
	virtual void HandleMouseLeave();

	virtual void Draw(JXWindowPainter& p, const JRect& rect);
	virtual void DrawFrame(JXWindowPainter& p, const JRect& rect);
	virtual void DrawContent(JXWindowPainter& p, const JRect& rect);
	virtual void DrawIcon(JXWindowPainter& p, const JRect& rect);
	virtual void DrawTitle(JXWindowPainter& p, const JRect& rect);
	virtual void DrawPopupGlyph(JXWindowPainter& p, const JRect& rect);
};

/////////////////////////////////////////////////////////////////////////////

#endif
