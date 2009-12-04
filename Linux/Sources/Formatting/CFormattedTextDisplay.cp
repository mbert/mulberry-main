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



#include "CFormattedTextDisplay.h"

#include "CClickElement.h"
#include "CClickList.h"
#include "CClipboard.h"
#include "CCommands.h"
#include "CFormatList.h"
#include "CParserEnriched.h"
#include "CParserHTML.h"
#include "CParserPlain.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CURL.h"
#include "CURLHelpersMap.h"
#include "ETag.h"

#include <strstream>
#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXTextMenu.h>
#include <jTime.h>

#include <X11/cursorfont.h>

#include <cassert>
#include <memory>

CFormattedTextDisplay::
CFormattedTextDisplay(JXContainer* enclosure,
		      const HSizingOption hSizing, const VSizingOption vSizing,
		      const JCoordinate x, const JCoordinate y,
		      const JCoordinate w, const JCoordinate h)
	: CSpacebarEditView(enclosure, hSizing, vSizing, x, y, w, h), 
	 	mColorList(GetColormap())
{
	mList = NULL;
	mClickList = NULL;
	mOverrideList = NULL;
	mHandleClick = true;
	mClickElement = NULL;
	mContextClickElement = NULL;
	mAnchorMap = NULL;
	mHasTooltip = false;
	mView = eViewFormatted;
	mFontScale = 0;
	mURLPopup = NULL;
	ResetColors();
  
	mLinkCursor = GetDisplay()->CreateBuiltInCursor("XC_hand2", XC_hand2);
}

CFormattedTextDisplay::~CFormattedTextDisplay()
{
	delete mList; 
	mList = NULL;

	delete mClickList; 
	mClickList = NULL;
		
	delete mAnchorMap;
	mAnchorMap = NULL;

	delete mOverrideList;
	mOverrideList = NULL;
	
	delete mURLPopup;
	mURLPopup = NULL;
}

void CFormattedTextDisplay::Reset(bool click, long scale)
{
	mText.clear(); 

	delete mList; 
	mList = NULL;

	if (click)
	{
		delete mClickList; 
		mClickList = NULL;
		mClickElement = NULL;
		mContextClickElement = NULL;

		delete mAnchorMap;
		mAnchorMap = NULL;
		
		// Hide the tooltip
		ClearHint();
		mHasTooltip = false;
	}

	delete mOverrideList;
	mOverrideList = NULL;
	
	mFontScale = scale;
	
	ResetColors();
}

void CFormattedTextDisplay::ResetColors()
{
	// Set default colors
	mBackground = (GetColormap())->GetDefaultFocusColor();
	mTextColor = (GetColormap())->GetBlackColor();
}

void CFormattedTextDisplay::DrawHeader(SInt32 start, SInt32 stop)
{
	mList->addElement(new CColorFormatElement(start, stop, CPreferences::sPrefs->mHeaderStyle.GetValue().color));
	
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & bold)
		mList->addElement(new CFaceFormatElement(start, stop, E_BOLD));
		
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & underline)
		mList->addElement(new CFaceFormatElement(start, stop, E_UNDERLINE));
		
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & italic)
		mList->addElement(new CFaceFormatElement(start, stop, E_ITALIC));
}

void CFormattedTextDisplay::DrawHeaderDirect(SInt32 start, SInt32 stop)
{
	// Formatting changes selection
	StPreserveSelection _preserve(this);

	ColorFormat(CPreferences::sPrefs->mHeaderStyle.GetValue().color, start, stop);

	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & bold)
		FaceFormat(E_BOLD, start, stop);

	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & underline)
		FaceFormat(E_UNDERLINE, start, stop);

	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & italic)
		FaceFormat(E_ITALIC, start, stop);
}

