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

#include "CAddressBookManager.h"
#include "CATSUIStyle.h"
#include "CBetterScrollerX.h"
#include "CGUtils.h"
#include "CClickList.h"
#include "CCommands.h"
#include "CICSupport.h"
#include "CMulberryCommon.h"
#include "CParserEnriched.h"
#include "CParserHTML.h"
#include "CParserPlain.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CTooltip.h"
#include "CURL.h"

LMenu* CFormattedTextDisplay::sURLContextMenu = NULL;

enum
{
	ePopup_OpenLink = 1,
	ePopup_CopyLinkToClipboard
};

CFormattedTextDisplay::CFormattedTextDisplay(LStream* instream) : CTextDisplay(instream)
{
	InitFormattedTextDisplay();
}

CFormattedTextDisplay::~CFormattedTextDisplay()
{
	// Always hide tooltip which has a pointer to this cached
	if (mHasTooltip)
		CTooltip::HideTooltip();

	delete mList;
	mList = NULL;

	delete mClickList;
	mClickList = NULL;

	delete mAnchorMap;
	mAnchorMap = NULL;

	delete mOverrideList;
	mOverrideList = NULL;
}

void CFormattedTextDisplay::InitFormattedTextDisplay()
{
	mList = NULL;
	mClickList = NULL;
	mClickElement = NULL;
	mAnchorMap = NULL;
	mHasTooltip = false;
	mOverrideList = NULL;
	mHandleClick = true;
	mView = eViewFormatted;
	mFontScale = 0;

	ResetColors();
}

// Respond to commands
Boolean CFormattedTextDisplay::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_CaptureAddress:
		CaptureAddress();
		break;

	default:
		cmdHandled = CTextDisplay::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CFormattedTextDisplay::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;
	SInt32 sel_start;
	SInt32 sel_end;
	GetSelection(&sel_start, &sel_end);

	switch (inCommand)
	{
	case cmd_CaptureAddress:
		// Must have selection
		outEnabled = (sel_start != sel_end);
		break;

	// Pass up
	default:
		CTextDisplay::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
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

		delete mAnchorMap;
		mAnchorMap = NULL;

		// Hide the tooltip
		CTooltip::HideTooltip();
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
	mBackground = Color_White;
	mTextColor = Color_Black;
}

