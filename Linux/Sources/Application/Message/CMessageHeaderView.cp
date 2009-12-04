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


// CMessageHeaderView.cpp : implementation file
//

#include "CMessageHeaderView.h"

#include "CDivider.h"
#include "CIconLoader.h"
#include "CMessageWindow.h"
#include "CPreferences.h"
#include "CSpacebarEdit.h"
#include "CTwister.h"

#include "ETag.h"
#include "TPopupMenu.h"

#include <JXColormap.h>
#include <JXFontManager.h>
#include <JXFlatRect.h>
#include <JXImageButton.h>
#include <JXImageWidget.h>
#include "JXMultiImageButton.h"
#include "JXMultiImageCheckbox.h"
#include <JXStaticText.h>
#include <JXTextMenu.h>
#include <JXUpRect.h>

#include <cassert>

static int cTwistOffset = 32;

static int cFrameSpacing = 24;
static int cFromFrameTop = 8;
static int cToFrameTop = cFromFrameTop + cFrameSpacing;
static int cCCFrameTop = cToFrameTop + cFrameSpacing;
static int cSubjectDateFrameTop = cCCFrameTop + cFrameSpacing ;

/////////////////////////////////////////////////////////////////////////////
// CMessageHeaderView


CMessageHeaderView::CMessageHeaderView
(
 JXContainer* enclosure,
 JXTextMenu* menu,
 const HSizingOption hSizing, const VSizingOption vSizing,
 const JCoordinate x, const JCoordinate y,
 const JCoordinate w, const JCoordinate h
)
 : JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h) ,
   mEditMenu(menu)
{
}

CMessageHeaderView::~CMessageHeaderView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CMessageHeaderView message handlers

void CMessageHeaderView::OnCreate(CCommander* cmdr)
{
// begin JXLayout1

    mFromMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,8, 575,24);
    assert( mFromMove != NULL );

    mFromTwister =
        new CTwister(mFromMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,2, 16,16);
    assert( mFromTwister != NULL );

    mFromTitle =
        new JXStaticText("From:", mFromMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 52,20);
    assert( mFromTitle != NULL );
    mFromTitle->SetFontSize(10);

    mFromField =
        new CSpacebarEdit(mFromMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,0, 475,20);
    assert( mFromField != NULL );

    mToMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,32, 575,24);
    assert( mToMove != NULL );

    mToTwister =
        new CTwister(mToMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,2, 16,16);
    assert( mToTwister != NULL );

    mToTitle =
        new JXStaticText("To:", mToMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 52,20);
    assert( mToTitle != NULL );
    mToTitle->SetFontSize(10);

    mToField =
        new CSpacebarEdit(mToMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,0, 475,20);
    assert( mToField != NULL );

    mCCMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,56, 575,24);
    assert( mCCMove != NULL );

    mCCTwister =
        new CTwister(mCCMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,2, 16,16);
    assert( mCCTwister != NULL );

    mCCTitle =
        new JXStaticText("Cc:", mCCMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 52,20);
    assert( mCCTitle != NULL );
    mCCTitle->SetFontSize(10);

    mCCField =
        new CSpacebarEdit(mCCMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,0, 475,20);
    assert( mCCField != NULL );

    mSubjectDateMove =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,80, 575,66);
    assert( mSubjectDateMove != NULL );

    mSubjectTitle =
        new JXStaticText("Subject:", mSubjectDateMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,0, 66,20);
    assert( mSubjectTitle != NULL );
    mSubjectTitle->SetFontSize(10);

    mSubjectField =
        new CSpacebarEdit(mSubjectDateMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,0, 475,38);
    assert( mSubjectField != NULL );

    mDateTitle =
        new JXStaticText("Date Sent:", mSubjectDateMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,42, 68,20);
    assert( mDateTitle != NULL );
    mDateTitle->SetFontSize(10);

    mDateField =
        new CSpacebarEdit(mSubjectDateMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,42, 315,20);
    assert( mDateField != NULL );

    mNumberField =
        new JXStaticText("Message 100000 of 100000", mSubjectDateMove,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 405,42, 167,20);
    assert( mNumberField != NULL );

    mPartsHeader =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,146, 575,28);
    assert( mPartsHeader != NULL );

    mPartsTitle =
        new JXStaticText("Parts:", mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 16,5, 49,20);
    assert( mPartsTitle != NULL );
    mPartsTitle->SetFontSize(10);

    mPartsTwister =
        new CTwister(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 2,5, 16,16);
    assert( mPartsTwister != NULL );

    mPartsField =
        new JXStaticText("", mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 70,5, 48,20);
    assert( mPartsField != NULL );
    mPartsField->SetFontSize(10);
    const JFontStyle mPartsField_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mPartsField->SetFontStyle(mPartsField_style);

    mAttachments =
        new JXImageWidget(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 122,6, 16,16);
    assert( mAttachments != NULL );

    mAlternative =
        new JXImageWidget(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 122,6, 16,16);
    assert( mAlternative != NULL );

    mFlatHierarchyBtn =
        new JXMultiImageCheckbox(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 69,6, 16,16);
    assert( mFlatHierarchyBtn != NULL );

    CDivider* obj1 =
        new CDivider(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 145,1, 2,24);
    assert( obj1 != NULL );

    mHeaderBtn =
        new JXMultiImageCheckbox(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 154,2, 24,22);
    assert( mHeaderBtn != NULL );

    mTextFormat =
        new HPopupMenu("",mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 185,3, 130,20);
    assert( mTextFormat != NULL );

    CDivider* obj2 =
        new CDivider(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 318,1, 2,24);
    assert( obj2 != NULL );

    mFontIncreaseBtn =
        new JXMultiImageButton(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 330,2, 24,22);
    assert( mFontIncreaseBtn != NULL );

    mFontDecreaseBtn =
        new JXMultiImageButton(mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 354,2, 24,22);
    assert( mFontDecreaseBtn != NULL );

    mFontScaleField =
        new JXStaticText("", mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 380,5, 25,20);
    assert( mFontScaleField != NULL );
    mFontScaleField->SetFontSize(10);

    mQuoteDepthPopup =
        new HPopupMenu("",mPartsHeader,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 415,3, 130,20);
    assert( mQuoteDepthPopup != NULL );

