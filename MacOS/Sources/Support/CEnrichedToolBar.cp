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


#include "CEnrichedToolBar.h"
#include "LGATextButton.h"
#include "PP_Messages.h"
#include "LGAIconButton.h"
#include "CFontPopup.h"
#include "CSizePopup.h"
#include "CColorPopup.h"
#include "LGAPopup.h"
#include "LView.h"
#include "CEditFormattedTextDisplay.h"
#include <LGAIconButton.h>
#include "CPreferences.h"


CEnrichedToolBar::CEnrichedToolBar() : CBackView(){
	mBold = NULL;
	mItalic = NULL;
	mUnderline = NULL;
	here = false;
}

CEnrichedToolBar::CEnrichedToolBar(LStream *inStream) : CBackView(inStream){
	mBold = NULL;
	mItalic = NULL;
	mUnderline = NULL;
	here = false;
}

CEnrichedToolBar::~CEnrichedToolBar(){
}

CEnrichedToolBar* CEnrichedToolBar::CreateFromStream(LStream *inStream)
{
	return (new CEnrichedToolBar(inStream));
}

void CEnrichedToolBar::FinishCreateSelf(){
	CBackView::FinishCreateSelf();
	
	mBold = (LGATextButton *)FindPaneByID(paneid_BoldBTN);
	mBold->Disable();
	mItalic = (LGATextButton *)FindPaneByID(paneid_ItalicBTN);
	mItalic->Disable();
	mUnderline = (LGATextButton *)FindPaneByID(paneid_UnderlineBTN);
	mUnderline->Disable();
	mAlignLeft = (LGAIconButton *)FindPaneByID(paneid_AlignLeftBTN);
	mAlignLeft->Disable();
	mAlignCenter = (LGAIconButton *)FindPaneByID(paneid_AlignCenterBTN);
	mAlignCenter->Disable();
	mAlignRight = (LGAIconButton *)FindPaneByID(paneid_AlignRightBTN);
	mAlignRight->Disable();
	mAlignJustify = (LGAIconButton *)FindPaneByID(paneid_AlignJustifyBTN);
	mAlignJustify->Disable();
	mFont = (CFontPopup *)FindPaneByID(paneid_EnrichedFont);
	mFont->SetFontName(CPreferences::sPrefs->mDisplayTextTraits.fontName);
	mSize = (CSizePopup *)FindPaneByID(paneid_EnrichedSize);
	//mSize->SetSize(CPreferences::sPrefs->mBaseSize);
	mSize->SetSize(10);

	mColor = (CColorPopup *)FindPaneByID(paneid_EnrichedColor);
	mColor->Enable();
	mColor->SetValue(1);
}


void CEnrichedToolBar::Show(){
	here = true;
	CBackView::Show();
}

void CEnrichedToolBar::Hide(){
	here = false;
	CBackView::Hide();
}


void CEnrichedToolBar::EnableCmd(CommandT inCommand){
	switch(inCommand){
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


void CEnrichedToolBar::SetHiliteState(CommandT inCommand, Boolean state){
	switch(inCommand){
		case cmd_Bold:
			mBold->SetHiliteState(state);
			break;
		case cmd_Italic:
			mItalic->SetHiliteState(state);
			break;
		case cmd_Underline:
			mUnderline->SetHiliteState(state);
			break;
		case cmd_JustifyLeft:
			mAlignLeft->SetHiliteState(state);
			break;
		case cmd_JustifyCenter:
			mAlignCenter->SetHiliteState(state);
			break;
		case cmd_JustifyRight:
			mAlignRight->SetHiliteState(state);
			break;
		case cmd_JustifyFull:
			mAlignJustify->SetHiliteState(state);
			break;
			
	}
}


void CEnrichedToolBar::Ambiguate(CommandT inCommand){
	switch(inCommand){
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

void CEnrichedToolBar::AddListener(LListener *inListener){
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

void CEnrichedToolBar::getFont(Str255 name){
	mFont->GetFontName(name);
}

Int16 CEnrichedToolBar::getSize(){
	return mSize->GetSize();
}

void CEnrichedToolBar::selectColor(Int32 color){
	mColor->selectColor(color);
	
}

void CEnrichedToolBar::setColor(RGBColor color){
	mColor->setColor(color);
}

RGBColor CEnrichedToolBar::getColor(){
	return mColor->getColor();
}


void CEnrichedToolBar::setFontName(Str255 fontName){
	mFont->SetFontName(fontName);
}

void CEnrichedToolBar::setFont(Int16 menuItem){
	mFont->SetValue(menuItem);
}

void CEnrichedToolBar::setSizeItem(Int16 menuItem){
	mSize->SetValue(menuItem);
}

void CEnrichedToolBar::setSize(Int16 size){
	mSize->SetSize(size);
}

void CEnrichedToolBar::SaveState(){
	mFont->SaveState();
	mSize->SaveState();
}

void CEnrichedToolBar::RestoreState(){
	mFont->RestoreState();
	mSize->RestoreState();
}