void CFormattedTextDisplay::DrawHeader(SInt32 start, SInt32 stop)
{
	mList->addElement(new CColorFormatElement(start, stop, CPreferences::sPrefs->mHeaderStyle.GetValue().color));

	if(CPreferences::sPrefs->mHeaderStyle.GetValue().style & bold)
		mList->addElement(new CFaceFormatElement(start, stop, E_BOLD));

	if(CPreferences::sPrefs->mHeaderStyle.GetValue().style & underline)
		mList->addElement(new CFaceFormatElement(start, stop, E_UNDERLINE));

	if(CPreferences::sPrefs->mHeaderStyle.GetValue().style & italic)
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
	if (mList == NULL)
		mList = new CFormatList;

	if (mClickList == NULL)
		mClickList = new CClickList;

	if (mAnchorMap == NULL)
		mAnchorMap = new CAnchorElementMap;

	// Only if header exists
	if (header != NULL)
	{
		// Convert to UTF16 for insertion into text widget
		cdustring uheader(header);
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
				if(*(p+1) == ' ')
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
			for(int j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
				PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
			PParser->LookForURLs(mText.length());
		}
		
		mText += uheader;
	}
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
		long format_start = 0;
		long format_end = 0;

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
			long dummy;
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

const unichar_t* CFormattedTextDisplay::ParseBody(const unichar_t* body, EContentSubType Etype, EView parsing, long quote, bool use_styles)
{
	int j = 0;

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
		switch(Etype)
		{
		case eContentSubHTML:
			switch(parsing)
			{
			case eViewFormatted:
				{
					std::auto_ptr<CParserHTML> HParser(new CParserHTML(body, mList, mHandleClick ? mClickList : NULL, mHandleClick ? mAnchorMap : NULL, use_styles));
					HParser->SetFontScale(mFontScale);
					std::auto_ptr<const unichar_t> data(HParser->Parse(mText.length(), true));
					mBackground = HParser->GetBackgroundColor();
					mTextColor = HParser->GetTextColor();
					mText += data.get();
				}
				break;
			case eViewPlain:
				{
					std::auto_ptr<CParserHTML> HParser(new CParserHTML(body, NULL, NULL, NULL));
					HParser->SetFontScale(mFontScale);
					std::auto_ptr<const unichar_t> data(HParser->Parse(mText.length(), true));
					mText += data.get();
				}
				break;
			case eViewRaw:
				{
					std::auto_ptr<CParserHTML> HParser(new CParserHTML(body, mList, NULL, NULL));
					HParser->SetFontScale(mFontScale);
					HParser->RawParse(mText.length());
					mText += body;
				}
				break;
			default:;
			}
			break;
		case eContentSubEnriched:
			switch(parsing)
			{
			case eViewFormatted:
				{
					std::auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, mList, use_styles));
					EParser->SetFontScale(mFontScale);
					std::auto_ptr<const unichar_t> data(EParser->Parse(mText.length(), true));
					EParser.reset();

					// URLs only if clicks handled
					if (mHandleClick)
					{
						std::auto_ptr<CParserPlain> PParser(new CParserPlain(data.get(), mOverrideList, mClickList));

						for(j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
							PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
						PParser->LookForURLs(mText.length());
					}
					mText += data.get();
				}
				break;
			case eViewPlain:
				{
					std::auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, NULL));
					EParser->SetFontScale(mFontScale);
					std::auto_ptr<const unichar_t> data(EParser->Parse(mText.length(), true));
					mText += data.get();
				}
				break;
			case eViewRaw:
				{
					std::auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, mList));
					EParser->SetFontScale(mFontScale);
					EParser->RawParse(mText.length());
					mText += body;
				}
				break;
			default:;
			}
			break;
		case eContentSubPlain:
		default:
			switch(parsing)
			{
			case eViewFormatted:
				{
					std::auto_ptr<CParserPlain> PParser(new CParserPlain(body, NULL, NULL));
					PParser->SetQuoteDepth(quote);
					std::auto_ptr<const unichar_t> data(PParser->Parse(mText.length()));
					PParser.reset();

					// URLs only if formatted display
					if(mHandleClick)
					{
						PParser.reset(new CParserPlain(data.get(), mOverrideList, mClickList));
						for(j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
							PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
						PParser->LookForURLs(mText.length());
					}
					mText += data.get();
				}
				break;
			default:;
				mText += body;
			}
			break;
		}
	}

	else
		mText += body;

	return mText.c_str();
}

void CFormattedTextDisplay::InsertFormatted(EView parse)
{
	mView = parse;

	// Set mono style flag
	switch(mView)
	{
	case eViewFormatted:
	case eViewRaw:
		//WEAllowStyles(true);
		break;
	default:
		//WEAllowStyles(false);
		break;
	}

	// Always set background color
	SetBackground(mBackground);

	FocusDraw();
	InsertText(mText);
	NeedTextLayout();
	if(mView == eViewFormatted)
	{
		// Change default text color if not default
		if ((mTextColor.red != Color_Black.red) ||
			(mTextColor.green != Color_Black.green) ||
			(mTextColor.blue != Color_Black.blue))
			ColorFormat(mTextColor, 0, mText.length());

		if (mList)
			mList->draw(this);

		if (!mText.empty())
			DoQuotation();

		// Do override mList (URLs) after everything else
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
		FocusDraw();
		InsertText(body);
		NeedTextLayout();
	}
	else
	{
		// Create styler list
		std::auto_ptr<CFormatList> flist(new CFormatList);
		std::auto_ptr<const unichar_t> data;

		// Get pos of insert cursor
		SInt32 selStart;
		SInt32 selEnd;
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
		FocusDraw();
		InsertText(data.get());
		NeedTextLayout();

		// Draw the styles
		{
			StPreserveSelection _preserve(this);
			flist->draw(this);
		}
	}
}

