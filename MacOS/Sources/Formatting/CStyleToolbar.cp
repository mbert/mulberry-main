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

#include "CCommands.h"
#include "CFontPopup.h"
#include "CSizePopup.h"
#include "CColorPopup.h"
#include "CPreferences.h"

#include "PP_Messages.h"

#include <LBevelButton.h>


CStyleToolbar::CStyleToolbar(LStream *inStream) : LPlacard(inStream)
{
	mBold = NULL;
	mItalic = NULL;
	mUnderline = NULL;
	here = false;
}

CStyleToolbar::~CStyleToolbar()
{
}

void CStyleToolbar::FinishCreateSelf()
{
	LPlacard::FinishCreateSelf();

	mBold = (LBevelButton *)FindPaneByID(paneid_BoldBTN);
	mBold->Disable();
	mItalic = (LBevelButton *)FindPaneByID(paneid_ItalicBTN);
	mItalic->Disable();
	mUnderline = (LBevelButton *)FindPaneByID(paneid_UnderlineBTN);
	mUnderline->Disable();
	mAlignLeft = (LBevelButton *)FindPaneByID(paneid_AlignLeftBTN);
	mAlignLeft->Disable();
	mAlignCenter = (LBevelButton *)FindPaneByID(paneid_AlignCenterBTN);
	mAlignCenter->Disable();
	mAlignRight = (LBevelButton *)FindPaneByID(paneid_AlignRightBTN);
	mAlignRight->Disable();
	mAlignJustify = (LBevelButton *)FindPaneByID(paneid_AlignJustifyBTN);
	mAlignJustify->Disable();
	mFont = (CFontPopup *)FindPaneByID(paneid_EnrichedFont);
	mFont->SetFontName(CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.fontName);
	mSize = (CSizePopup *)FindPaneByID(paneid_EnrichedSize);
	mSize->SetSize(CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.size);

	mColor = (CColorPopup *)FindPaneByID(paneid_EnrichedColor);
	mColor->Enable();
	mColor->SetValue(1);
}


void CStyleToolbar::Show()
{
	here = true;
	LPlacard::Show();
}

void CStyleToolbar::Hide()
{
	here = false;
	LPlacard::Hide();
}


void CStyleToolbar::EnableCmd(CommandT inCommand)
{
	switch(inCommand)
	{
	case cmd_Bold:
		mBold->Enable();
		break;
	case cmd_Italic:
		mItalic->Enable();
		break;
	case cmd_Underline:
		mUnderline->Enable();
		break;
	case cmd_JustifyLeft:
		mAlignLeft->Enable();
		break;
	case cmd_JustifyCenter:
		mAlignCenter->Enable();
		break;
	case cmd_JustifyRight:
		mAlignRight->Enable();
		break;
	case cmd_JustifyFull:
		mAlignJustify->Enable();
		break;
	case cmd_Font:
		mFont->Enable();
		break;
	case cmd_Size:
		mSize->Enable();
		break;
	}
}


void CStyleToolbar::SetHiliteState(CommandT inCommand, Boolean state)
{
	switch(inCommand)
	{
	case cmd_Bold:
		mBold->SetValue(state);
		break;
	case cmd_Italic:
		mItalic->SetValue(state);
		break;
	case cmd_Underline:
		mUnderline->SetValue(state);
		break;
	case cmd_JustifyLeft:
		mAlignLeft->SetValue(state);
		break;
	case cmd_JustifyCenter:
		mAlignCenter->SetValue(state);
		break;
	case cmd_JustifyRight:
		mAlignRight->SetValue(state);
		break;
	case cmd_JustifyFull:
		mAlignJustify->SetValue(state);
		break;
	}
}


void CStyleToolbar::Ambiguate(CommandT inCommand)
{
	switch(inCommand)
	{
	case cmd_Color:
		mColor->Ambiguate();
		break;
	case cmd_Size:
		mSize->Ambiguate();
		break;
	case cmd_Font:
		mFont->Ambiguate();
		break;
	}
}

void CStyleToolbar::AddListener(LListener *inListener)
{
	mBold->AddListener(inListener);
	mItalic->AddListener(inListener);
	mUnderline->AddListener(inListener);
	mAlignLeft->AddListener(inListener);
	mAlignCenter->AddListener(inListener);
	mAlignRight->AddListener(inListener);
	mAlignJustify->AddListener(inListener);
	mFont->AddListener(inListener);
	mSize->AddListener(inListener);
	mColor->AddListener(inListener);
}

void CStyleToolbar::getFont(Str255 name)
{
	mFont->GetFontName(name);
}

SInt16 CStyleToolbar::getSize()
{
	return mSize->GetSize();
}

void CStyleToolbar::selectColor(SInt32 color)
{
	mColor->selectColor(color);
}

void CStyleToolbar::setColor(RGBColor color)
{
	mColor->setColor(color);
}

RGBColor CStyleToolbar::getColor()
{
	return mColor->getColor();
}


void CStyleToolbar::setFontName(Str255 fontName)
{
	mFont->SetFontName(fontName);
}

void CStyleToolbar::setFont(SInt16 menuItem)
{
	mFont->SetValue(menuItem);
}

void CStyleToolbar::setSizeItem(SInt16 menuItem)
{
	mSize->SetValue(menuItem);
}

void CStyleToolbar::setSize(SInt16 size)
{
	mSize->SetSize(size);
}

void CStyleToolbar::SaveState()
{
	mFont->SaveState();
	mSize->SaveState();
}

void CStyleToolbar::RestoreState()
{
	mFont->RestoreState();
	mSize->RestoreState();
}

void CStyleToolbar::Setup(EContentSubType type)
{
	switch(type)
	{
	case eContentSubEnriched:
		mAlignRight->Show();
		mAlignJustify->Show();
		break;
	case eContentSubHTML:
		mAlignRight->Hide();
		mAlignJustify->Hide();
		break;
	default:;
	}
}