// end JXLayout1

	// Must call these to get context menus
	mFromField->OnCreate();
	mToField->OnCreate();
	mCCField->OnCreate();
	mSubjectField->OnCreate();
	mDateField->OnCreate();
	mDateField->SetBreakCROnly(kTrue);

	// Initialise to the standard text font
	mSubjectField->ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), 0);

	// Null these out to prevent captions flash when first opened
	mNumberField->SetText(cdstring::null_str);

	mAttachments->SetImage(CIconLoader::GetIcon(IDI_MSG_ATTACHMENT, mAttachments, 16, 0x00CCCCCC), kFalse);
	mAlternative->SetImage(CIconLoader::GetIcon(IDI_MSG_ALTERNATIVE, mAlternative, 16, 0x00CCCCCC), kFalse);
	mFlatHierarchyBtn->SetImages(IDI_SERVERFLAT, 0, IDI_SERVERHIERARCHIC, 0);
	mFlatHierarchyBtn->Hide();
	mHeaderBtn->SetImage(IDI_HEADERMSG);
	mFontIncreaseBtn->SetImage(IDI_FONTINCREASE);
	mFontDecreaseBtn->SetImage(IDI_FONTDECREASE);

	// Format Menu
	mTextFormat->SetMenuItems("Formatted %r | Plain Text %r | Source %r%l | as HTML %r | as Enriched %r | With Fixed Font %r%l | Raw Message %r");
	mTextFormat->SetUpdateAction(JXMenu::kDisableNone);
	mTextFormat->SetToPopupChoice(kTrue, eViewFormatted);

	// Quote Depth Menu
	mQuoteDepthPopup->SetMenuItems("All Quotes %r | No Quotes %r%l | > 1 Level %r | >> 2 Levels%r | >>> 3 Levels%r | >>>> 4 Levels%r");
	mQuoteDepthPopup->SetUpdateAction(JXMenu::kDisableNone);
	mQuoteDepthPopup->SetToPopupChoice(kTrue, eQuoteDepth_All);

	return;
}
