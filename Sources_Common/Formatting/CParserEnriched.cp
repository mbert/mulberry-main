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


#include "CParserEnriched.h"

#include "CEnrichedTransformer.h"
#include "CPreferences.h"
#include "CStringUtils.h"

#include <memory>
#include <strstream>

CParserEnriched::CParserEnriched(const unichar_t* st, CFormatList* format, bool use_styles)
{
	InitParserEnriched();

	mTxt = ::unistrdup(st);
#if __dest_os == __win32_os
	// Must remove \n as RichEdit2.0 only uses \r
	::FilterOutLFs(mTxt);
#endif	
	mFormatList = format;
	mUseStyles = use_styles;
}

CParserEnriched::CParserEnriched(CFormatList* format)
{
	InitParserEnriched();

	mFormatList = format;
}

CParserEnriched::CParserEnriched()
{
	InitParserEnriched();
}

CParserEnriched::~CParserEnriched()
{
	//delete mFormatList;
	delete mTxt;
	delete mStack;
	mStack = NULL;
}

void CParserEnriched::InitParserEnriched()
{
	mFormatList = NULL;
	mUseStyles = true;
	mDoAlignment = true;
	mTxt = NULL;
	mStack = new CParserEnrichedStack;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mStyledFontSize = CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.size;
	mFixedFontSize = CPreferences::sPrefs->mFixedTextTraits.GetValue().traits.size;
	mFixedFontName = CPreferences::sPrefs->mFixedTextTraits.GetValue().traits.fontName;
#elif __dest_os == __win32_os
	HDC hDC = ::GetDC(NULL);
	int logpix = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if (logpix == 0)
		logpix = 1;
	::ReleaseDC(NULL, hDC);

	LOGFONT fontInfo = CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().logfont;
	int ht = fontInfo.lfHeight;
	int pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mStyledFontSize = pt_size;

	fontInfo = CPreferences::sPrefs->mFixedTextFontInfo.GetValue().logfont;
	ht = fontInfo.lfHeight;
	pt_size = ((2*-72*ht/logpix) + 1)/2;	// Round up to nearest int
	mFixedFontSize = pt_size;
	mFixedFontName = fontInfo.lfFaceName;
#elif __dest_os == __linux_os
	mStyledFontSize = CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().size;
	mFixedFontSize = CPreferences::sPrefs->mFixedTextFontInfo.GetValue().size;
	mFixedFontName = CPreferences::sPrefs->mFixedTextFontInfo.GetValue().fontname;
#else
#error __dest_os
#endif
	mBaseFontSize = 3;
}

void CParserEnriched::SetFontScale(long scale)
{
	mStyledFontSize = std::max(1.0, pow(1.2, scale) * mStyledFontSize);
	mFixedFontSize = std::max(1.0, pow(1.2, scale) * mFixedFontSize);
}

void CParserEnriched::DrawTag(int start, int stop)
{
	// Only if something present
	if (start == stop)
		return;

	mFormatList->addElement(new CColorFormatElement(start, stop, CPreferences::sPrefs->mTagStyle.GetValue().color));

	if (CPreferences::sPrefs->mTagStyle.GetValue().style & bold)
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_BOLD));

	if (CPreferences::sPrefs->mTagStyle.GetValue().style & underline)
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_UNDERLINE));

	if (CPreferences::sPrefs->mTagStyle.GetValue().style & italic)
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_ITALIC));
}

void CParserEnriched::FlushStack(int index)
{
	std::auto_ptr<CParserEnrichedStackElement> element(mStack->pop());

	while(element.get() != NULL)
	{
		// Only do styles if requested by user
		if (mUseStyles)
		{
			if (element->mStart < index)
			{
				switch(element->mTagid)
				{
				case E_BOLD:
				case E_UNDERLINE:
				case E_ITALIC:
					mFormatList->addElement(new CFaceFormatElement(element->mStart, index, element->mTagid));
					break;
				case E_BIGGER:
				case E_SMALLER:
					{
						mBaseFontSize += (element->mTagid == E_BIGGER) ? 1 : -1;
						if (mBaseFontSize > 7)
							mBaseFontSize = 7;
						else if (mBaseFontSize <= 0)
							mBaseFontSize = 1;
						int Amount = mBaseFontSize;
						HandleScaledSize(element->mStart, index, Amount);
					}
					break;
				case E_CENTER:
				case E_FLEFT:
				case E_FRIGHT:
				case E_FBOTH:
					if (mDoAlignment)
						mFormatList->addElement(new CAlignmentFormatElement(element->mStart, index, element->mTagid));
					break;
				case E_FONT:
					{
						cdstring utf8 = element->mParam.ToUTF8();
						mFormatList->addElement(new CFontFormatElement(element->mStart, index, utf8));
					}
					break;
				case E_COLOR:
					{
						cdstring utf8 = element->mParam.ToUTF8();
						mFormatList->addElement(new CColorFormatElement(element->mStart, index, utf8));
					}
					break;
				case E_FIXED:
					{
						mFormatList->addElement(new CFontFormatElement(element->mStart, index, mFixedFontName));
						HandleScaledSize(element->mStart, index, 3, true);
					}
					break;
				case E_EXCERPT:
					mFormatList->addElement(new CExcerptFormatElement(element->mStart, index));
					break;
				default:;
				}
			}
		}

		element.reset(mStack->pop());
	}
}

