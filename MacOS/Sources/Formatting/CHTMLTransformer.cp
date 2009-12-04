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


#include "CHTMLTransformer.h"

#include "CATSUIStyle.h"
#include "CCharSpecials.h"
#include "CClickList.h"
#include "CHTMLCharacterEntity.h"
#include "CPreferences.h"
#include "CParserHTML.h"
#include "CParserPlain.h"
#include "CPluginManager.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CUStringUtils.h"
#include "CUTF8.h"

#pragma mark ________________________________CHTMLUtils

// UTF8 in, UTF8 out
char* CHTMLUtils::ConvertToEnriched(const char* htmlText)
{
	std::ostrstream output;
	bool runCR = false;

	int CurrentSize;
	int pre = 0;

	CFontStack fontStack;

	switch(CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.size)
	{
	case 9:
		CurrentSize = 2;
		break;
	case 10:
		CurrentSize = 3;
		break;
	case 12:
		CurrentSize = 4;
		break;
	case 14:
		CurrentSize = 5;
		break;
	case 18:
		CurrentSize = 6;
		break;
	case 24:
		CurrentSize = 7;
		break;
	default:
		CurrentSize = 3;
		break;
	}

	const char* p = htmlText;
	while(*p != 0)
	{
		if (*p == '<')
		{
			p++;
			cdstring token;
			for(; (*p != 0) && (*p != '>'); p++)
				token += (char)tolower(*p);

			if (*p == 0)
				break;

			if (!::strcmpnocase(token.c_str(), "p") || !::strcmpnocase(token.c_str(), "br"))
			{
				if (!runCR)
				{
					output << os_endl2;
					runCR = true;
				}
				else
					output << os_endl;
				p++;
				continue;
			}
			else
			{
				runCR = false;
			}

			if (!::strcmpnocase(token.c_str(), "pre"))
			{
				pre++;
				output << "<nofill>";
			}
			else if (!::strcmpnocase(token.c_str(), "/pre"))
			{
				pre--;
				output << "</nofill>";
			}
			else if (!::strcmpnocase(token.c_str(), "b"))
			{
				output << "<bold>";
			}
			else if (!::strcmpnocase(token.c_str(), "/b"))
			{
				output << "</bold>";
			}
			else if (!::strcmpnocase(token.c_str(), "i"))
			{
				output << "<italic>";
			}
			else if (!::strcmpnocase(token.c_str(), "/i"))
			{
				output << "</italic>";
			}
			else if (!::strcmpnocase(token.c_str(), "u"))
			{
				output << "<underline>";
			}
			else if (!::strcmpnocase(token.c_str(), "/u"))
			{
				output << "</underline>";
			}
			else if (!::strcmpnocase(token.c_str(), "tt"))
			{
				output << "<fixed>";
			}
			else if (!::strcmpnocase(token.c_str(), "/tt"))
			{
				output << "</fixed>";
			}
			else if (!::strcmpnocase(token.c_str(), "blockquote"))
			{
				output << "<excerpt>";
			}
			else if (!::strcmpnocase(token.c_str(), "/blockquote"))
			{
				output << "</excerpt>";
			}
			else if (!::strcmpnocase(token.c_str(), "center"))
			{
				output << "<center>";
			}
			else if (!::strcmpnocase(token.c_str(), "/center"))
			{
				output << "</center>";
			}
			else if (!::strncmpnocase(token.c_str(), "font", 4))
			{
				ConvertFontTag(output, token.c_str() + 4, CurrentSize, fontStack);
			}
			else if (!::strcmpnocase(token.c_str(), "/font"))
			{
				if (!fontStack.empty())
				{
					SFontParam element(fontStack.back());
					fontStack.pop_back();

					if (element.mFace)
					{
						output << "</fontfamily>";
					}
					if (element.mColor)
					{
						output << "</color>";
					}
					int theSize = element.mSize;

					if (theSize < 0)
					{
						for(int l = 0; l > theSize; l--)
						{
							output << "</smaller>";
							CurrentSize++;
						}
					}

					if (theSize > 0)
					{
						for(int l = 0; l < theSize; l++)
						{
							output << "</bigger>";
							CurrentSize--;
						}
					}
				}
			}
		}

		else if (*p == '&')
		{
			p++;
			const char* start = p;
			for(; (*p != 0) && (*p != ';') && (*p != ' ') && (*p != lendl1); p++)
			{
			}
			const char* stop = p;
			if (*p == ';')
				p++;

			cdstring format(start, stop - start);

			CParserHTML::HandleAmpCharUTF8(format, &output, NULL);

			p--;

			runCR = false;
		}
		else if ((*p == ' ') && (p[1] == ' '))
		{
			// Ignore space runs
			output << *p;
			while(*++p == ' ') {}
			p--;
		}
		else
		{
			if ((*p != lendl1) || ((os_endl_len == 2) && (*p != lendl2)))
			{
				output << *p;
				runCR = false;
			}
		}
		p++;
	}

	output << std::ends;
	return output.str();
}

