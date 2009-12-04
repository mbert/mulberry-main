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


// CDisplayFormatter.cp : implementation file
//


#include "CDisplayFormatter.h"

#include "CClickList.h"
#include "CFormatList.h"
#include "CFormattedTextDisplay.h"
#include "CLetterTextEditView.h"
#include "CLetterWindow.h"
#include "CParserPlain.h"
#include "CParserEnriched.h"
#include "CParserHTML.h"
#include "CPreferences.h"
#include "CStringUtils.h"
	
/////////////////////////////////////////////////////////////////////////////
// CDisplayFormatter


CDisplayFormatter::CDisplayFormatter(CFormattedTextDisplay* disp)
{
	mList = NULL;
	mOverrideList = NULL;
	mClickList = NULL;
	mAnchorMap = NULL;
	mMessageText = disp;
	mLetterText = NULL;
	mView = eViewFormatted;
	mFontScale = 0;
	ResetColors();
}


CDisplayFormatter::CDisplayFormatter(CLetterTextEditView* disp)
{
	mList = NULL;
	mOverrideList = NULL;
	mClickList = NULL;
	mAnchorMap = NULL;
	mLetterText = disp;
	mMessageText = NULL;
	mView = eViewFormatted;
	mFontScale = 0;
	ResetColors();
}


CDisplayFormatter::~CDisplayFormatter()
{
	delete mList;
	mList = NULL;
	
	delete mClickList;
	mClickList = NULL;

	delete mAnchorMap;
	mAnchorMap = NULL;
	
	delete mOverrideList;
	mOverrideList = NULL;
}

void CDisplayFormatter::Reset(bool click, long scale)
{
	mText.clear();
	
	delete mList;
	mList = NULL;
	
	if (click)
	{
		delete mClickList;
		mClickList = NULL;

		delete mAnchorMap;
		mAnchorMap = NULL;
	}
	
	delete mOverrideList;
	mOverrideList = NULL;
	
	mFontScale = scale;
	
	ResetColors();
}

void CDisplayFormatter::ResetColors()
{
	// Set default colors
	mBackground = ::GetSysColor(COLOR_WINDOW);
	mTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
}

void CDisplayFormatter::DrawHeader(SInt32 start, SInt32 stop)
{
	mList->addElement(new CColorFormatElement(start, stop, CPreferences::sPrefs->mHeaderStyle.GetValue().color));
	
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & bold)
		mList->addElement(new CFaceFormatElement(start, stop, E_BOLD));
		
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & underline)
		mList->addElement(new CFaceFormatElement(start, stop, E_UNDERLINE));
		
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & italic)
		mList->addElement(new CFaceFormatElement(start, stop, E_ITALIC));
}

void CDisplayFormatter::DrawHeaderDirect(SInt32 start, SInt32 stop)
{
	// Formatting changes selection
	CCmdEditView::StPreserveSelection _preserve(&GetTextDisplay());

	ColorFormat(CPreferences::sPrefs->mHeaderStyle.GetValue().color, start, stop);
	
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & bold)
		FaceFormat(E_BOLD, start, stop);
		
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & underline)
		FaceFormat(E_UNDERLINE, start, stop);
		
	if (CPreferences::sPrefs->mHeaderStyle.GetValue().style & italic)
		FaceFormat(E_ITALIC, start, stop);
}

