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


// Source for CTextBase class

#include "CTextBase.h"

#include "CCharsetManager.h"
#include "CCommands.h"
#include "CPreferences.h"
#include "CRFC822.h"

#include "ustrfind.h"

#include <limits.h>

StStopRedraw::StStopRedraw(CTextBase* theTD)
{
	mText = theTD;
	mWasVisible = mText->IsRedrawOn();
	if (mWasVisible) mText->SetRedraw(false);
}

StStopRedraw::~StStopRedraw()
{
	if (mWasVisible)
	{
		mText->SetRedraw(true);
	}
}

// __________________________________________________________________________________________________
// C L A S S __ C T E X T D I S P L A Y
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CTextBase::CTextBase(LStream *inStream)
		: CTextWidget(inStream)
{
	mDirty = false;
}

// Default destructor
CTextBase::~CTextBase()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get all text as utf8
void CTextBase::GetText(cdstring& all) const
{
	cdustring uniall;
	GetText(uniall);
	all = uniall.ToUTF8();
}

cdstring CTextBase::GetText() const
{
	cdstring result;
	GetText(result);
	return result;
}

// Get selected text as utf8
void CTextBase::GetSelectedText(cdstring& text) const
{
	cdustring unitext;
	GetSelectedText(unitext);
	text = unitext.ToUTF8();
}

// Set text - text MUST be utf16 with byte (not wchar_t) count
void CTextBase::SetTextPtr(const char* inTextP, SInt32 inTextLen)
{
	CTextWidget::SetTextPtr(inTextP, inTextLen);

	// Not dirty and no pending undo
	SetDirty(false);
}

// Set all text from utf8
void CTextBase::SetText(const cdstring& all, bool scroll_to_top)
{
	cdustring utf16all(all);
	SetText(utf16all, scroll_to_top);
}

void CTextBase::SetText(const char* txt, size_t len, bool scroll_to_top)
{
	cdustring utf16all(txt, len);
	SetText(utf16all, scroll_to_top);
}

// Get all text as utf16
void CTextBase::GetText(cdustring& all) const
{
	all.assign(mTextPtr, mTextLength);
}

// Get selected text as utf16
void CTextBase::GetSelectedText(cdustring& text) const
{
	SInt32 selStart;
	SInt32 selEnd;
	GetSelection(&selStart, &selEnd);
	if (selStart != selEnd)
	{
		text.assign(mTextPtr + selStart, selEnd - selStart);
	}
}

// Set all text from utf16
void CTextBase::SetText(const cdustring& all, bool scroll_to_top)
{
	SetTextPtr((const char*)all.c_str(), all.length() * sizeof(unichar_t));
	if (scroll_to_top)
	{
		// Need to make things visible and force scroll check
		bool was_visible = IsRedrawOn();
		if (!was_visible)
			SetRedraw(true);
		//CheckScroll();
		
		// Now scroll to top
		ScrollImageTo(0, 0, true);
		SetSelectionRange(0, 0);
		
		// Reset visibility
		if (!was_visible)
			SetRedraw(false);
	}
}

// Insert text from utf16
void CTextBase::InsertText(const cdustring& utxt)
{
	InsertText(utxt.c_str());
}

// Insert text from utf16
void CTextBase::InsertText(const unichar_t* utxt, size_t len)
{
	if (len == -1)
		len = ::unistrlen(utxt);
	
	FocusDraw();
	CTextWidget::InsertText(utxt, len);
	
	//CheckScroll(true);
	UserChangedText();

}

// Insert unstyled utf8 at selection
void CTextBase::InsertUTF8(const cdstring& txt)
{
	InsertUTF8(txt.c_str(), txt.length());
}

// Insert unstyled utf8 at selection
void CTextBase::InsertUTF8(const char* txt, size_t len)
{
	if (txt == NULL)
		return;

	if (len == -1)
		len = ::strlen(txt);
	
	cdustring utxt(txt, len);

	FocusDraw();
	CTextWidget::InsertText(utxt, utxt.length());
	
	//CheckScroll(true);
	UserChangedText();
}

// Force dirty
void CTextBase::UserChangedText(void)
{
	if (!IsDirty())
	{
		SetUpdateCommandStatus(true);
		SetDirty(true);
	}
}
#pragma mark ____________________________Text Processing

void CTextBase::DoPaste()
{
	if (not IsReadOnly())
	{
		if (!mAllowStyles)
		{
			bool has_utf16 = UScrap::HasData(kScrapFlavorTypeUnicode);
			bool has_text = UScrap::HasData(kScrapFlavorTypeText);
			ScrapFlavorType flavor = has_utf16 ? kScrapFlavorTypeUnicode : kScrapFlavorTypeText;

			// Always do plain text insert - no styles
			Handle txt = ::NewHandle(0);
			if (txt && UScrap::GetData(flavor, txt))
			{
				StHandleLocker lock(txt);
				if (has_utf16)
					InsertText((unichar_t*)*txt, ::GetHandleSize(txt) / sizeof(unichar_t));
				else
					InsertUTF8(*txt, ::GetHandleSize(txt));
			}

			// Done with original scrap handle
			if (txt)
				::DisposeHandle(txt);
			
			ShowSelection(true);
		}
		else
		{
			CTextWidget::DoPaste();
		}
	}
}