// UTF8 in
void CFormattedTextDisplay::ParseHeader(const char* header, EView parsing)
{
	int start=0, stop=0;
	
	if (mList == NULL)
		mList = new CFormatList;
	
	if (mClickList == NULL)
		mClickList = new CClickList;
	
	if (mAnchorMap == NULL)
		mAnchorMap = new CAnchorElementMap;

	// Only if headerexists
	if (header != NULL)
	{
		// Convert to UTF16 for insertion into text widget
		cdustring uheader(header);
		uheader.ConvertEndl();
		const unichar_t* p = uheader;
		int char_offset = 0;
		int start = 0;
		int stop = 0;
		for(; *p; p++, char_offset++)
		{
			switch(*p)
			{
			case '\r':
			case '\n':
				start = char_offset;
				break;
			case ':':
				if (*(p+1) == ' ')
					stop = char_offset;
				else
				{
					start = -1;
					stop = -1;
				}
				break;
			case ' ':
			case '\t':
				start = -1;
				stop = -1;
				break;
			}

			if ((stop > 0) && (start >= 0))
			{
				stop++;
				DrawHeader(start, stop);
				stop = 0;
				start = -1;
			}
		}

		// Do URLs
		{
			std::auto_ptr<CParserPlain> PParser(new CParserPlain(uheader, mList, mClickList));
			for(unsigned long j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
				PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
			PParser->LookForURLs(mText.length());
		}

		// Do font
		cdstring fontName = CPreferences::sPrefs->mDisplayTextFontInfo.GetValue().fontname;
		mList->addElement(new CFontFormatElement(1, char_offset, fontName));
		mList->addElement(new CFontSizeFormatElement(1, char_offset, CPreferences::sPrefs->mDisplayTextFontInfo.GetValue().size, false, true));
		
		mText += uheader;
	}
}

const unichar_t* CFormattedTextDisplay::ParseBody(const unichar_t* body, EContentSubType etype,  EView parsing, long quote, bool use_styles)
{
	// Reset background/text colors
	ResetColors();

	if (mList == NULL)
		mList = new CFormatList;

	if (mClickList == NULL)
		mClickList = new CClickList;

	if (mAnchorMap == NULL)
		mAnchorMap = new CAnchorElementMap;

	if (mOverrideList == NULL)
		mOverrideList = new CFormatList;

	if(body)
	{
		switch(etype)
		{
		case eContentSubHTML:
			{
				if (parsing != eViewRaw)
				{
					std::auto_ptr<CParserHTML> HParser(new CParserHTML(body, mList,
													(mHandleClick && (parsing == eViewFormatted)) ? mClickList : NULL,
													(mHandleClick && (parsing == eViewFormatted)) ? mAnchorMap : NULL, use_styles));
					HParser->SetFontScale(mFontScale);
					std::auto_ptr<const unichar_t> data(HParser->Parse(mText.length(), true));
					if (parsing == eViewFormatted)
					{
						mBackground = mColorList.Add(HParser->GetBackgroundColor());
						mTextColor = mColorList.Add(HParser->GetTextColor());
					}
					mText += data.get();
				}
				else
				{
					std::auto_ptr<CParserHTML> HParser(new CParserHTML(body, mList, mClickList, mAnchorMap));
					HParser->SetFontScale(mFontScale);
					HParser->RawParse(mText.length());
					
					mText += body;
				}
			}
			break;
		case eContentSubEnriched:
			{
				if (parsing != eViewRaw)
				{
					std::auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, mList, use_styles));
					EParser->SetFontScale(mFontScale);
					std::auto_ptr<const unichar_t> data(EParser->Parse(mText.length(), true));
					EParser.reset();
					
					if (mHandleClick)
					{
						// URLs only if formatted display
						if (parsing == eViewFormatted)
						{
							std::auto_ptr<CParserPlain> PParser(new CParserPlain(data.get(), mOverrideList, mClickList));
						
							for(int j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
								PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
							PParser->LookForURLs(mText.length());
						}
					}
					mText += data.get();
				}
				else
				{
					std::auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, mList));
					EParser->SetFontScale(mFontScale);
					EParser->RawParse(mText.length());
					
					mText += body;
				}
			}
			break;
		case eContentSubPlain:
		default:
			if (mHandleClick)
			{
				std::auto_ptr<CParserPlain> PParser(new CParserPlain(body, NULL, NULL));
				if (parsing == eViewFormatted)
					PParser->SetQuoteDepth(quote);
				std::auto_ptr<const unichar_t> data(PParser->Parse(mText.length()));
				PParser.reset();

				// URLs only if formatted display
				if (parsing == eViewFormatted)
				{
					PParser.reset(new CParserPlain(data.get(), mOverrideList, mClickList));
					for(int j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
						PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
					PParser->LookForURLs(mText.length());
				}
				mText += data.get();
			}
			else
				mText += body;
			break;
		}
	}

	else
	{
		mText += body;
	}

	return mText.c_str();
}