void CFormattedTextDisplay::AdjustMouseSelf(Point inPortPt, const EventRecord& inMacEvent, RgnHandle outMouseRgn)
{
	FocusDraw();

	Point localPt = inPortPt;
	PortToLocalPoint(localPt);

	int offset = GetOffset(localPt);

	// Lookup current click element
	CClickElement* hit_element = (mClickList ? mClickList->findCursor(offset) : NULL);

	// Check for end of text
	bool change = (mHandleClick && mClickList && (offset < GetTextLength()) ? mClickList->findCursor(offset) : false);

	// Check for line end
	if (change)
	{
		SInt32 line_start;
		SInt32 line_end;
		FindLine(offset, &line_start, &line_end);

		if (offset == line_end - 1)
		{
			// Look at point of next character
			Point thePoint = GetPoint(offset + 1);
			if (thePoint.h <= localPt.h)
			{
				change = false;
				hit_element = NULL;
			}
		}
	}

	if (change && !(inMacEvent.modifiers & (cmdKey | shiftKey | optionKey | controlKey)))
	{
		// Show finger cursor
		UCursor::SetThemeCursor(kThemePointingHandCursor);
	}
	else
		// Normal cursor
		CTextDisplay::AdjustMouseSelf(inPortPt, inMacEvent, outMouseRgn);

	// Do this after setting cursor
	PortToGlobalPoint(inPortPt);
	
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
		CTooltip::HideTooltip();
		mHasTooltip = false;
	}
	else if (mClickElement != hit_element)
	{
		// Save the current element
		mClickElement = hit_element;
		display_tooltip = true;
		
		// Hide the tooltip
		CTooltip::HideTooltip();
		mHasTooltip = false;
	}
	
	// Initiate display if requested
	if (display_tooltip)
	{
		// Get line info
		SInt32 lineNum = OffsetToLine(offset);

		// Get height of current line
		SInt32 lineHeight = GetLineHeight(lineNum);

		// Get URL text - decode %'s
		cdstring url(mClickElement->GetDescriptor());
		url.DecodeURL();
		
		// Check for mailto and decode =?'s
		if (url.compare_start(cMailtoURLScheme, true))
			CRFC822::TextFrom1522(url);

		// Point to position and add line height or at least cursor height
		SPoint32 pos = {inPortPt.h, inPortPt.v};
		pos.v += std::max(lineHeight, 18L);

		// Start a tooltip
		CTooltip::ShowTooltip(this, pos, LStr255(url));
		mHasTooltip = true;
	}
}

void CFormattedTextDisplay::MouseLeave()
{
	// Hide tooltip window
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
		mClickElement = NULL;
	}
	
	// Do inherited
	CTextDisplay::MouseLeave();
}

void CFormattedTextDisplay::DeactivateSelf()
{
	// Hide tooltip
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
		mClickElement = NULL;
	}
	
	// Do inherited
	CTextDisplay::DeactivateSelf();
}