ETag CParserEnriched::GetTag(unichar_t* format)
{
	if(!::unistrcmp(format, "bold"))
		return E_BOLD;
	else if(!::unistrcmp(format, "underline"))
		return E_UNDERLINE;
	else if(!::unistrcmp(format, "italic"))
		return E_ITALIC;
	else if(!::unistrcmp(format, "bigger"))
		return E_BIGGER;
	else if(!::unistrcmp(format, "smaller"))
		return E_SMALLER;
	else if(!::unistrcmp(format, "center"))
		return E_CENTER;
	else if(!::unistrcmp(format, "flushleft"))
		return E_FLEFT;
	else if(!::unistrcmp(format, "flushright"))
		return E_FRIGHT;
	else if(!::unistrcmp(format, "flushboth"))
		return E_FBOTH;
	else if(!::unistrcmp(format, "fontfamily"))
		return E_FONT;
	else if(!::unistrcmp(format, "color"))
		return E_COLOR;
	else if(!::unistrcmp(format, "fixed"))
		return E_FIXED;
	else if(!::unistrcmp(format, "excerpt"))
		return E_EXCERPT;
	else
		return E_NOTHING;
}

void CParserEnriched::HandleFormat(unichar_t* format, int index)
{
	if (format[0] == '/')
	{
		ETag tag = GetTag(format + 1);
		std::auto_ptr<CParserEnrichedStackElement> element(mStack->pop(tag));
		if (element.get() != NULL)
		{
			// Only do styles if requested by user
			if (mUseStyles)
			{
				if (element->mStart < index)
				{
					switch(tag)
					{
					case E_BOLD:
					case E_UNDERLINE:
					case E_ITALIC:
						mFormatList->addElement(new CFaceFormatElement(element->mStart, index, element->mTagid));
						break;
					case E_BIGGER:
						mFormatList->addElement(new CFontSizeFormatElement(element->mStart, index, 1));
						break;
					case E_SMALLER:
						mFormatList->addElement(new CFontSizeFormatElement(element->mStart, index, -1));
						break;
					case E_CENTER:
					case E_FLEFT:
					case E_FRIGHT:
					case E_FBOTH:
						if (mDoAlignment)
							mFormatList->addElement(new CAlignmentFormatElement(element->mStart, index, element->mTagid));
						break;
					case E_FONT:
						{
							cdstring utf8 = element->mParam.ToUTF8();
							mFormatList->addElement(new CFontFormatElement(element->mStart, index, utf8));
						}
						break;
					case E_COLOR:
						{
							cdstring utf8 = element->mParam.ToUTF8();
							mFormatList->addElement(new CColorFormatElement(element->mStart, index, utf8));
						}
						break;
					case E_FIXED:
						{
							mFormatList->addElement(new CFontFormatElement(element->mStart, index, mFixedFontName));
							HandleScaledSize(element->mStart, index, 3, true);
						}
						break;
					case E_EXCERPT:
						mFormatList->addElement(new CExcerptFormatElement(element->mStart, index));
						break;
					default:;
					}
				}
			}
		}
	}
	else
		mStack->push(new CParserEnrichedStackElement(GetTag(format), index));
}

void CParserEnriched::HandleScaledSize(long start, long stop, long relsize, bool fixed)
{
	// Only if something present
	if (start == stop)
		return;

	// Only do styles if requested by user
	if (mUseStyles)
	{
		// Use 20% increments
		int styled_size = std::max(1.0, pow(1.2, relsize - 3) * (fixed ? mFixedFontSize : mStyledFontSize));

		mFormatList->addElement(new CFontSizeFormatElement(start, stop, styled_size, false));
	}
}