// UTF8 in
void CFormattedTextDisplay::InsertFormattedHeader(const char* header)
{
	if ((header == NULL) || !*header)
		return;

	// Start at the top
	SetSelectionRange(0, 0);

	// Convert to UTF16 for insertion into text widget
	cdustring uheader(header);

	// Format header text at the start of the message
	const unichar_t* p = uheader;
	while(*p)
	{
		bool do_format = false;
		unsigned long format_start = 0;
		unsigned long format_end = 0;

		// Look for start of unfolded line
		if (!isuspace(*p))
		{
			// Get header text to format
			const unichar_t* field_start = p;
			while(*p && (*p != ':'))
				p++;
			if (*p)
				p++;
			
			// Get current insert position
			JIndex dummy;
			GetSelectionRange(format_start, dummy);
			
			// Insert the header field text
			InsertText(field_start, p - field_start);
			do_format = true;
			format_end = format_start + p - field_start;
		}

		// Insert entire line unformatted
		const unichar_t* line_start = p;
		while(*p && (*p != lendl1))
			p++;
		if (*p)
			p++;
		InsertText(line_start, p - line_start);
			
		// Format it (do this after inserting the other text to prevent the other text
		// picking up the formatting style of the header)
		if (do_format)
			DrawHeaderDirect(format_start, format_end);
	}
}

void CFormattedTextDisplay::InsertFormatted(EView parsing)
{
	mView = parsing;

	SetText(mText.ToUTF8().c_str());

	// Always set background color
	SetBackgroundColor(mBackground);

	if(mView == eViewFormatted)
	{
		// Change default text color if not default
		if (mTextColor != (GetColormap())->GetBlackColor())
			ColorFormat((GetColormap())->JColormap::GetRGB(mTextColor));

		if (mList)
			mList->draw(this);

		if (!mText.empty())
			DoQuotation();

		// Do override list (URLs) after everything else
		if (mOverrideList)
			mOverrideList->draw(this);
	}
	else if (mView == eViewRaw)
	{
		if (mList)
			mList->draw(this);
	}

	ClearUndo();

	Reset(false);
}

void CFormattedTextDisplay::InsertFormattedText(const unichar_t* body, EContentSubType subtype, bool quote, bool forward)
{
	// This is always quoted text - we do not spell check it
	StPauseSpelling _spelling(this);

	if (subtype == eContentSubPlain)
	{
		// Just insert text as-is
		InsertText(body);
	}
	else
	{
		// Create styler list
		std::auto_ptr<CFormatList> flist(new CFormatList);
		std::auto_ptr<const unichar_t> data;

		// Get pos of insert cursor
		JIndex selStart;
		JIndex selEnd;
		GetSelectionRange(selStart, selEnd);

		// Parse the text to get raw text and styles
		if (subtype == eContentSubHTML)
		{
			std::auto_ptr<CParserHTML> HParser(new CParserHTML(body, flist.get(), NULL, NULL, true));
			data.reset(HParser->Parse(selStart, true, quote, forward));
		}
		else if (subtype == eContentSubEnriched)
		{
			std::auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, flist.get(), true));
			data.reset(EParser->Parse(selStart, true, quote, forward));
		}
		
		// Now insert the text
		InsertText(data.get());

		// Draw the styles
		{
			StPreserveSelection _preserve(this);
			flist->draw(this);
		}
	}
}