// UTF8 in, UTF8 out
char* CHTMLUtils::ToHTML(const char* text)
{
	// Convert to utf16 then convert it
	cdustring utf16(text);
	return ToHTML(utf16);
}

// UTF16 in, UTF8 out
char* CHTMLUtils::ToHTML(const unichar_t* text)
{
	// Convert text into html escaped text
	std::ostrstream output;
	Write(output, text, ::unistrlen(text));

	output << std::ends;
	return output.str();
}

// UTF16 in, UTF8 out
void CHTMLUtils::Write(std::ostrstream& sout, const unichar_t* string, size_t length)
{
	i18n::CUTF8 utf8_converter;

	const unichar_t* p = string;
	bool space_run = false;

	while(length--)
	{
		if (space_run && (*p != ' '))
			space_run = false;

		if (*p == '&')
			sout << "&amp;";

		else if (*p == '<')
			sout << "&lt;";

		else if (*p == '>')
			sout << "&gt;";

		else if (*p == '\"')
			sout << "&quot;";
		
		else if ((*p < 0x0100) && !cUSASCIIChar[*p])
		{
			bool found = false;
			if (*p >= cStartCharacterEntity)
			{
				// Scan map for matching character
				for(const SMapHTML* map = cHTMLCharacterEntity; *map->html; map++)
				{
					// Must be case sensitive compare
					if (*p == map->mapped)
					{
						found = true;
						sout << "&" << map->html << ";";
						break;
					}
				}
			}
			
			if (!found)
				sout << "&#" << *p << ';';
		}
		else if (*p >= 0x80)
		{
			// Do utf8 encoding of utf16 character
			char buffer[16];
			int count = utf8_converter.w_2_c(*p, buffer);
			sout.write(buffer, count);
		}
		
		// Look for start of space run
		else if ((*p == ' ') && !space_run)
		{
			// Look for start of space run
			if (length && (*(p+1) == ' '))
			{
				space_run = true;
				sout << "&nbsp;";
			}
			else
				sout << ' ';
		}
		// Look for start of space run
		else if ((*p == ' ') && space_run)
			sout << "&nbsp;";

		else if ((*p == '\t'))
		{
			for(int i = 0; i < CPreferences::sPrefs->spaces_per_tab.GetValue(); i++)
				sout << "&nbsp;";
		}
		else if (*p == '\r')
		{
			sout << "<br>\r";
		}
		else
			sout << (char)*p;

		p++;
	}
}

// UTF8 in, UTF8 out
void CHTMLUtils::ConvertFontTag(std::ostrstream& output, const char* param, int& size, CFontStack& myStack)
{
	bool done = false;
	cdstring colorStr;
	cdstring sizeStr;
	cdstring faceStr;
	int addSize = 0;

	for( ; strlen(param) > 0 && !done; param++)
	{
		if (!::strncmpnocase(param, "color", 5))
		{
			param += 5;
			param += CParserHTML::FindParamUTF8(param, colorStr) - 1;
		}

		if (!::strncmpnocase(param, "size", 4))
		{
			param += 4;
			param += CParserHTML::FindParamUTF8(param, sizeStr) - 1;
		}

		if (!::strncmpnocase(param, "face", 4))
		{
			param += 4;
			param += CParserHTML::FindParamUTF8(param, faceStr, true) - 1;
		}
	}

	if (!colorStr.empty())
	{
		const char* tmpString = colorStr.c_str();
		if (tmpString[0] == '#')
		{
			tmpString++;

			output << "<color><param>";

			long value = ::strtol(tmpString, NULL, 16);
			int r = (value & 0x00FF0000);
			r = (r >> 8) | (r >> 16);
			int g = (value & 0x0000FF00);
			g = g | (g >> 8);
			int b = (value & 0x000000FF);
			b = b | (b << 8);

			char colString[16];

			::snprintf(colString, 16, "%.4x,%.4x,%.4x", r, g, b);

			output << colString;
			output << "</param>";
		}
	}

	if (!sizeStr.empty())
	{

		int Amount = (int) ::strtol(sizeStr.c_str(), NULL, 10);

		if (sizeStr[0UL] == '+')
		{
			for(int k = 0; k < Amount; k++)
			{
				output << "<bigger>";
				addSize++;
			}
		}
		else if
		(sizeStr[0UL] == '-')
		{
			for(int k = 0; k > Amount; k--)
			{
				output << "<smaller>";
				addSize--;
			}
		}

		else
		{
			if (size > Amount)
			{
				for(int k = size; k > Amount; k--)
				{
					output << "<smaller>";
					addSize--;
				}
			}
			else if (size < Amount)
			{
				for(int k = size; k < Amount; k++)
				{
					output << "<bigger>";
					addSize++;
				}
			}
		}

		size += addSize;
	}

	if (!faceStr.empty())
	{
		char* tmpStr = ::strtok((char*) faceStr.c_str(), ",");

		output << "<fontfamily><param>";
		output << tmpStr;
		output << "</param>";
	}

	myStack.push_back(CHTMLUtils::SFontParam(!faceStr.empty(), addSize, !colorStr.empty()));
}