// UTF8 in
void CDisplayFormatter::ParseHeader(const char* header, EView parsing)
{
	int start=0, stop=0;
	
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
		::FilterOutLFs(uheader.c_str_mod());
		const unichar_t* p = uheader;
		int char_offset = 0;
		int start = 0;
		int stop = 0;
		for(; *p; p++, char_offset++)
		{
			switch(*p)
			{
			case '\r':
				start = char_offset;
				break;
			case '\n':
				// Ignore LFs with RichEdit2.0
				char_offset--;
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
			auto_ptr<CParserPlain> PParser(new CParserPlain(uheader, mList, mClickList));
			for(int j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
				PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
			PParser->LookForURLs(mText.length());
		}

		// Do font
		LOGFONT fontInfo = CPreferences::sPrefs->mDisplayTextFontInfo.GetValue().logfont;
		cdstring fontName = fontInfo.lfFaceName;
		mList->addElement(new CFontFormatElement(0, char_offset, fontName));

		HDC hDC = ::GetDC(NULL);
		int logpix = ::GetDeviceCaps(hDC, LOGPIXELSY);
		if (logpix == 0)
			logpix = 1;
		::ReleaseDC(NULL, hDC);
		int ht = fontInfo.lfHeight;
		int pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
		mList->addElement(new CFontSizeFormatElement(0, char_offset, pt_size, false, true));
		
		mText += uheader;
	}
}

// UTF8 in
void CDisplayFormatter::InsertFormattedHeader(const char* header)
{
	if ((header == NULL) || !*header)
		return;

	// Start at the top
	GetTextDisplay().SetSelectionRange(0, 0);

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
			GetTextDisplay().GetSelectionRange(format_start, dummy);
			
			// Insert the header field text
			GetTextDisplay().InsertText(field_start, p - field_start);
			do_format = true;
			format_end = format_start + p - field_start;
		}

		// Insert entire line unformatted
		const unichar_t* line_start = p;
		while(*p && (*p != lendl1))
			p++;
		if (*p)
			p++;
		GetTextDisplay().InsertText(line_start, p - line_start); 
			
		// Format it (do this after inserting the other text to prevent the other text
		// picking up the formatting style of the header)
		if (do_format)
			DrawHeaderDirect(format_start, format_end);
	}
}