void CFormattedTextDisplay::AdjustCursor(const JPoint& pt, const JXKeyModifiers& modifiers)
{
	CaretLocation caret = CalcCaretLocation(pt);
	JIndex offset = caret.charIndex;
	bool mods_off = !(modifiers.shift() || modifiers.control() || modifiers.alt() || modifiers.meta());

	// Lookup current click element
	CClickElement* hit_element = (mClickList ? mClickList->findCursor(offset) : NULL);

	// Check for end of text
	bool change = (mHandleClick && mods_off && mClickList && (offset < GetTextLength()) ? mClickList->findCursor(offset - 1) : false);

	// Check for line end
	if (change)
	{
		// Look at point of next character
		JCoordinate rhs = GetCharRight(offset);
		if (rhs < pt.x)
		{
			change = false;
			hit_element = NULL;
		}
	}

	if (change)
		// Show finger cursor
		DisplayCursor(mLinkCursor);
	else
		// Normal cursor
		CSpacebarEditView::AdjustCursor(pt, modifiers);

	// Do tooltip
	bool display_tooltip = false;
	if (!mClickElement && hit_element)
	{
		// Save the current element
		mClickElement = hit_element;
		display_tooltip = true;
	}
	else if (mClickElement && !hit_element)
	{
		// Clear the current element
		mClickElement = NULL;
		
		// Hide the tooltip
		ClearHint();
		mHasTooltip = false;
	}
	else if (mClickElement != hit_element)
	{
		// Save the current element
		mClickElement = hit_element;
		display_tooltip = true;
		
		// Hide the tooltip
		ClearHint();
		mHasTooltip = false;
	}

	// Initiate display if requested
	if (display_tooltip)
	{
		// Get URL text - decode %'s
		cdstring url(mClickElement->GetDescriptor());
		url.DecodeURL();
		
		// Check for mailto and decode =?'s
		if (url.compare_start(cMailtoURLScheme, true))
			CRFC822::TextFrom1522(url);

		// Determine suitable rect
		JRect rect(pt.y, pt.x, pt.y + 16, pt.x + 16);

		// Start a tooltip
		SetHint(url.c_str(), &rect);
		mHasTooltip = true;
	}
}

void CFormattedTextDisplay::HandleMouseDown(const JPoint& pt,
	const JXMouseButton button, const JSize clickCount,
	const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// Hide tooltip
	if (mHasTooltip)
	{
		ClearHint();
		mHasTooltip = false;
		mClickElement = NULL;
	}
	
	CaretLocation caret = CalcCaretLocation(pt);
	JIndex po = caret.charIndex;
	bool mods_off = !(modifiers.shift() || modifiers.control() || modifiers.alt() || modifiers.meta());

	// Must check for left click
	CClickElement* element = FindCursor(po - 1);
	if ((button == kJXButton1) && mods_off && element && (po < GetTextLength() - 1) && (GetCharRight(po) >= pt.x))
		mClickElement = element;
	else
	{
		mClickElement = NULL;
		CSpacebarEditView::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
	}
}