void CFormattedTextDisplay::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Hide tooltip
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
		mClickElement = NULL;
	}
	
	FocusDraw();

	bool handled = false;

	if (mHandleClick && !(inMouseDown.macEvent.modifiers & (cmdKey | shiftKey | optionKey | controlKey)))
	{
		int offset = GetOffset(inMouseDown.whereLocal);

		// Check for end of line
		CClickElement* target = (mClickList && (offset < GetTextLength()) ? mClickList->findCursor(offset) : NULL);

		// Check for line end
		if (target)
		{
			SInt32 line_start;
			SInt32 line_end;
			FindLine(offset, &line_start, &line_end);

			if (offset == line_end - 1)
			{
				// Look at point of next character
				Point thePoint = GetPoint(offset + 1);
				if (thePoint.h <= inMouseDown.whereLocal.h)
					target = NULL;
			}
		}

		if (target)
		{
			// Set selection
			SetSelectionRange(target->getStart(), target->getStop());

			// Handle target action
			if (target->IsAnchor() || target->Act(this))
			{
				// Flash selection
				for(int i = 0; i < 3; i++)
				{
					unsigned long dummy;
					::Delay(5, &dummy);
					UnhilightCurrentSelection(0, mLineCount);

					::Delay(5, &dummy);
					HilightCurrentSelection(0, mLineCount);
				}
				
				// Change style to URL seen if formatted display
				if (mView == eViewFormatted)
				{
					ColorFormat(CPreferences::sPrefs->mURLSeenStyle.GetValue().color, target->getStart(), target->getStop());
					FaceFormat(E_PLAIN, target->getStart(), target->getStop());
					if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style)
					{
						if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & bold)
							FaceFormat(E_BOLD, target->getStart(), target->getStop());

						if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & underline)
							FaceFormat(E_UNDERLINE, target->getStart(), target->getStop());

						if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & italic)
							FaceFormat(E_ITALIC, target->getStart(), target->getStop());
					}
				}
				
				// Now do anchor action
				if (target->IsAnchor())
					target->Act(this);
			}

			handled = true;
		}
	}

	//sTextClicking = this;
	if (!handled)
		CTextDisplay::ClickSelf(inMouseDown);
}

// HandleContextMenuEvent:
//	A context menu event is detected. This method allows the text display to intercept
//	the default context menu handling and do its own thing.
//
// cmmEvent		in		event that triggered context menu call
//
// return				true: if this class handled it and no more context menu processing should be done
//						false: if not handled here and standard context menu processing should be done
//
bool CFormattedTextDisplay::HandleContextMenuEvent(const EventRecord& cmmEvent)
{
	bool result = false;

	FocusDraw();

	// Hide tooltip
	if (mHasTooltip)
	{
		CTooltip::HideTooltip();
		mHasTooltip = false;
		mClickElement = NULL;
	}
	
	// Check whether click is on a URL

	// Convert event point to a local point
	Point pt = cmmEvent.where;
	GlobalToPortPoint(pt);
	PortToLocalPoint(pt);

	if (mHandleClick)
	{
		int offset = GetOffset(pt);

		// Check for end of line
		CClickElement* target = (mClickList && (offset < GetTextLength()) ? mClickList->findCursor(offset) : NULL);

		// Check for line end
		if (target)
		{
			SInt32 line_start;
			SInt32 line_end;
			FindLine(offset, &line_start, &line_end);

			if (offset == line_end - 1)
			{
				// Look at point of next character
				Point thePoint = GetPoint(offset + 1);
				if (thePoint.h <= pt.h)
					target = NULL;
			}
		}

		// Only use special menu for 'external' URLs (i.e. not anchors within this document)
		if (target && !target->IsAnchor())
		{
			// Set selection
			SetSelectionRange(target->getStart(), target->getStop());

			// Create popup menu
			if (sURLContextMenu == NULL)
			{
				sURLContextMenu = new LMenu(2501);
				LMenuBar::GetCurrentMenuBar()->InstallMenu(sURLContextMenu, -1);
			}
			
			// Process the command status for this entire menu
			LEventDispatcher::GetCurrentEventDispatcher()->UpdateMenus();

			// Always enable the first two items
			::EnableItem(sURLContextMenu->GetMacMenuH(), ePopup_OpenLink);
			::EnableItem(sURLContextMenu->GetMacMenuH(), ePopup_CopyLinkToClipboard);

			// Do popup menu of suggestions
			Point startPt = cmmEvent.where;
			SInt32 popup_result = ::PopUpMenuSelect(sURLContextMenu->GetMacMenuH(), startPt.v, startPt.h, 0);
			short menu_hit = HiWord(popup_result);
			unsigned short item_hit = LoWord(popup_result);

			if (item_hit)
			{
				bool handled = false;
				if (menu_hit == 2501)
				{
					switch(item_hit)
					{
					case ePopup_OpenLink:
						// Handle target action
						if (target->Act(this))
						{
							// Flash selection
							FocusDraw();
							for(int i = 0; i < 3; i++)
							{
								unsigned long dummy;
								::Delay(5, &dummy);
								UnhilightCurrentSelection(0, mLineCount);

								::Delay(5, &dummy);
								HilightCurrentSelection(0, mLineCount);
							}
							
							// Change style to URL seen if formatted display
							if (mView == eViewFormatted)
							{
								ColorFormat(CPreferences::sPrefs->mURLSeenStyle.GetValue().color, target->getStart(), target->getStop());
								FaceFormat(E_PLAIN, target->getStart(), target->getStop());
								if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style)
								{
									if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & bold)
										FaceFormat(E_BOLD, target->getStart(), target->getStop());

									if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & underline)
										FaceFormat(E_UNDERLINE, target->getStart(), target->getStop());

									if (CPreferences::sPrefs->mURLSeenStyle.GetValue().style & italic)
										FaceFormat(E_ITALIC, target->getStart(), target->getStop());
								}
							}
						}
						handled = true;
						break;
					case ePopup_CopyLinkToClipboard:
					{
						// get url text
						cdstring url = target->GetDescriptor();

						// Create text handle
						LHandleStream url_txt;
						url_txt.WriteBlock(url.c_str(), url.length());

						// Now copy to scrap
						{
							StHandleLocker lock1(url_txt.GetDataHandle());

							UScrap::SetData(kScrapFlavorTypeText, url_txt.GetDataHandle());
						}
						handled = true;
						break;
					}
					default:;
					}
				}
				
				if (!handled)
				{
					// Get command number
					CommandT cmd;

					// Check for hit in this menu
					if (menu_hit == 2501)
						cmd = sURLContextMenu->CommandFromIndex(item_hit);
					else
						// Must try and get menu from menu bar
						cmd = LMenuBar::GetCurrentMenuBar()->FindCommand(menu_hit, item_hit);

					ObeyCommand(cmd, NULL);
				}
			}

			result = true;
		}
	}
	
	// Do inherited if we do not handle it
	return result ? result : CTextDisplay::HandleContextMenuEvent(cmmEvent);
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
		Point thePoint = GetPoint(target->getStart());
		LocalToPortPoint(thePoint);

		SInt32 line_index = OffsetToLine(target->getStart());
		
		SInt32 adjust = GetLineHeight(line_index);

		// Scroll target element text position to top of screen
		ScrollPinnedImageBy(0, thePoint.v - adjust - mFrameLocation.v, true);
		
		return true;
	}
	
	return false;
}

