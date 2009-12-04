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


// CLetterHeaderView.h : header file
//

#ifndef __CLETTERHEADERVIEW__MULBERRY__
#define __CLETTERHEADERVIEW__MULBERRY__

#include <JXWidgetSet.h>

#include "HPopupMenu.h"

class CAddressDisplay;
class CCommander;
class CIdentityPopup;
class CMailboxPopup;
class CStaticText;
class CTextInputField;
class CTwister;
class JXFlatRect;
class JXImageWidget;
class JXMultiImageButton;
class JXStaticText;
class JXTextMenu;

/////////////////////////////////////////////////////////////////////////////
// CLetterHeaderView view

class CLetterHeaderView : public JXWidgetSet
{
	friend class CLetterWindow;
	friend class CLetterTextEditView;

// Implementation
protected:
	CLetterHeaderView(JXContainer* enclosure, JXTextMenu* editMenu,
			  const HSizingOption hSizing, 
			  const VSizingOption vSizing,
			  const JCoordinate x, const JCoordinate y,
			  const JCoordinate w, const JCoordinate h);
	virtual ~CLetterHeaderView();

protected:
	void OnCreate(CCommander* cmdr);

private:
// begin JXLayout1

    JXFlatRect*         mHeaderMove;
    JXFlatRect*         mFromMove;
    CIdentityPopup*     mIdentityPopup;
    JXMultiImageButton* mIdentityEditBtn;
    JXFlatRect*         mToMove;
    CTwister*           mToTwister;
    CAddressDisplay*    mToField;
    JXFlatRect*         mCCMove;
    CTwister*           mCCTwister;
    CAddressDisplay*    mCCField;
    JXFlatRect*         mBCCMove;
    CTwister*           mBCCTwister;
    CAddressDisplay*    mBCCField;
    JXFlatRect*         mSubjectMove;
    CTextInputField*    mSubjectField;
    JXImageWidget*      mSentIcon;
    JXFlatRect*         mPartsMove;
    CTwister*           mPartsTwister;
    JXStaticText*       mPartsTitle;
    JXStaticText*       mAttachmentTitle;
    JXStaticText*       mPartsField;
    JXImageWidget*      mAttachments;
    CMailboxPopup*      mCopyTo;

// end JXLayout1
// begin JXLayout2

    JXFlatRect*  mRecipientMove;
    CStaticText* mRecipientText;

// end JXLayout2

  // Style menu
};

/////////////////////////////////////////////////////////////////////////////

#endif