void CFormattedTextDisplay::HandleMouseUp(const JPoint& pt, const JXMouseButton button,
	const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// Check that mouse up was in same URL
	CaretLocation caret = CalcCaretLocation(pt);
	JIndex po = caret.charIndex;
	CClickElement* element = FindCursor(po - 1);

	if (mClickElement && (mClickElement == element) &&  (GetCharRight(po) >= pt.x))
	{
		// Select URL first
		SetSel(mClickElement->getStart(), mClickElement->getStop());
		
		if (element->IsAnchor() || element->Act(this))			// WARNING mClickElement sometimes gets set to NULL
																// during this call so revert to using 'element'
		{
			// Flash selection
			SetCaretLocation(element->getStart() + 1);
			Redraw();
			JWait(0.10);
			SetSel(element->getStart(), element->getStop());
			Redraw();
			JWait(0.10);
			SetSel(element->getStart(), element->getStart());
			Redraw();
			JWait(0.10);
			SetSel(element->getStart(), element->getStop());
			
			// Change style to URL seen if formatted display
			if (mView == eViewFormatted)
			{
				ColorFormat(CPreferences::sPrefs->mURLSeenStyle.GetValue().color, element->getStart(), element->getStop());
				if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & bold)
					FaceFormat(E_BOLD, element->getStart(), element->getStop());
				else
					FaceFormat(E_UNBOLD, element->getStart(), element->getStop());
				if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & underline)
					FaceFormat(E_UNDERLINE, element->getStart(), element->getStop());
				else
					FaceFormat(E_UNUNDERLINE, element->getStart(), element->getStop());
				if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & italic)
					FaceFormat(E_ITALIC, element->getStart(), element->getStop());
				else
					FaceFormat(E_UNITALIC, element->getStart(), element->getStop());
			}
			
			// Now do anchor action
			if (element->IsAnchor())
				element->Act(this);
		}
	}
	else
		// Do inherited
		CSpacebarEditView::HandleMouseUp(pt, button, buttonStates, modifiers);

	mClickElement = NULL;
}

void CFormattedTextDisplay::ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	// Hide tooltip
	if (mHasTooltip)
	{
		ClearHint();
		mHasTooltip = false;
		mClickElement = NULL;
	}
	mContextClickElement = NULL;

	if (mHandleClick)
	{
		// Find character offset at event point
		CaretLocation caret = CalcCaretLocation(pt);
		JIndex po = caret.charIndex;
		CClickElement* element = FindCursor(po - 1);

		if (element && (po < GetTextLength() - 1) && (GetCharRight(po) >= pt.x))
			mContextClickElement = element;

		// Only use special menu for 'external' URLs (i.e. not anchors within this document)
		if (mContextClickElement && !mContextClickElement->IsAnchor())
		{
			// Set selection
			SetSel(mContextClickElement->getStart(), mContextClickElement->getStop());

			// Create popup menu
			if (!mURLPopup)
			{
				// Create array of context menus
				JPtrArray<JXMenu> menus(JPtrArrayT::kForgetAll);
				CMainMenu::sMMenu->CreateContext(CMainMenu::eContextMessageTextURL, this, menus);

				// Grab the first one
				assert(menus.GetElementCount() > 0);
				mURLPopup = static_cast<JXTextMenu*>(menus.FirstElement());

				// Listen to them all
				// NB Use of virtual base means that mOwner also ends up listening!
				for(unsigned int i = 1; i <= menus.GetElementCount(); i++)
					ListenTo(menus.NthElement(i));
			}

			// Do popup menu of suggestions
			mURLPopup->PopUp(this, pt, buttonStates, modifiers);

			// Stop further context menu processing
			return;
		}
	}
	
	CSpacebarEditView::ContextEvent(pt, buttonStates, modifiers);
}


bool CFormattedTextDisplay::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	bool result = true;

	switch(cmd)
	{
	case CCommand::eEditOpenLink:
		OnOpenLink();
		break;

	case CCommand::eEditCopyLink:
		OnCopyLink();
		break;
	
	default:
		result = CSpacebarEditView::ObeyCommand(cmd, menu);
	}
	
	return result;
}

// Handle menus our way
void CFormattedTextDisplay::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eEditOpenLink:
	case CCommand::eEditCopyLink:
		cmdui->Enable(kTrue);
		return;
	}

	CSpacebarEditView::UpdateCommand(cmd, cmdui);
}