void CFormattedTextDisplay::FaceFormat(ETag tagid, SInt32 start, SInt32 stop)
{
	SetSelectionRange(start, stop);

	Style ts = normal;

	switch(tagid)
	{
	case E_BOLD:
		ts |= bold;
		break;
	case E_UNDERLINE:
		ts |= underline;
		break;
	case E_ITALIC:
		ts |= italic;
		break;
	case E_PLAIN:
		break;
	default:;
	}

	SetFontStyle(ts);
}

void CFormattedTextDisplay::FontFormat(char *font, SInt32 start, SInt32 stop)
{
	SetSelectionRange(start, stop);
	
	TextStyle ts;

	const char *fonty = font;

	LStr255 fontname(fonty, strlen(font));

	GetFNum(fontname, &ts.tsFont);
	if(ts.tsFont == 0)
	{
		for(int i=0; i < strlen(font); i++)
		{
			if(font[i] == '_')
				font[i] = ' ';
		}
	}

	fonty = font;

	LStr255 fontnameNUS(fonty, strlen(font));
	SetFontName(fontnameNUS);
}

void CFormattedTextDisplay::ColorFormat(RGBColor color, SInt32 start, SInt32 stop)
{
	SetSelectionRange(start, stop);

	SetFontColor(color);
}

void CFormattedTextDisplay::FontSizeFormat(short size, bool adding, SInt32 start, SInt32 stop, bool overrideprefs)
{
	if (adding)
	{
		// Loop over style runs
		UniCharCount length = stop - start;
		
		while(length > 0)
		{
			// Get style run for start char
			ATSUStyle style;
			UniCharArrayOffset runStart;
			UniCharCount runLength;
			if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
				return;
			
			// Get minimum length of change within this run
			UniCharCount applyLength = std::min(runLength - (start - runStart), length);
			
			// Now create new style
			CATSUIStyle newStyle;
			::ATSUCopyAttributes(style, newStyle.Get());
			newStyle.SetFontSize(newStyle.GetFontSize() + IntToFixed(size));
			
			// Apply style over run range
			try
			{
				mTextLayout.SetRunStyle(newStyle, start, applyLength);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);
				
				// Force end of loop
				applyLength = length;
			}
			
			// Adjust apply lengths
			start += applyLength;
			length -= applyLength;
		}
	}
	else
	{
		SetSelectionRange(start,stop);

		// Limit size to preference minimum
		if (!overrideprefs && (size < CPreferences::sPrefs->mMinimumFont.GetValue()))
			size = CPreferences::sPrefs->mMinimumFont.GetValue();
		SetFontSize(::Long2Fix(size));
	}
}


