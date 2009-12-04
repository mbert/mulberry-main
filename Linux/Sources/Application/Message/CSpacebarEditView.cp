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


// CSpacebarEditView.cpp : implementation file
//


#include "CSpacebarEditView.h"

#include "CAttachment.h"
#include "CCaptionParser.h"
#include "CMainMenu.h"
#include "CMessageView.h"
#include "CMessageWindow.h"
#include "CPreferences.h"
#include "StPenState.h"

#include <jASCIIConstants.h>
#include <JXScrollbar.h>
#include <jXKeysym.h>
#include <JPagePrinter.h>

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEditView

CSpacebarEditView::CSpacebarEditView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
		: CAddressText(enclosure, hSizing,  vSizing, x, y, w, h)
{
	mMsgView = NULL;
	mMsgWindow = NULL;
}

CSpacebarEditView::~CSpacebarEditView()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSpacebarEditView message handlers

void CSpacebarEditView::OnCreate()
{
	// Do this here to override default context menu
	CreateContextMenu(CMainMenu::eContextMessageText);

	CAddressText::OnCreate();
}

// Handle key presses
bool CSpacebarEditView::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	if (IsReadOnly())
	{
		switch (key)
		{
		// Scroll down then to next message
		case ' ':
			{
				// Check whether scrolled to the bottom or no scrollbar
				JXScrollbar* horiz;
				JXScrollbar* vert;
				GetScrollbars(&horiz, &vert);
				if (!vert->IsVisible() || vert->IsAtMax())
					{
						// Control key => delete and read next
						if (modifiers.control())
						{
							if (mMsgWindow)
								mMsgWindow->OnMessageDeleteRead();
							else if (mMsgView)
								mMsgView->OnMessageDeleteRead();
						}
						else if (modifiers.shift())
						{
							if (mMsgWindow)
								mMsgWindow->OnMessageReadPrev();
							else if (mMsgView)
								mMsgView->OnMessageReadPrev();
						}
						else
						{
							if (mMsgWindow)
								mMsgWindow->OnMessageReadNext();
							else if (mMsgView)
								mMsgView->OnMessageReadNext();
						}
					}
				else
					// Do page down
					CAddressText::HandleChar(XK_Page_Down, modifiers);
			}
			return true;
			
		// Select next new in preview
		case '\t':
			// Only do in a preview pane
			if (mMsgView)
			{
				mMsgView->OnMessageReadNextNew();
				return true;
			}
			return false;

		// Delete and next
		case kJDeleteKey:
		case kJForwardDeleteKey:
			if (mMsgWindow)
				mMsgWindow->OnMessageDeleteRead();
			else if (mMsgView)
				mMsgView->OnMessageDeleteRead();
			return true;

		default:;
		}
	}
	
	return CAddressText::HandleChar(key, modifiers);
}

CMessage* CSpacebarEditView::GetMessage() const
{
	// Just get message from message window
	return mMsgWindow ? mMsgWindow->GetMessage() : (mMsgView ? mMsgView->GetMessage() : NULL);
}

void CSpacebarEditView::CleanMessage()
{
	// Does nothing as message belongs to message window
}

bool CSpacebarEditView::ChangePrintFont() const
{
	bool change_font = false;
	if (mMsgWindow)
	{
		bool use_part = (mMsgWindow->GetCurrentPart() != NULL) && (mMsgWindow->GetViewAs() != eViewAsRaw);
		change_font = use_part &&
							(mMsgWindow->GetCurrentPart()->GetContent().GetContentSubtype() == eContentSubPlain);
	}
	else if (mMsgView)
	{
		bool use_part = (mMsgView->GetCurrentPart() != NULL) && (mMsgView->GetViewAs() != eViewAsRaw);
		change_font = use_part &&
							(mMsgView->GetCurrentPart()->GetContent().GetContentSubtype() == eContentSubPlain);
	}

	return change_font;
}

// Temporarily add header summary for printing
void CSpacebarEditView::AddPrintSummary()
{
	// Check for summary headers
	if (mMsgWindow)
		mMsgWindow->AddPrintSummary();
	else
		mMsgView->AddPrintSummary();
}

// Remove temp header summary after printing
void CSpacebarEditView::RemovePrintSummary()
{
	// Check for summary headers
	if (mMsgWindow)
		mMsgWindow->RemovePrintSummary();
	else
		mMsgView->RemovePrintSummary();
}

bool CSpacebarEditView::MessagePrint() const
{
	return true;
}

// Printing

const int cCaptionVertMargin = 16;
const int cBoxBottomMargin = 4;

JCoordinate CSpacebarEditView::GetPrintHeaderHeight(JPagePrinter& p) const
{
	// Count lines in header
	cdstring headTxt = CCaptionParser::ParseCaption(MessagePrint() ?
														CPreferences::sPrefs->mHeaderCaption.GetValue() :
														CPreferences::sPrefs->mLtrHeaderCaption.GetValue(), GetMessage(), false, 100);
	return GetCaptionHeight(p, headTxt);
}