void CFormattedTextDisplay::OnOpenLink()
{
	if (!mContextClickElement)
		return;

	CClickElement* element = mContextClickElement;
	mContextClickElement = NULL;

	if (element->Act(this))			// WARNING mClickElement sometimes gets set to NULL
															// during this call so revert to using 'element'
	{
		// Flash selection
		SetCaretLocation(element->getStart() + 1);
		Redraw();
		JWait(0.10);
		SetSel(element->getStart(), element->getStop());
		Redraw();
		JWait(0.10);
		SetSel(element->getStart(), element->getStart());
		Redraw();
		JWait(0.10);
		SetSel(element->getStart(), element->getStop());
		
		// Change style to URL seen if formatted display
		if (mView == eViewFormatted)
		{
			ColorFormat(CPreferences::sPrefs->mURLSeenStyle.GetValue().color, element->getStart(), element->getStop());
			if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & bold)
				FaceFormat(E_BOLD, element->getStart(), element->getStop());
			else
				FaceFormat(E_UNBOLD, element->getStart(), element->getStop());
			if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & underline)
				FaceFormat(E_UNDERLINE, element->getStart(), element->getStop());
			else
				FaceFormat(E_UNUNDERLINE, element->getStart(), element->getStop());
			if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & italic)
				FaceFormat(E_ITALIC, element->getStart(), element->getStop());
			else
				FaceFormat(E_UNITALIC, element->getStart(), element->getStop());
		}
	}
}

void CFormattedTextDisplay::OnCopyLink()
{
	if (!mContextClickElement)
		return;

	// get url text
	cdstring url = mContextClickElement->GetDescriptor();
	mContextClickElement = NULL;

	// Now copy to scrap
	CClipboard::CopyToPrimaryClipboard(GetDisplay(), url);
}

bool CFormattedTextDisplay::DoAnchor(const CClickElement* anchor)
{
	// Only f anchors exist
	if (mAnchorMap == NULL)
		return false;

	// See if matching anchor exists
	CAnchorElementMap::const_iterator found = mAnchorMap->find(anchor->GetDescriptor());
	if (found != mAnchorMap->end())
	{
		// Get target element
		const CAnchorClickElement* target = &(*found).second;
		
		// Get position of character at target
		JRect rect = GetBounds();
		rect.right = rect.left + 1;
		rect.top = GetLineTop(GetLineForChar(target->getStart()));

		// Scroll target element text position to top of screen
		ScrollToRect(rect);
		
		return true;
	}
	
	return false;
}

CClickElement* CFormattedTextDisplay::FindCursor(int po)
{
	return (mClickList ? mClickList->findCursor(po) : NULL);
}

bool CFormattedTextDisplay::LaunchURL(const cdstring& url)
{
	return CURLHelpersMap::LaunchURL(url);
}

void CFormattedTextDisplay::FaceFormat(ETag tagid)
{
	if (!HasSelection()) return;
	switch (tagid) {
	case E_BOLD:
		SetCurrentFontBold(kTrue);
		break;
	case E_UNBOLD:
		SetCurrentFontBold(kFalse);
		break;
	case E_UNDERLINE:
		SetCurrentFontUnderline(1);
		break;
	case E_UNUNDERLINE:
		SetCurrentFontUnderline(0);
		break;
	case E_ITALIC:
		SetCurrentFontItalic(kTrue);
		break;
	case E_UNITALIC:
		SetCurrentFontItalic(kFalse);
		break;
	case E_PLAIN:
		//would be nice to do these all at once, but
		//can't without changing all of them back to default color
		SetCurrentFontBold(kFalse);
		SetCurrentFontItalic(kFalse);
		SetCurrentFontUnderline(0);
		break;
	default:
		//do nothing
		;
	}
}