const unichar_t* CDisplayFormatter::ParseBody(const unichar_t* body, EContentSubType Etype, EView parsing, long quote, bool use_styles)
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
	
	if (body)
	{
		switch(Etype)
		{
		case eContentSubHTML:
			{
				if (parsing != eViewRaw)
				{
					auto_ptr<CParserHTML> HParser(new CParserHTML(body, mList,
													(mMessageText && (parsing == eViewFormatted)) ? mClickList : NULL,
													(mMessageText && (parsing == eViewFormatted)) ? mAnchorMap : NULL, use_styles));
					HParser->SetFontScale(mFontScale);
					auto_ptr<const unichar_t> data(HParser->Parse(mText.length(), true));
					if (parsing == eViewFormatted)
					{
						mBackground = HParser->GetBackgroundColor();
						mTextColor = HParser->GetTextColor();
					}
					mText += data.get();
				}
				else
				{
					auto_ptr<CParserHTML> HParser(new CParserHTML(body, mList, mClickList, mAnchorMap));
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
					auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, mList, use_styles));
					EParser->SetFontScale(mFontScale);
					auto_ptr<const unichar_t> data(EParser->Parse(mText.length(), true));
					EParser.reset();
					
					if (mMessageText)
					{
						// URLs only if formatted display
						if (parsing == eViewFormatted)
						{
							auto_ptr<CParserPlain> PParser(new CParserPlain(data.get(), mOverrideList, mClickList));
						
							for(int j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
								PParser->AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
							PParser->LookForURLs(mText.length());
						}
					}
					mText += data.get();
				}
				else
				{
					auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, mList));
					EParser->SetFontScale(mFontScale);
					EParser->RawParse(mText.length());
					
					mText += body;
				}
			}
			break;
		case eContentSubPlain:
		default:
			if (mMessageText)
			{
				auto_ptr<CParserPlain> PParser(new CParserPlain(body, NULL, NULL));
				if (parsing == eViewFormatted)
					PParser->SetQuoteDepth(quote);
				auto_ptr<const unichar_t> data(PParser->Parse(mText.length()));
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

void CDisplayFormatter::InsertFormatted(EView parse)
{
	mView = parse;

	{
		// Prvent drawing while this is added
		StStopRedraw redraw(mMessageText ? (CCmdEditView*) mMessageText : (CCmdEditView*) mLetterText);

		// Always set background color
		GetRichEditCtrl().SetBackgroundColor(FALSE, mBackground);

		GetTextDisplay().SetSelectionRange(0, -1);
		GetTextDisplay().InsertText(mText);
		
		if (mView == eViewFormatted)
		{
			// Change default text color if not default
			if (mTextColor != ::GetSysColor(COLOR_WINDOWTEXT))
				ColorFormat(mTextColor);

			if (mList)
				mList->draw(this);

			// Only quote in incoming messages
		 	if (mMessageText && !mText.empty())
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

		Reset(false);
	}

	GetRichEditCtrl().Invalidate();	
}

void CDisplayFormatter::InsertFormattedText(const unichar_t* body, EContentSubType subtype, bool quote, bool forward)
{
	{
		// Prvent drawing while this is added
		StStopRedraw redraw(mMessageText ? (CCmdEditView*) mMessageText : (CCmdEditView*) mLetterText);

		// This is always quoted text - we do not spell check it
		CTextDisplay::StPauseSpelling _spelling(&GetTextDisplay());

		if (subtype == eContentSubPlain)
		{
			// Just insert text as-is
			GetTextDisplay().InsertText(body);
		}
		else
		{
			// Create styler list
			auto_ptr<CFormatList> flist(new CFormatList);
			auto_ptr<const unichar_t> data;

			// Get pos of insert cursor
			long selStart;
			long selEnd;
			GetTextDisplay().GetSelectionRange(selStart, selEnd);

			// Parse the text to get raw text and styles
			if (subtype == eContentSubHTML)
			{
				auto_ptr<CParserHTML> HParser(new CParserHTML(body, flist.get(), NULL, NULL, true));
				data.reset(HParser->Parse(selStart, true, quote, forward));
			}
			else if (subtype == eContentSubEnriched)
			{
				auto_ptr<CParserEnriched> EParser(new CParserEnriched(body, flist.get(), true));
				data.reset(EParser->Parse(selStart, true, quote, forward));
			}
			
			// Now insert the text
			GetTextDisplay().InsertText(data.get());

			// Draw the styles
			{
				CTextDisplay::StPreserveSelection _preserve(&GetTextDisplay());
				flist->draw(this);
			}
		}
	}
	
	GetRichEditCtrl().Invalidate();	

}

CTextDisplay& CDisplayFormatter::GetTextDisplay()
{
	return (mMessageText ? *static_cast<CTextDisplay*>(mMessageText) : *static_cast<CTextDisplay*>(mLetterText));
}

CRichEditCtrl& CDisplayFormatter::GetRichEditCtrl()
{
	if (mMessageText)
		return mMessageText->GetRichEditCtrl();
	else
		return mLetterText->GetRichEditCtrl();
}

void CDisplayFormatter::FaceFormat(ETag tagid)
{
	long start, stop;
	
	GetTextDisplay().GetSelectionRange(start, stop);
	FaceFormat(tagid, start, stop);
}	

void CDisplayFormatter::FaceFormat(ETag tagid, SInt32 start, SInt32 stop)
{
	GetTextDisplay().SetSelectionRange(start, stop);
	
	CHARFORMAT2 format;
	
	switch(tagid)
	{
	case E_BOLD:
		format.dwMask = CFM_BOLD;
		format.dwEffects = CFE_BOLD;
		break;
	case E_UNBOLD:
		format.dwMask = CFM_BOLD;
		format.dwEffects = 0;
		break;
	case E_UNDERLINE:
		format.dwMask = CFM_UNDERLINETYPE;
		format.bUnderlineType = CFU_UNDERLINE;
		break;
	case E_UNUNDERLINE:
		format.dwMask = CFM_UNDERLINETYPE;
		format.bUnderlineType = CFU_UNDERLINENONE;
		break;
	case E_ITALIC:
		format.dwMask = CFM_ITALIC;
		format.dwEffects = CFE_ITALIC;
		break;
	case E_UNITALIC:
		format.dwMask = CFM_ITALIC;
		format.dwEffects = 0;
		break;
	case E_PLAIN:
		format.dwMask = CFM_BOLD | CFM_UNDERLINE | CFM_UNDERLINETYPE | CFM_ITALIC;
		format.dwEffects = 0;
		format.bUnderlineType = CFU_UNDERLINENONE;
		break;
	}
	
	GetRichEditCtrl().SetSelectionCharFormat(format);
}

void CDisplayFormatter::FontFormat(const char* font)
{
	long start, stop;
	
	GetTextDisplay().GetSelectionRange(start,stop);
	FontFormat(font, start, stop);
}


void CDisplayFormatter::FontFormat(const char* font, SInt32 start, SInt32 stop)
{
	GetTextDisplay().SetSelectionRange(start, stop);
	CHARFORMAT format;
	
	format.dwMask = CFM_FACE | CFM_CHARSET;
	format.dwEffects = 0;
	format.yHeight = 0;
	format.yOffset = 0;
	if (CFontPopup::GetInfo(font, &(format.bCharSet), &(format.bPitchAndFamily)))
	{
	}
	else
	{
		for(int i=0; i < strlen(font); i++)
		{
			if (font[i] == '_')
				((char*) font)[i] = ' ';
		}
		CFontPopup::GetInfo(font, &(format.bCharSet), &(format.bPitchAndFamily));
	}
	
	::lstrcpyn(format.szFaceName, cdstring(font).win_str(), LF_FACESIZE);
	format.szFaceName[LF_FACESIZE - 1] = 0;
    GetRichEditCtrl().SetSelectionCharFormat(format);
}


void CDisplayFormatter::ColorFormat(RGBColor color)
{
	long start, stop;
	
	GetTextDisplay().GetSelectionRange(start,stop);
	ColorFormat(color, start, stop);
}

void CDisplayFormatter::ColorFormat(RGBColor color, SInt32 start, SInt32 stop)
{
	GetTextDisplay().SetSelectionRange(start, stop);
	
	CHARFORMAT format;
	format.dwEffects = 0;
	format.dwMask = CFM_COLOR;
	format.crTextColor = color;
	GetRichEditCtrl().SetSelectionCharFormat(format);
}


void CDisplayFormatter::FontSizeFormat(short size, bool adding, bool overrideprefs)
{
	long start, stop;
	
	GetTextDisplay().GetSelectionRange(start, stop);
	FontSizeFormat(size, adding, start, stop, overrideprefs);
}


void CDisplayFormatter::FontSizeFormat(short size, bool adding, SInt32 start, SInt32 stop, bool overrideprefs)
{
	CHARFORMAT format;
	format.dwMask = CFM_SIZE;

	int ht;
	int pt_size;
	
	if (adding)
	{
		CHARFORMAT currentFormat;
		currentFormat.dwMask = CFM_SIZE;
		bool accumulate = false;
		int acc_start = start;
		int last_ht = 0;
		for(int i = start; i < stop; i++)
		{
			GetTextDisplay().SetSelectionRange(i, i + 1);
			GetRichEditCtrl().GetSelectionCharFormat(currentFormat);
			ht = currentFormat.yHeight;
			if (ht == last_ht)
				accumulate = true;
			else if (accumulate)
			{
				// Do format for previous run
				GetTextDisplay().SetSelectionRange(acc_start, i);
				format.yHeight = last_ht + size * 20;
				GetRichEditCtrl().SetSelectionCharFormat(format);
				accumulate = false;
				acc_start = i;
			}
			last_ht = ht;
		}
		
		// Do remaining
		if (accumulate)
		{
			// Do format for previous run
			GetTextDisplay().SetSelectionRange(acc_start, stop);
			format.yHeight = ht + size * 20;
			GetRichEditCtrl().SetSelectionCharFormat(format);
		}
	}
	else
	{
		pt_size = size;

		// Limit size to preference minimum
		if (!overrideprefs && (pt_size < CPreferences::sPrefs->mMinimumFont.GetValue()))
			pt_size = CPreferences::sPrefs->mMinimumFont.GetValue();

		format.yHeight = pt_size * 20;

		GetTextDisplay().SetSelectionRange(start, stop);
		GetRichEditCtrl().SetSelectionCharFormat(format);
	}
}

void CDisplayFormatter::AlignmentFormat(ETag tagid)
{
	long start, stop;
	
	GetTextDisplay().GetSelectionRange(start, stop);
	AlignmentFormat(tagid, start, stop);
}


void CDisplayFormatter::AlignmentFormat(ETag tagid, SInt32 start, SInt32 stop)
{
	
	GetTextDisplay().SetSelectionRange(start, stop);
	PARAFORMAT format;
	
	format.dwMask = PFM_ALIGNMENT;
	
	switch(tagid)
	{
	case E_CENTER:
		format.wAlignment = PFA_CENTER;
		break;
	case E_FLEFT:
		format.wAlignment = PFA_LEFT;
		break;
	case E_FRIGHT:
		format.wAlignment = PFA_RIGHT;
		break;
	case E_FBOTH:
		format.wAlignment = PFA_LEFT;
		break;
	}
	GetRichEditCtrl().SetParaFormat(format);
}

void CDisplayFormatter::ExcerptFormat(char Excerpt, SInt32 start, SInt32 stop)
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

bool CDisplayFormatter::GetLineRange(SInt32 &first, SInt32 &last, SInt32 start, SInt32 stop)
{
	/*int line;
	long lineStart, lineEnd;
	
	first = last = -1;
	
	int i = start;
	
	while(i < stop){
		line = WEOffsetToLine(i, mWEReference);
		if (first < 0)
			first = line;
		last = line;
		
		WEGetLineRange(line, &lineStart, &lineEnd);
		i = lineEnd + 1;
	}
	if (first >= 0 && last >= 0){
		return true;
	}
	else
		return false;*/
	return false;
}

DWORD CDisplayFormatter::GetContinuousCharSelection(CHARFORMAT &currentFormat)
{
	return GetRichEditCtrl().GetSelectionCharFormat(currentFormat);
}

DWORD CDisplayFormatter::GetContinuousParaSelection(PARAFORMAT &currentFormat)
{
	return GetRichEditCtrl().GetParaFormat(currentFormat);
}

	
CClickElement* CDisplayFormatter::FindCursor(int po)
{
	return (mClickList ? mClickList->findCursor(po) : false);
}

bool CDisplayFormatter::LaunchURL(const cdstring& url)
{
	TCHAR dir[MAX_PATH];
	if (::GetCurrentDirectory(MAX_PATH, dir))
	{
		HINSTANCE hinst = ::ShellExecute(*::AfxGetMainWnd(), _T("open"), url.win_str(), NULL, dir, SW_SHOWNORMAL);
		
		return (((int) hinst) > 32);
	}
	else
		return false;
}

bool CDisplayFormatter::DoAnchor(const CClickElement* anchor)
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
		
		// Scroll target element text position to top of screen
		GetRichEditCtrl().LineScroll(GetRichEditCtrl().LineFromChar(target->getStart()) - GetRichEditCtrl().GetFirstVisibleLine(), 0);
		
		return true;
	}
	
	return false;
}

int CDisplayFormatter::CharFromPos(CPoint pt) const
{
	 HWND m_hWnd;
	 if (mLetterText)
	 	m_hWnd = mLetterText->m_hWnd;
	 else if (mMessageText)
	 	m_hWnd = mMessageText->m_hWnd;
	 
	 ASSERT(::IsWindow(m_hWnd));
	 int me = ::SendMessage(m_hWnd, EM_CHARFROMPOS, (WPARAM)&me, (LPARAM)&pt); 
	 return me;
}

void CDisplayFormatter::DrawQuotation(long start, long stop, long depth)
{
	FaceFormat(E_PLAIN, start, stop);
	
	if (depth == 1)
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

void CDisplayFormatter::DoQuotation()
{
	// Don't bother if no quotes to recognize
	if (!CPreferences::sPrefs->mRecognizeQuotes.GetValue().size())
		return;

	// Get number of lines and create line info array
	int lines = GetRichEditCtrl().GetLineCount();
	auto_ptr<SLineInfo> info(new SLineInfo[lines]);

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
		int astart, astop;
		astart = GetRichEditCtrl().LineIndex(i);
		astop = GetRichEditCtrl().LineIndex(i + 1);
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
				
long CDisplayFormatter::GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes)
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