JCoordinate CSpacebarEditView::GetPrintFooterHeight(JPagePrinter& p) const
{
	// Count lines in footer
	cdstring footTxt = CCaptionParser::ParseCaption(MessagePrint() ?
														CPreferences::sPrefs->mFooterCaption.GetValue() :
														CPreferences::sPrefs->mLtrFooterCaption.GetValue(), GetMessage(), false, 100);
	return GetCaptionHeight(p, footTxt);
}

JCoordinate CSpacebarEditView::GetCaptionHeight(JPagePrinter& p, const cdstring& caption) const
{
	// Get caption font line height
	JFontStyle style;
	JSize line_height = p.GetLineHeight(CPreferences::sPrefs->mCaptionTextFontInfo.GetValue().fontname,
										CPreferences::sPrefs->mCaptionTextFontInfo.GetValue().size, style);

	// Count lines in caption
	unsigned long ctr = 0;
	const char* txt = caption.c_str();
	while(*txt)
	{
		if ((*txt == '\n') && *(txt+1))
			ctr++;
		txt++;
	}

	// Adjust for line without a trailing LF
	if (*--txt != '\n')
		ctr++;

	return ctr * line_height + cCaptionVertMargin;
}

void CSpacebarEditView::DrawPrintHeader(JPagePrinter& p, const JCoordinate headerHeight)
{
	// Get text
	cdstring headTxt = CCaptionParser::ParseCaption(MessagePrint() ?
														CPreferences::sPrefs->mHeaderCaption.GetValue() :
														CPreferences::sPrefs->mLtrHeaderCaption.GetValue(), GetMessage(), false, p.GetPageIndex());

	// Get area to draw into
	JRect drawable = p.GetPageRect();
	drawable.bottom = drawable.top + headerHeight - cCaptionVertMargin + cBoxBottomMargin;

	// Draw it
	DrawCaption(p, drawable, headTxt, MessagePrint() ?
										CPreferences::sPrefs->mHeaderBox.GetValue() :
										CPreferences::sPrefs->mFooterBox.GetValue());
}

void CSpacebarEditView::DrawPrintFooter(JPagePrinter& p, const JCoordinate footerHeight)
{
	// Get text
	cdstring footTxt = CCaptionParser::ParseCaption(MessagePrint() ?
														CPreferences::sPrefs->mFooterCaption.GetValue() :
														CPreferences::sPrefs->mLtrFooterCaption.GetValue(), GetMessage(), false, p.GetPageIndex());

	// Get area to draw into
	JRect drawable = p.GetPageRect();
	drawable.top = drawable.bottom - footerHeight + cCaptionVertMargin - cBoxBottomMargin;

	// Draw it
	DrawCaption(p, drawable, footTxt, CPreferences::sPrefs->mFooterBox.GetValue());
}

void CSpacebarEditView::DrawCaption(JPagePrinter& p, const JRect& boxRect, const cdstring& caption, bool box)
{
	// Save drawing state
	StPenState save(&p);

	// Set font to caption font
	p.SetFontName(CPreferences::sPrefs->mCaptionTextFontInfo.GetValue().fontname);
	p.SetFontSize(CPreferences::sPrefs->mCaptionTextFontInfo.GetValue().size);

	// Draw caption one line at a time
	{
		JRect captionRect = boxRect;
		captionRect.Shrink(2, 0);

		char* tok = ::strtok(const_cast<char*>(caption.c_str()), "\n");
		while(tok)
		{
			p.String(captionRect, tok);
			captionRect.top += p.GetLineHeight();
			tok = ::strtok(NULL, "\n");
		}
	}

	// Draw box
	if (box)
	{
		p.SetFilling(kFalse);
		p.Rect(boxRect);
	}
}

void CSpacebarEditView::Print(JPagePrinter& p)
{
	// Must reset to printer font if non-styled text
	if (ChangePrintFont())
	{
		SetPrinting(kTrue);
#ifndef USE_FONTMAPPER
		ResetFont(CPreferences::sPrefs->mPrintTextFontInfo.GetValue(), 0);
#else
		ResetFont(CPreferences::sPrefs->mPrintTextFontInfo.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits, 0);
#endif
		SetPrinting(kFalse);
	}

	// Check for summary headers
	AddPrintSummary();

	// Do inherited
	CAddressText::Print(p);

	// Check for summary headers
	RemovePrintSummary();

	// Reset font change
	if (ChangePrintFont())
	{
		SetPrinting(kTrue);
#ifndef USE_FONTMAPPER
		ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue(), 0);
#else
		ResetFont(CPreferences::sPrefs->mDisplayTextFontInfo.GetValue().GetCharsetFontDescriptor(eUSAscii).mTraits.traits, 0);
#endif
		SetPrinting(kFalse);
	}

	// Clean up any message created during print
	CleanMessage();
}