void CFormattedTextDisplay::FaceFormat(ETag tagid, SInt32 start, SInt32 stop)
{
	// Adjust for JX selection range and start index
	SetSel(start, stop);
	FaceFormat(tagid);
}

void CFormattedTextDisplay::FontFormat(const char* font)
{
	if (HasSelection())
		SetCurrentFontName(font);
}

void CFormattedTextDisplay::FontFormat(const char* font, SInt32 start, SInt32 stop) 
{
	// Adjust for JX selection range and start index
	start++;

	if (start > stop) return;
	JTextEditor16::SetFontName(start, stop, font, kTrue);
}

void CFormattedTextDisplay::ColorFormat(JRGB color)
{
	if (HasSelection())
	{		
		JColorIndex colorIndex;
		colorIndex = mColorList.Add(color);
		SetCurrentFontColor(colorIndex);
	}
}

void CFormattedTextDisplay::ColorFormat(JRGB color, SInt32 start, SInt32 stop)
{
	SetSel(start, stop);
	ColorFormat(color);
}

void CFormattedTextDisplay::FontSizeFormat(short size, bool adding, bool overrideprefs)
{
	if (!adding)
	{
		// Limit size to preference minimum
		if (!overrideprefs && (size < CPreferences::sPrefs->mMinimumFont.GetValue()))
			size = CPreferences::sPrefs->mMinimumFont.GetValue();
		SetCurrentFontSize(size);
	}
	else
	{
		//just take size of first and add to it
		JIndex start,end;
		if (GetSelection(&start, &end))
		{
			JSize currentSize = GetFontSize(start);
			SetCurrentFontSize(currentSize + size);
		}
	}
}

void CFormattedTextDisplay::FontSizeFormat(short size, bool adding, SInt32 start, SInt32 stop, bool overrideprefs)
{
	SetSel(start, stop);
	FontSizeFormat(size, adding, overrideprefs);
}

void CFormattedTextDisplay::AlignmentFormat(ETag tagid)
{
	switch(tagid)
	{
	case E_FLEFT:
		SetCurrentFontAlign(kAlignLeft);
		break;
	case E_CENTER:
		SetCurrentFontAlign(kAlignCenter);
		break;
	case E_FRIGHT:
		SetCurrentFontAlign(kAlignRight);
		break;
	default:;
	}
}

void CFormattedTextDisplay::AlignmentFormat(ETag tagid, SInt32 start, SInt32 stop)
{
	SetSel(start, stop);
	AlignmentFormat(tagid);
}

void CFormattedTextDisplay::ExcerptFormat(char Excerpt, SInt32 start, SInt32 stop)
{
	// Just apply quotation style from prefs
	ColorFormat(CPreferences::sPrefs->mQuotationStyle.GetValue().color, start, stop);
	
	if (CPreferences::sPrefs->mQuotationStyle.GetValue().style & bold)
		FaceFormat(E_BOLD, start, stop);
	
	if (CPreferences::sPrefs->mQuotationStyle.GetValue().style & underline)
		FaceFormat(E_UNDERLINE, start, stop);
	
	if (CPreferences::sPrefs->mQuotationStyle.GetValue().style & italic)
		FaceFormat(E_ITALIC, start, stop);		
}

void CFormattedTextDisplay::DrawQuotation(long start, long stop, long depth)
{
	// Already using JX selection range and start index

	//Get the right color
	JRGB theColor;
	if (depth == 1)
		theColor = CPreferences::sPrefs->mQuotationStyle.GetValue().color;
	else
	{
		// Decrement to bump down index
		depth--;
		
		// Click to max size
		long max_size = CPreferences::sPrefs->mQuoteColours.GetValue().size();
		if (depth > max_size)
			depth = max_size;
		if (depth)
			theColor = CPreferences::sPrefs->mQuoteColours.GetValue().at(depth - 1);
	}

	JColorIndex theColorIndex;
	theColorIndex = mColorList.Add(theColor);

	//create a font style with right settings
	JFontStyle  fs(
		JBoolean(CPreferences::sPrefs->mQuotationStyle.GetValue().style & bold),
		JBoolean(CPreferences::sPrefs->mQuotationStyle.GetValue().style & italic),
		(CPreferences::sPrefs->mQuotationStyle.GetValue().style & underline) ? 1 : 0,
		kFalse, theColorIndex);

	//slap it on
	JTextEditor16::SetFontStyle(start, stop, fs, kTrue);
}