#pragma mark ________________________________CHTMLTransformer

CHTMLTransformer::CHTMLTransformer(CTextDisplay* textPane)
{
	mTextPane = textPane;
	mStack = new CParserHTMLStack;
	mCurrentSize = CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.size;
	const unsigned char* myFont = CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.fontName;
	GetFNum(myFont, &mDefaultFont);
}

CHTMLTransformer::~CHTMLTransformer()
{
	delete mStack;
	mStack = NULL;
}

char* CHTMLTransformer::Transform()
{
	int runNumber = 0;
	SInt32 runExpires = -1;

	int continuos_length = 0;			// Length of continuosly styled text

	cdustring utxt;
	mTextPane->GetText(utxt);

	unichar_t* p = utxt;					// Start of next continuos style run
	unichar_t* q = utxt;					// Next character to examine

	size_t length = utxt.length();

	// Use plain parser to get URLs
	CClickList clicks;
	CParserPlain parser(p, NULL, &clicks, false);
	for(int j = 0; j < CPreferences::sPrefs->mRecognizeURLs.GetValue().size(); j++)
		parser.AddURL(CPreferences::sPrefs->mRecognizeURLs.GetValue()[j].c_str());
	parser.LookForURLs(0);
	CClickElement* target = clicks.front();
	bool inside_url = false;
	CParserHTMLStackElement* ahref = NULL;

	// Must add HTML start tags
	mOut << "<HTML>\r\r<BODY>\r\r";

	for(int i = 0; i < length; i++)
	{
		// Look for end of A HREF before setting other tags
		if (target && (target->getStop() == i))
		{
			// Force output to buffer
			if (continuos_length)
				CHTMLUtils::Write(mOut, p, continuos_length);
			p = q;
			continuos_length = 0;

			// Terminate the A tag with proper stack unwind/rewind
			RemoveAnchor();
			delete ahref;
			ahref = NULL;
			
			// Pop the click element and delete it
			clicks.pop_front();
			delete target;
			target = clicks.front();

			inside_url = false;
		}

		// Check for end of existing style run
		if (i >= runExpires)
		{
			// Flush existing text to output stream as style changing
			if (continuos_length)
				CHTMLUtils::Write(mOut, p, continuos_length);

			// Get next run details
			ATSUStyle style;
			UniCharArrayOffset runStart;
			UniCharCount runLength;
			::ATSUGetRunStyle(mTextPane->GetTextLayout(), i, &style, &runStart, &runLength);
			runExpires = runStart + runLength;
		
			// Copy style as we might modify it
			CATSUIStyle styles;
			::ATSUCopyAttributes(style, styles.Get());

			// Special check for spelling - remove spelling style if present
			if (styles.IsSpelling())
			{
				styles.SetUnderline(false);
			}

			// Now examine style differences
			ExamineStack(styles);

			// Reset text range
			p = q;
			continuos_length = 0;
		}

		// Check for A HREF after setting other tags
		if (target && (target->getStart() == i))
		{
			// Force output to buffer
			if (continuos_length)
				CHTMLUtils::Write(mOut, p, continuos_length);
			p = q;
			continuos_length = 0;

			// Output the A HREF tag with the URL
			cdustring url(utxt, target->getStart(), target->getStop() - target->getStart());
			ahref = new CParserHTMLStackElement(E_ANCHOR, -1);
			ahref->mParam = url;
			AddElement(ahref);
			mStack->push(ahref);
			
			inside_url = true;
		}

		// Replace CR's with <BR>CR
		if (*q == '\r')
		{
			// Force output to buffer with extra CR
			if (continuos_length)
				CHTMLUtils::Write(mOut, p, continuos_length);
			mOut << "<br>\r";
			p = q + 1;
			continuos_length = -1;
		}

		// Bump up range length
		continuos_length++;

		// Always bump char ptr
		q++;
	}

	// Flush remaining text to output stream as style ends (there will always be one left at the end)
	if (continuos_length)
		CHTMLUtils::Write(mOut, p, continuos_length);

	// Flush remaining tags
	FlushStack();

	// Must add HTML stop tags
	mOut << os_endl2 << "</BODY>" << os_endl2 << "</HTML>" << os_endl << std::ends;
	return mOut.str();
}