const unichar_t* CParserEnriched::Parse(int offset, bool for_display, bool quote, bool forward)
{
	CParserEnrichedStackElement* tempElement;
	int finalCount = 0;
	int nofill = 0;
	unsigned long quote_depth = 0;

#if __dest_os == __win32_os
	// Windows RichEdit2.0 uses CR only not CRLF so that's what we use
	// here to ensure that formatting ranges stay in sync with displayed text
	cdustring plain_endl;
	plain_endl.append_ascii(for_display ? "\r" : os_endl);
	int plain_endl_len = for_display ? 1 : os_endl_len;
#else
	cdustring plain_endl;
	plain_endl.append_ascii(os_endl);
	int plain_endl_len = os_endl_len;
#endif
	cdustring quoted_endl = plain_endl;
	int quoted_endl_len = plain_endl_len;
	if (quote)
	{
		cdstring prefix_utf8 = forward ?
									CPreferences::sPrefs->mForwardQuote.GetValue() :
									CPreferences::sPrefs->mReplyQuote.GetValue();
		cdustring prefix(prefix_utf8);
		if (prefix.length() != 0)
		{
			quoted_endl += prefix;
			if (quoted_endl[quoted_endl.length() - 1] != ' ')
				quoted_endl += unichar_t(' ');
			quoted_endl_len = quoted_endl.length();
			
			// Always turn of alignment when quoting
			mDoAlignment = false;
		}
	}

	const unichar_t* end_line = plain_endl.c_str();
	int end_line_len = plain_endl.length();

	std::ostrstream out;

	const unichar_t* p = mTxt;

	if (p)
	{
		while(*p)
		{
			const unichar_t* start;
			const unichar_t* stop;

			if(*p == '<')
			{
				p++;
				if (*p == '<')
				{
					out.write((const char*)p, sizeof(unichar_t));
					finalCount++;
					p++;
				}
				else
				{
					start = p;
					while(*p && (*p != '>')) p++;
					stop = p;
					if (*p) p++;

					std::auto_ptr<unichar_t> format(::unistrndup(start, stop-start));
					if (format.get())
					{
						::unistrlower(format.get());
						if (!::unistrcmp(format.get(), "param"))
						{
							if (mFormatList)
								tempElement = mStack->pop();

							start = p;
							while(*p && (*p != '<')) p++;
							stop = p;
							p++;
							if (mFormatList)
							{
								tempElement->mParam = cdustring(start, stop-start);
								mStack->push(tempElement);
							}
							while(*p && (*p != '>')) p++;
							if (*p) p++;
						}
						else if(!::unistrcmp(format.get(), "nofill"))
							nofill++;
						else if(!::unistrcmp(format.get(), "/nofill"))
						{
							if (nofill > 0)
								nofill--;
						}
						else if(!::unistrcmp(format.get(), "excerpt"))
						{
							// Do formatting change before CR added
							if (mFormatList)
								HandleFormat(format.get(), finalCount + offset);

							// Switch to quote endls
							if (quote_depth++ == 0)
							{
								end_line = quoted_endl.c_str();
								end_line_len = quoted_endl.length();
							}

							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
						}
						else if(!::unistrcmp(format.get(), "/excerpt"))
						{
							if (mFormatList)
								HandleFormat(format.get(), finalCount + offset);

							// Switch to plain endls when depth goes to zero
							if ((quote_depth > 0) && (--quote_depth == 0))
							{
								end_line = plain_endl.c_str();
								end_line_len = plain_endl.length();
							}
						}
						else
						{
							if (mFormatList)
								HandleFormat(format.get(), finalCount + offset);
						}
					}
				}
			}

			else if (((*p == '\r') || (*p == '\n')) && (nofill == 0))
			{
				// Bump past CR in CRLF pair
				if ((*p == '\r') && (*(p+1) == '\n'))
					p++;

				// If single CR/LF output space, else drop if starting run
				if ((*(p+1) != '\n') && (*(p+1) != '\r') && (*(p+1) != 0))
				{
					unichar_t space = ' ';
					out.write((const char*)&space, sizeof(unichar_t));
					finalCount++;
				}
				p++;
				
				while((*p == '\r') || (*p == '\n'))
				{
					/// Output local line end
					out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
					finalCount += end_line_len;

					// Bump past CR in CRLF pair
					if ((*p == '\r') && (*(p+1) == '\n'))
						p++;
					
					p++;
				}
			}
			else
			{
				out.write((const char*)p, sizeof(unichar_t));
				p++;
				finalCount++;
			}
		}
	}

	if (mFormatList)
		FlushStack(finalCount + offset);
	out << std::ends << std::ends;
	return reinterpret_cast<const unichar_t*>(out.str());
}


void CParserEnriched::RawParse(int offset)
{
	const unichar_t* p = mTxt;
	if (!p)
		return;

	int currPos = offset;
	int startPos;
	int stopPos;

	while(*p)
	{
		const unichar_t* start;
		const unichar_t* stop;

		if (*p == '<')
		{
			p++;
			currPos++;
			if (*p == '<')
			{
				p++;
				currPos++;
			}
			else
			{
				start = p;
				startPos = currPos;

				while(*p && (*p != '>'))
				{
					p++;
					currPos++;
				}

				stop = p;
				stopPos = currPos;

				if (*p)
				{
					p++;
					currPos++;
				}

				std::auto_ptr<unichar_t> format(::unistrndup(start, stop-start));
				if (format.get())
				{
					::unistrlower(format.get());
					if(!::unistrcmp(format.get(), "param"))
					{
						start = p;

						while(*p && (*p != '<'))
						{
							p++;
							currPos++;
						}

						stop = p;

						p++;
						currPos++;

						while(*p && (*p != '>'))
						{
							p++;
							currPos++;
						}

						if (*p)
						{
							p++;
							currPos++;
						}

						if (mFormatList)
							DrawTag(startPos-1, currPos);
					}
					else
					{
						if (mFormatList)
							DrawTag(startPos-1, stopPos+1);
					}
				}
			}
		}


		else
		{
			currPos++;
			p++;
		}
	}
}