void CFormattedTextDisplay::DoQuotation()
{
	// Don't bother if no quotes to recognize
	if (!CPreferences::sPrefs->mRecognizeQuotes.GetValue().size())
	return;

	// Get number of lines and create line info array
	int lines = GetLineCount();
	std::auto_ptr<lineInfo> info(new lineInfo[lines]);

	// Get quotes and pre-calculate their sizes
	const cdstrvect& quotes = CPreferences::sPrefs->mRecognizeQuotes.GetValue();
	cdustrvect uquotes;
	ulvector sizes;
	for(cdstrvect::const_iterator iter = quotes.begin(); iter != quotes.end(); iter++)
	{
		uquotes.push_back(cdustring(*iter));
		sizes.push_back(uquotes.back().length());
	}
  
	// Determine quote depth of each line
	long current_depth = 0;
	for(int i = 1; i <= lines; i++)
    {
      int astart, astop;
      astart = GetLineStart(i);
      astop = GetLineEnd(i);
      const unichar_t* c = &mText.c_str()[astart - 1];
      
      // Check whether line is 'real' ie previous line ended with CR
      bool real_line = (astart == 1) || (mText.c_str()[astart - 2] == '\n');
      
      info.get()[i - 1].start = astart;
      info.get()[i - 1].stop = astop;
      
      // Look for quotation only if 'real' line
      if (real_line)
		current_depth = GetQuoteDepth(c, uquotes, sizes);
      
      info.get()[i - 1].depth = current_depth;
    }
  
	long start = info.get()[0].start;
	long stop = info.get()[0].stop;
	current_depth = 0;
	for(int i = 0; i < lines; i++)
	{
		// Check if same depth
		if ((current_depth == info.get()[i].depth) ||
	 		// Check for orphaned lines
			(!info.get()[i].depth &&
		 		(info.get()[i+1].depth == current_depth)))
			// Accumulate same depth quotation
			stop = info.get()[i].stop;

		else
		{
			// Draw current quotation
			if (current_depth)
				DrawQuotation(start, stop, current_depth);

			// Reset to new accumulation
			start = info.get()[i].start;
			stop = info.get()[i].stop;
			current_depth = info.get()[i].depth;
		}
	}

	// Draw last quotation
	if (current_depth)
		DrawQuotation(start, stop, current_depth);
}

long CFormattedTextDisplay::GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes)
{
	long depth = 0;
	const unichar_t* p = line;
	while((*p == ' ') || (*p == '\t')) p++;
	while(*p)
	{
		// Compare beginning of line with each quote
		cdustrvect::const_iterator iter1 = quotes.begin();
		ulvector::const_iterator iter2 = sizes.begin();
		bool found = false;
		for(; iter1 != quotes.end(); iter1++, iter2++)
		{
			// Must check that size is non-zero otherwise infinite loop occurs
			if (*iter2 && (::unistrncmp(*iter1, p, *iter2) == 0))
			{
				p += *iter2;
				found = true;
				break;
			}
		}
		
		// Done with tests if not found
		if (!found)
			break;

		// Bump up quote depth
		depth++;
		
		// Always exit after first quote if multiples not being used
		if ((depth == 1) && !CPreferences::sPrefs->mUseMultipleQuotes.GetValue())
			break;

		while((*p == ' ') || (*p == '\t')) p++;
	}
	
	return depth;
}

