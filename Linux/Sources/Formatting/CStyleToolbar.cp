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


#include "CStyleToolbar.h"

#include "CColorPopup.h"
#include "CCommands.h"
#include "CEditFormattedTextDisplay.h"
#include "CFontNameMenu.h"
#include "CFontSizeMenu.h"
#include "CPreferences.h"
#include "TPopupMenu.h"

#include "HResourceMap.h"

#include "JXMultiImageCheckbox.h"

#include <cassert>

CStyleToolbar::CStyleToolbar(
		JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h)
 : JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mInited = false;
	mType = eContentSubPlain;
}

CStyleToolbar::~CStyleToolbar()
{
}

void CStyleToolbar::OnCreate(JXWindowDirector* director)
{
// begin JXLayout1

    mBold =
        new JXMultiImageCheckbox(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,2, 24,22);
    assert( mBold != NULL );

    mItalic =
        new JXMultiImageCheckbox(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 30,2, 24,22);
    assert( mItalic != NULL );

    mUnderline =
        new JXMultiImageCheckbox(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 55,2, 24,22);
    assert( mUnderline != NULL );

    mAlignLeft =
        new JXMultiImageCheckbox(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 431,2, 24,22);
    assert( mAlignLeft != NULL );

    mAlignCenter =
        new JXMultiImageCheckbox(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 456,2, 24,22);
    assert( mAlignCenter != NULL );

    mAlignRight =
        new JXMultiImageCheckbox(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 481,2, 24,22);
    assert( mAlignRight != NULL );

    mFont =
        new CFontNamePopup("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 100,2, 160,22);
    assert( mFont != NULL );

    mSize =
        new CFontSizePopup("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 270,2, 60,22);
    assert( mSize != NULL );

    mColor =
        new CColorPopup("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 345,2, 60,22);
    assert( mColor != NULL );

// end JXLayout1

	mBold->SetImage(IDI_BOLDBTN);
	mItalic->SetImage(IDI_ITALICBTN);
	mUnderline->SetImage(IDI_UNDERLINEBTN);
	mAlignLeft->SetImage(IDI_ALIGNLEFTBTN);
	mAlignCenter->SetImage(IDI_ALIGNCENTERBTN);
	mAlignRight->SetImage(IDI_ALIGNRIGHTBTN);

	mFont->OnCreate();
	mFont->SetFontName(CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().fontname);
    mFont->CFontNameMenu::SetToPopupChoice();

	mSize->OnCreate(mFont);
	mSize->SetFontSize(CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().size);
    mSize->CFontSizeMenu::SetToPopupChoice();

	mColor->OnCreate(director);
	mColor->SetToPopupChoice(kTrue, 1);

	mBold->SetHint(stringFromResource(IDC_LETTER_ENR_BOLD));
	mItalic->SetHint(stringFromResource(IDC_LETTER_ENR_ITALIC));
	mUnderline->SetHint(stringFromResource(IDC_LETTER_ENR_UNDERLINE));
	mFont->SetHint(stringFromResource(IDC_LETTER_ENR_FONT));
	mSize->SetHint(stringFromResource(IDC_LETTER_ENR_SIZE));
	mColor->SetHint(stringFromResource(IDC_LETTER_ENR_COLOR));
	mAlignLeft->SetHint(stringFromResource(IDC_LETTER_ENR_LEFT));
	mAlignCenter->SetHint(stringFromResource(IDC_LETTER_ENR_CENTER));
	mAlignRight->SetHint(stringFromResource(IDC_LETTER_ENR_RIGHT));
}


bool CStyleToolbar::HasCommand(unsigned long inCommand) const
{
	switch(inCommand)
	{
	// All styles
	case CCommand::eStyleBold:
	case CCommand::eStyleItalic:
	case CCommand::eStyleUnderline:
	case CCommand::eAlignLeft:
	case CCommand::eAlignCenter:
	case CCommand::eColourRed:
	case CCommand::eColourBlue:
	case CCommand::eColourGreen:
	case CCommand::eColourYellow:
	case CCommand::eColourCyan:
	case CCommand::eColourMagenta:
	case CCommand::eColourBlack:
	case CCommand::eColourWhite:
	case CCommand::eColourMulberry:
	case CCommand::eColourOther:
		return (mType == eContentSubEnriched) || (mType == eContentSubHTML);
	
	// Enriched only
	case CCommand::eAlignRight:
		return mType == eContentSubEnriched;
	
	default:
		return false;
	}
}

void CStyleToolbar::SetHiliteState(unsigned long inCommand, bool state)
{
	switch(inCommand)
	{
	case CCommand::eStyleBold:
		mBold->SetState(JBoolean(state));
		break;
	case CCommand::eStyleItalic:
		mItalic->SetState(JBoolean(state));
		break;
	case CCommand::eStyleUnderline:
		mUnderline->SetState(JBoolean(state));
		break;
	case CCommand::eAlignLeft:
		mAlignLeft->SetState(JBoolean(state));
		break;
	case CCommand::eAlignCenter:
		mAlignCenter->SetState(JBoolean(state));
		break;
	case CCommand::eAlignRight:
		mAlignRight->SetState(JBoolean(state));
		break;
	}
}

void CStyleToolbar::Ambiguate(unsigned long inCommand)
{
	switch(inCommand)
	{
	case CCommand::eDraftColour:
		//mColor->Ambiguate();
		break;
	case CCommand::eDraftFont:
		//mFont->Ambiguate();
		break;
	case CCommand::eDraftSize:
		//mSize->Ambiguate();
		break;
	}
}

void CStyleToolbar::AddListener(CEditFormattedTextDisplay* inListener)
{
	inListener->StyledToolbarListen(mBold);
	inListener->StyledToolbarListen(mItalic);
	inListener->StyledToolbarListen(mUnderline);
	inListener->StyledToolbarListen(mAlignLeft);
	inListener->StyledToolbarListen(mAlignCenter);
	inListener->StyledToolbarListen(mAlignRight);
	inListener->StyledToolbarListen(mFont);
	inListener->StyledToolbarListen(mSize);
	inListener->StyledToolbarListen(mColor);
}

void CStyleToolbar::GetFont(cdstring& name)
{
	name = mFont->GetFontName();
}

JSize CStyleToolbar::GetSize()
{
	return mSize->GetFontSize();
}

void CStyleToolbar::SelectColor(JIndex color)
{
	mColor->SelectColor(color);
}

void CStyleToolbar::SetColor(const RGBColor& color)
{
	mColor->SetColor(color);
}

RGBColor CStyleToolbar::GetColor()
{
	return mColor->GetColor();
}


void CStyleToolbar::SetFontName(const cdstring& fontName)
{
	mFont->SetFontName(fontName);
}

void CStyleToolbar::SetSizeItem(JIndex menuItem)
{
	//mSize->SetValue(menuItem);
}

void CStyleToolbar::SetSize(JSize size)
{
	mSize->SetFontSize(size);
}

void CStyleToolbar::Setup(EContentSubType type)
{
	switch(type)
	{
	case eContentSubEnriched:
	case eContentSubHTML:
		if (!mInited)
		{
			mFont->BuildMenuNow(true);		// Quick screen font only build of menu
			mInited = true;
		}
		break;
	default:;
	}

	switch(type)
	{
	case eContentSubEnriched:
		mAlignRight->Show();
		break;
	case eContentSubHTML:
		mAlignRight->Hide();
		break;
	default:;
	}
	mType = type;
}
