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


// CLetterHeaderView.cpp : implementation file
//

#include "CLetterHeaderView.h"

#include "CAddressDisplay.h"
#include "CDivider.h"
#include "CIconLoader.h"
#include "CIdentityPopup.h"
#include "CMailboxPopup.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextField.h"
#include "CTwister.h"

#include <JXColormap.h>
#include <JXFlatRect.h>
#include <JXImageWidget.h>
#include "JXMultiImageButton.h"
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXWindow.h>

#include <cassert>

static int cTwistOffset = 32;

static int cFrameSpacing = 24;
static int cToFrameTop = 8;
static int cCCFrameTop = cToFrameTop + cFrameSpacing;
static int cBCCFrameTop = cCCFrameTop + cFrameSpacing;
static int cSubjectFrameTop = cBCCFrameTop + cFrameSpacing ;

/////////////////////////////////////////////////////////////////////////////
// CLetterHeaderView


CLetterHeaderView::CLetterHeaderView
(
 JXContainer* enclosure,
 JXTextMenu* menu,
 const HSizingOption hSizing, const VSizingOption vSizing,
 const JCoordinate x, const JCoordinate y,
 const JCoordinate w, const JCoordinate h
)
 : JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
}

CLetterHeaderView::~CLetterHeaderView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CLetterHeaderView message handlers

void CLetterHeaderView::OnCreate(CCommander* cmdr)
{
	{
// begin JXLayout1

    mHeaderMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 575,24);
    assert( mHeaderMove != NULL );

    mFromMove =
        new JXFlatRect(mHeaderMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 574,24);
    assert( mFromMove != NULL );

    JXStaticText* obj1 =
        new JXStaticText("From:", mFromMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,4, 50,20);
    assert( obj1 != NULL );
    obj1->SetFontSize(10);

    mIdentityPopup =
        new CIdentityPopup("",mFromMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 80,2, 460,20);
    assert( mIdentityPopup != NULL );

    mIdentityEditBtn =
        new JXMultiImageButton(mFromMove,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 544,1, 24,22);
    assert( mIdentityEditBtn != NULL );

    mToMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,24, 575,24);
    assert( mToMove != NULL );

    mToTwister =
        new CTwister(mToMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,2, 16,16);
    assert( mToTwister != NULL );

    JXStaticText* obj2 =
        new JXStaticText("To:", mToMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 36,20);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);

    mToField =
        new CAddressDisplay(mToMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,0, 460,20);
    assert( mToField != NULL );

    mCCMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,48, 575,24);
    assert( mCCMove != NULL );

    mCCTwister =
        new CTwister(mCCMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,2, 16,16);
    assert( mCCTwister != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Cc:", mCCMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 36,20);
    assert( obj3 != NULL );
    obj3->SetFontSize(10);

    mCCField =
        new CAddressDisplay(mCCMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,0, 460,20);
    assert( mCCField != NULL );

    mBCCMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,72, 575,24);
    assert( mBCCMove != NULL );

    mBCCTwister =
        new CTwister(mBCCMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,2, 16,16);
    assert( mBCCTwister != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Bcc:", mBCCMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 36,20);
    assert( obj4 != NULL );
    obj4->SetFontSize(10);

    mBCCField =
        new CAddressDisplay(mBCCMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,0, 460,20);
    assert( mBCCField != NULL );

    mSubjectMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,96, 575,24);
    assert( mSubjectMove != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Subject:", mSubjectMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 56,20);
    assert( obj5 != NULL );
    obj5->SetFontSize(10);

    mSubjectField =
        new CTextInputField(mSubjectMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 80,0, 460,20);
    assert( mSubjectField != NULL );

    mSentIcon =
        new JXImageWidget(mSubjectMove,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 546,2, 16,16);
    assert( mSentIcon != NULL );

    mPartsMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,120, 575,28);
    assert( mPartsMove != NULL );

    mPartsTwister =
        new CTwister(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,6, 16,16);
    assert( mPartsTwister != NULL );

    mPartsTitle =
        new JXStaticText("Parts:", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,6, 54,20);
    assert( mPartsTitle != NULL );
    mPartsTitle->SetFontSize(10);

    mAttachmentTitle =
        new JXStaticText("Attached:", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,6, 60,20);
    assert( mAttachmentTitle != NULL );
    mAttachmentTitle->SetFontSize(10);

    mPartsField =
        new JXStaticText("", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 80,6, 48,20);
    assert( mPartsField != NULL );
    mPartsField->SetFontSize(10);
    const JFontStyle mPartsField_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mPartsField->SetFontStyle(mPartsField_style);

    mAttachments =
        new JXImageWidget(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 132,7, 16,16);
    assert( mAttachments != NULL );

    CDivider* obj6 =
        new CDivider(mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 150,2, 2,24);
    assert( obj6 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Copy To:", mPartsMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 156,6, 60,20);
    assert( obj7 != NULL );
    obj7->SetFontSize(10);

    mCopyTo =
        new CMailboxPopup(false,mPartsMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 214,4, 325,20);
    assert( mCopyTo != NULL );

// end JXLayout1
	}
	{
// begin JXLayout2

    mRecipientMove =
        new JXFlatRect(mHeaderMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 575,24);
    assert( mRecipientMove != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Recipient:", mRecipientMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,4, 63,20);
    assert( obj1 != NULL );
    obj1->SetFontSize(10);

    mRecipientText =
        new CStaticText("", mRecipientMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 80,4, 460,20);
    assert( mRecipientText != NULL );

// end JXLayout2
	}

	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue(), true);
	mIdentityPopup->SetToPopupChoice(kTrue, eIdentityPopup_FirstWithCustom);
	mIdentityEditBtn->SetImage(IDI_IDENTITYEDIT);
	mIdentityEditBtn->Hide();
	
	mRecipientMove->Hide();

	// Must call these to get context menus
	mToField->OnCreate();
	mCCField->OnCreate();
	mBCCField->OnCreate();
	mSubjectField->OnCreate();

	// Initialise to the standard text font
	mSubjectField->ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), 0);

	// Need to update title for every character typed in subject field
	mSubjectField->ShouldBroadcastAllTextChanged(kTrue);

	// Set twisters
	mToField->SetTwister(mToTwister);
	mCCField->SetTwister(mCCTwister);
	mBCCField->SetTwister(mBCCTwister);

	mCopyTo->SetUpdateAction(JXMenu::kDisableNone);
	mCopyTo->SetValue(CCopyToMenu::cPopupAppendNone);
	mCopyTo->SetDefault();

	mSentIcon->SetImage(CIconLoader::GetIcon(IDI_FLAG_UNSEEN, this, 16, 0x00CCCCCC), kFalse);
	mSentIcon->Hide();

	mAttachmentTitle->Hide();
	mAttachments->SetImage(CIconLoader::GetIcon(IDI_MSG_ATTACHMENT, this, 16, 0x00CCCCCC), kFalse);

}