void CFormattedTextDisplay::AlignmentFormat(ETag tagid, SInt32 start, SInt32 stop)
{
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

bool CFormattedTextDisplay::GetLineRange(SInt32 &first, SInt32 &last, SInt32 start, SInt32 stop)
{
	int line;
	long lineStart, lineEnd;

	first = last = -1;

	int i = start;

	while(i < stop)
	{
		line = OffsetToLine(i);
		if (first < 0)
			first = line;
		last = line;

		FindLineRange(line, &lineStart, &lineEnd);
		i = lineEnd + 1;
	}
	if (first >= 0 && last >= 0)
		return true;
	else
		return false;
}

bool CFormattedTextDisplay::LaunchURL(const char* url)
{
	OSStatus err;
	err = CICSupport::ICLaunchURL((char*) url);   // justin grab_c_str change

	return (err == noErr);
}

void CFormattedTextDisplay::DrawQuotation(long start, long stop, long depth)
{
	FaceFormat(E_PLAIN, start, stop);

	if (depth <= 1)
		ColorFormat(CPreferences::sPrefs->mQuotationStyle.GetValue().color, start, stop);
	else
	{
		// Decrement to bump down index
		depth--;

		// Click to max size
		unsigned long max_size = CPreferences::sPrefs->mQuoteColours.GetValue().size();
		if (depth > max_size)
			depth = max_size;
		if (depth)
			ColorFormat(CPreferences::sPrefs->mQuoteColours.GetValue().at(depth - 1), start, stop);
	}

	if (CPreferences::sPrefs->mQuotationStyle.GetValue().style & bold)
		FaceFormat(E_BOLD, start, stop);

	if (CPreferences::sPrefs->mQuotationStyle.GetValue().style & underline)
		FaceFormat(E_UNDERLINE, start, stop);

	if (CPreferences::sPrefs->mQuotationStyle.GetValue().style & italic)
		FaceFormat(E_ITALIC, start, stop);
}

void CFormattedTextDisplay::DoQuotation()
{
	// Don't bother if no quotes to recognize
	if (!CPreferences::sPrefs->mRecognizeQuotes.GetValue().size())
		return;

	// Get number of lines and create line info array
	SInt32 lines = GetLineCount();
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
	for(int i = 0; i < lines; i++)
	{
		SInt32 astart, astop;
		FindLineRange(i, &astart, &astop);
		const unichar_t* c = &mText.c_str()[astart];

		// Check whether line is 'real' ie previous line ended with CR
		bool real_line = (astart && (mText.c_str()[astart - 1] == '\r')) || (!astart);

		info.get()[i].start = astart;
		info.get()[i].stop = astop;

		// Look for quotation only if 'real' line
		if (real_line)
			current_depth = GetQuoteDepth(c, uquotes, sizes);

		info.get()[i].depth = current_depth;
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

void CFormattedTextDisplay::CaptureAddress()
{
	// Get Selection
	cdstring selection;
	GetSelectedText(selection);
	
	// Do capture if address capability available
	if (CAddressBookManager::sAddressBookManager)
		CAddressBookManager::sAddressBookManager->CaptureAddress(selection);
}