void CHTMLTransformer::FlushStack()
{
	CParserHTMLStackElement* currElement = mStack->pop();

	while(currElement)
	{
		RemoveElement(currElement);
		delete currElement;
		currElement = mStack->pop();
	}
}

void CHTMLTransformer::ExamineStack(const CATSUIStyle& styles)
{
	CParserHTMLStack Pile;
	bool found = false;
	bool bolding = false;
	bool italicing = false;
	bool underlining = false;
	bool aligning = false;
	bool fonting = false;

	if (!mStack)
		mStack = new CParserHTMLStack();

	// Reverse current stack
	CParserHTMLStackElement* currElement = mStack->pop();
	while(currElement)
	{
		Pile.push(currElement);
		currElement = mStack->pop();
	}

	// Now scan for first difference, pushing back that that are the same
	currElement = Pile.pop();
	bool matched = true;
	while(currElement && matched)
	{
		// Mark if styles match
		switch(currElement->mTagid)
		{
		case E_BOLD:
			if (styles.GetStyle() & bold)
			{
				bolding = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_ITALIC:
			if (styles.GetStyle() & italic)
			{
				italicing = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_UNDERLINE:
			if (styles.GetStyle() & underline)
			{
				underlining = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_FONT:
		{
			RGBColor color = styles.GetColor();
			if ((styles.GetFontNum() == ((CParserHTMLFontStackElement*) currElement)->GetFont()) &&
				(::Fix2Long(styles.GetFontSize()) == ((CParserHTMLFontStackElement*) currElement)->GetSize()) &&
				(color.red == ((CParserHTMLFontStackElement*) currElement)->GetRed()) &&
				(color.green == ((CParserHTMLFontStackElement*) currElement)->GetGreen()) &&
				(color.blue == ((CParserHTMLFontStackElement*) currElement)->GetBlue()))
			{
				fonting = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		}
		case E_ANCHOR:
			// Anchors handled elsewhere
			mStack->push(currElement);
			break;
		default:;
		}

		if (matched)
			currElement = Pile.pop();
		else
			// Must pop back the one that's mis-matched
			Pile.push(currElement);
	}

	// Now reverse the Pile
	CParserHTMLStack Pile2;
	currElement = Pile.pop();
	while(currElement)
	{
		Pile2.push(currElement);
		currElement = Pile.pop();
	}

	// Now remove all tags but add those that match back into pile
	currElement = Pile2.pop();
	while(currElement)
	{
		matched = false;
		switch(currElement->mTagid)
		{
		case E_BOLD:
			if (styles.GetStyle() & bold)
			{
				Pile.push(currElement);
				bolding = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_ITALIC:
			if (styles.GetStyle() & italic)
			{
				Pile.push(currElement);
				italicing = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_UNDERLINE:
			if (styles.GetStyle() & underline)
			{
				Pile.push(currElement);
				underlining = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_FONT:
		{
			RGBColor color = styles.GetColor();
			if ((styles.GetFontNum() == ((CParserHTMLFontStackElement*) currElement)->GetFont()) &&
				(::Fix2Long(styles.GetFontSize()) == ((CParserHTMLFontStackElement*) currElement)->GetSize()) &&
				(color.red == ((CParserHTMLFontStackElement*) currElement)->GetRed()) &&
				(color.green == ((CParserHTMLFontStackElement*) currElement)->GetGreen()) &&
				(color.blue == ((CParserHTMLFontStackElement*) currElement)->GetBlue()))
			{
				Pile.push(currElement);
				fonting = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		}
		case E_ANCHOR:
			// Anchors handled elsewhere
			Pile.push(currElement);
			matched = true;
			RemoveElement(currElement);
			break;
		default:;
		}

		// If element no longer used, delete it
		if (!matched)
			delete currElement;

		currElement = Pile2.pop();
	}

	// Now add back tags that still exist
	currElement = Pile.pop();
	while(currElement)
	{
		AddElement(currElement);
		mStack->push(currElement);
		currElement = Pile.pop();
	}

	// Now add new tags
	// Do font and alignment before the rest to minimise stack unwind/rewind
	if (!fonting)
	{
		currElement = new CParserHTMLFontStackElement(E_FONT, -1, styles.GetColor(), ::Fix2Long(styles.GetFontSize()), styles.GetFontNum());
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((styles.GetStyle() & bold) && !bolding)
	{
		currElement = new CParserHTMLStackElement(E_BOLD, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((styles.GetStyle() & italic) && !italicing)
	{
		currElement = new CParserHTMLStackElement(E_ITALIC, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((styles.GetStyle() & underline) && !underlining)
	{
		currElement = new CParserHTMLStackElement(E_UNDERLINE, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

}

void CHTMLTransformer::AddElement(CParserHTMLStackElement* element)
{
	switch(element->mTagid)
	{
	case E_BOLD:
		mOut << "<b>";
		break;
	case E_ITALIC:
		mOut << "<i>";
		break;
	case E_UNDERLINE:
		mOut << "<u>";
		break;
	case E_FONT:
		{
			CParserHTMLFontStackElement* felement = static_cast<CParserHTMLFontStackElement*>(element);
			Str255 fname;
			bool myFonting = false;
			bool mySizing = false;
			bool myColoring = false;

			int mySize;


			if (felement->mFont != mDefaultFont)
			{
				::GetFontName(felement->mFont, fname);
				myFonting = true;
			}

			if (felement->mSize != 10)
			{
				mySizing = true;

				switch(felement->mSize)
				{
				case 9:
					mySize = 2;
					break;
				case 10:
					mySize = 3;
					break;
				case 12:
					mySize = 4;
					break;
				case 14:
					mySize = 5;
					break;
				case 18:
					mySize = 6;
					break;
				case 24:
					mySize = 7;
					break;
				default:
					mySize = 3;
					break;
				}
			}


			if ((felement->GetRed() != 0) ||
				(felement->GetGreen() != 0) ||
				(felement->GetBlue() != 0))
				myColoring = true;

			if (myFonting || mySizing || myColoring)
			{
				mOut << "<font";

				if (myFonting)
				{
					mOut << " face=\"";
					mOut << cdstring(fname);
					mOut << "\"";
				}

				if (mySizing)
				{
					mOut << " size=";
					mOut << mySize;
				}

				if (myColoring)
				{
					char myColor[8];

					mOut << " color=\"#";
					::snprintf(myColor, 8, "%.2x%.2x%.2x", felement->GetRed() / 256,
													  felement->GetGreen() / 256,
													  felement->GetBlue() / 256);
					mOut << myColor;
					mOut << "\"";
				}


				mOut << ">";
			}
		}
		break;
	case E_CENTER:
		mOut << "<center>";
		break;
	case E_ANCHOR:
		mOut << "<A HREF=\"" << element->mParam.ToUTF8() << "\">";
		break;
	default:;
	}
}

void CHTMLTransformer::RemoveElement(CParserHTMLStackElement* element)
{
	switch(element->mTagid)
	{
	case E_BOLD:
		mOut << "</b>";
		break;
	case E_ITALIC:
		mOut << "</i>";
		break;
	case E_UNDERLINE:
		mOut << "</u>";
		break;
	case E_FONT:
		mOut << "</font>";
		break;
	case E_CENTER:
		mOut << "</center>";
		break;
	case E_ANCHOR:
		mOut << "</A>";
		break;
	default:;
	}
}

void CHTMLTransformer::RemoveAnchor()
{
	CParserHTMLStack Pile;

	// Remove all tags down to the anchor
	CParserHTMLStackElement* currElement = mStack->pop();
	while(currElement)
	{
		if (currElement->mTagid == E_ANCHOR)
		{
			// Terminate anchor tag and force exit from loop
			RemoveElement(currElement);
			break;
		}
		else
		{
			// Terminate tag and add to pile
			RemoveElement(currElement);
			Pile.push(currElement);
		}
		currElement = mStack->pop();
	}

	// Now add back tags that still exist
	currElement = Pile.pop();
	while(currElement)
	{
		AddElement(currElement);
		mStack->push(currElement);
		currElement = Pile.pop();
	}
}
