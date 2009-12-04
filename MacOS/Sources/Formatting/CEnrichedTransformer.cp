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


#include "CEnrichedTransformer.h"

#include "CATSUIStyle.h"
#include "CCharSpecials.h"
#include "CHTMLCharacterEntity.h"
#include "CParserEnrichedStack.h"
#include "CPluginManager.h"
#include "CSpellPlugin.h"
#include "CStringUtils.h"
#include "CPreferences.h"
#include "CTextDisplay.h"
#include "CUStringUtils.h"
#include "CUTF8.h"

#pragma mark ________________________________CEnrichedUtils

// UTF8 in, UTF8 out
char* CEnrichedUtils::ConvertToHTML(const char* enrichedText)
{
	int nofill=0;
	std::ostrstream output;
	cdstring myParam;

	// Must add HTML start tags
	output << "<HTML>" << os_endl2 << "<BODY>" << os_endl2;

	cdstring font = "DEFAULT";
	int size = 3;
	cdstring color = "DEFAULT";

	cdstrvect colorStack;
	cdstrvect fontStack;
	int paramTag = 0;
	int fontCT = 0;

	bool change = false;
	bool paramGood = false;

	bool spaced = false;

	const char* p = enrichedText;
	while(*p != 0)
	{
		if (*p == '<')
		{
			p++;
			if (*p == '<')
			{
				output << "&lt;";
			}
			else
			{
				cdstring token;
				for(; (*p != 0) && (*p != '>'); p++)
					token += (char)tolower(*p);

				if (*p == 0)
					break;

				if (!::strcmp(token.c_str(), "/param"))
				{
					paramGood = false;
					if (paramTag == 1)
					{
						change = true;
						if (!font.empty())
							fontStack.push_back(font);
						font = myParam;
					}
					if (paramTag == 2)
					{
						change = true;
						if (!color.empty())
							colorStack.push_back(color);
						color = myParam;
					}

					myParam = cdstring::null_str;
				}

				else if (!::strcmp(token.c_str(), "param"))
				{
					if (paramTag == 1 || paramTag == 2)
					{
						paramGood = true;
					}
				}

				else{
					if (!::strcmpnocase(token.c_str(), "nofill"))
					{
						nofill++;
						output << "<pre>";
					}
					else if (!::strcmpnocase(token.c_str(), "/nofill"))
					{
						nofill--;
						output << "</pre>";
					}
					else if (!::strcmpnocase(token.c_str(), "bold"))
					{
						output << "<b>";
					}
					else if (!::strcmpnocase(token.c_str(), "/bold"))
					{
						output << "</b>";
					}
					else if (!::strcmpnocase(token.c_str(), "italic"))
					{
						output << "<i>";
					}
					else if (!::strcmpnocase(token.c_str(), "/italic"))
					{
						output << "</i>";
					}
					else if (!::strcmpnocase(token.c_str(), "underline"))
					{
						output << "<u>";
					}
					else if (!::strcmpnocase(token.c_str(), "/underline"))
					{
						output << "</u>";
					}
					else if (!::strcmpnocase(token.c_str(), "fixed"))
					{
						output << "<tt>";
					}
					else if (!::strcmpnocase(token.c_str(), "/fixed"))
					{
						output << "</tt>";
					}
					else if (!::strcmpnocase(token.c_str(), "excerpt"))
					{
						output << "<blockquote>";
					}
					else if (!::strcmpnocase(token.c_str(), "/excerpt"))
					{
						output << "</blockquote>";
					}
					else if (!::strcmpnocase(token.c_str(), "center"))
					{
						output << "<center>";
					}
					else if (!::strcmpnocase(token.c_str(), "/center"))
					{
						output << "</center>";
					}
					else if (!::strcmpnocase(token.c_str(), "bigger"))
					{
						if (size < 7)
						{
							size++;
							change = true;
						}
					}
					else if (!::strcmpnocase(token.c_str(), "/bigger"))
					{
						if (size > 1)
						{
							size--;
							change = true;
						}
					}

					else if (!::strcmpnocase(token.c_str(), "smaller"))
					{
						if (size > 1)
						{
							size--;
							change = true;
						}
					}
					else if (!::strcmpnocase(token.c_str(), "/smaller"))
					{
						if (size < 7)
						{
							size++;
							change = true;
						}
					}

					else if (!::strcmpnocase(token.c_str(), "fontfamily"))
					{
						paramTag = 1;

					}
					else if (!::strcmpnocase(token.c_str(), "/fontfamily"))
					{
						change = true;
						if (fontStack.empty())
							font = "DEFAULT";
						else
						{
							font = fontStack.back();
							fontStack.pop_back();
						}
					}
					else if (!::strcmpnocase(token.c_str(), "color"))
					{
						paramTag = 2;
					}
					else if (!::strcmpnocase(token.c_str(), "/color"))
					{
						change = true;
						if (colorStack.empty())
							color = "DEFAULT";
						else
						{
							color = colorStack.back();
							colorStack.pop_back();
						}
					}
				}
			}
		}

		else if (paramGood)
		{
			myParam += *p;
		}

		else if (change)
		{
			bool something = false;
			change = false;

			if (fontCT > 0)
			{
				output << "</font>";
				fontCT--;
			}

			fontCT++;

			if (!color.empty() && (::strcmp(color.c_str(), "DEFAULT") != 0))
			{
				long red = 0;
				long green = 0;
				long blue = 0;
				const char* col;
				if ((col = ::strtok((char*)color.c_str(), ",")) != NULL)
					red = strtol(col, NULL, 16);
				if ((col = ::strtok(NULL, ",")) != NULL)
					green = strtol(col, NULL, 16);
				if ((col = ::strtok(NULL, ",")) != NULL)
					blue = strtol(col, NULL, 16);

				if (!something)
				{
					output << "<font";
					something = true;
				}

				output << " color = \"#";
				char tmpColor[10];
				::snprintf(tmpColor, 10, "%.2x%.2x%.2x", red / 256, green / 256, blue / 256);
				output << tmpColor;
				output << "\"";
			}

			if (!font.empty() && (::strcmp(font.c_str(), "DEFAULT") != 0))
			{
				//cdstring fontName = CPreferences::sPrefs->mDisplayTextTraits.fontName;
				//if (font != fontName)
				{
					if (!something)
					{
						output << "<font";
						something = true;
					}

					output << " face = \"";
					output << font;
					output << "\"";
				}
			}

			if (size != 3)
			{
				if (!something)
				{
					output << "<font";
					something = true;
				}

				output << " size = ";
				output << size;
			}

			if (something)
			{
				output << ">";
			}
			p--;
		}

		else if (*p == '>')
		{
			output << "&gt;";
		}
		else if (*p == '&')
		{
			output << "&amp;";
		}
		else
		{
			if ((*p == lendl1) && ((os_endl_len == 1) || (*p == lendl2)) && nofill <= 0)
			{
				// If single CR output space, else drop if starting run
				if (p[os_endl_len] != lendl1)
				{
					output << (spaced ? "&nbsp;" : " ");
					spaced = true;
				}
				else
				{
					spaced = false;
					p += os_endl_len;

					while((*p == lendl1) && ((os_endl_len == 1) || (*p == lendl2)))
					{
						output << "<br>" << os_endl;
						p += os_endl_len;
					}
					p--;
				}
			}

			if (spaced && (*p == ' '))
			{
				output << "&nbsp;";
			}
			else if (*p == ' ')
			{
				spaced = true;
				output << " ";
			}
			else
			{
				WriteHTML(*p, output);
				spaced = false;
			}
		}
		p++;
	}

	for(int k = 0; k < fontCT; k++)
	{
		output << "</font>";
	}

	// Must add HTML stop tags
	output << os_endl2 << "</BODY>" << os_endl2 << "</HTML>" << os_endl;

	output << std::ends;
	return output.str();
}

// UTF8 in, UTF8 out
void CEnrichedUtils::WriteHTML(char ch, std::ostream &output)
{
	if (ch == '&')
		output << "&amp;";

	else if (ch == '<')
		output << "&lt;";

	else if (ch == '>')
		output << "&gt;";

	else if (ch == '\"')
		output << "&quot;";

	else
		// Just write it out - including raw utf8 bytes (i.e. no conversion
		// to HTML entities is done here)
		output << ch;
}

// UTF8 in, UTF8 out
char* CEnrichedUtils::ToEnriched(const char* text)
{
	// Convert to utf16 then convert it
	cdustring utf16(text);
	return ToEnriched(utf16);
}

// UTF16 in, UTF8 out
char* CEnrichedUtils::ToEnriched(const unichar_t* text)
{
	// Convert plain text blob to enriched
	std::ostrstream output;
	Write(output, text, ::unistrlen(text));
	
	output << std::ends;
	return output.str();
}

// UTF16 in, UTF8 out
void CEnrichedUtils::Write(std::ostrstream& sout, const unichar_t* string, size_t length)
{
	const unichar_t* p = string;
	bool got_endl = false;

	while(length--)
	{
		if (*p == '<')
		{
			sout << "<<";
			got_endl = false;
		}
		else if ((*p == '\r') && !got_endl)
		{
			// Double the first endl in a run
			sout << "\r\r";
			got_endl = true;
		}
		else
		{
			sout << (char)*p;
			got_endl = false;
		}

		p++;
	}
}

// UTF16 in, UTF8 out
void CEnrichedUtils::WriteNoEndlDouble(std::ostrstream& sout, const unichar_t* string, size_t length)
{
	const unichar_t* p = string;

	while(length--)
	{
		if (*p == '<')
			sout << "<<";
		else
			sout << (char)*p;

		p++;
	}
}

#pragma mark ________________________________CEnrichedTransformer

CEnrichedTransformer::CEnrichedTransformer(CTextDisplay* textPane)
{
	mTextPane = textPane;
	mStack = new CParserEnrichedStack;
	mCurrentSize = CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.size;
	const unsigned char *myFont = CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.fontName;
	GetFNum(myFont, &mDefaultFont);
}

CEnrichedTransformer::~CEnrichedTransformer()
{
	delete mStack;
	mStack = NULL;
}

char* CEnrichedTransformer::Transform()
{
	int runNumber = 0;
	SInt32 runExpires = -1;

	int continuos_length = 0;			// Length of continuosly styled text

	cdustring utxt;
	mTextPane->GetText(utxt);

	unichar_t* p = utxt;					// Start of next continuos style run
	unichar_t* q = utxt;					// Next character to examine

	size_t length = utxt.length();

	bool runCR = false;					// Keeps track of CR run

	for(UniCharArrayOffset i = 0; i < length; i++)
	{
		// Check for end of existing style run
		if (i >= runExpires)
		{
			// Flush existing text to output stream as style changing
			if (continuos_length)
				CEnrichedUtils::WriteNoEndlDouble(mOut, p, continuos_length);

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

			// Reset text range and CR runs
			p = q;
			continuos_length = 0;
			runCR = false;
		}

		// Must double up CRs
		if ((*q == '\r') && !runCR)
		{
			// Force output to buffer with extra CR
			if (continuos_length)
				CEnrichedUtils::WriteNoEndlDouble(mOut, p, continuos_length);
			mOut << os_endl2;
			p = q + 1;
			continuos_length = -1;
			runCR = true;
		}
		else if ((*q != '\r') && runCR)
			runCR = false;

		// Bump up range length
		continuos_length++;

		// Always bump char ptr
		q++;
	}

	// Flush remaining text to output stream as style ends (there will always be one left at the end)
	if (continuos_length)
		CEnrichedUtils::WriteNoEndlDouble(mOut, p, continuos_length);

	// Flush remaining tags
	FlushStack();

	mOut << os_endl << std::ends;
	return mOut.str();
}



void CEnrichedTransformer::FlushStack()
{
	CParserEnrichedStackElement *currElement = mStack->pop();

	while(currElement)
	{
		RemoveElement(currElement);
		delete currElement;
		currElement = mStack->pop();
	}
}


void CEnrichedTransformer::ExamineStack(const CATSUIStyle& styles)
{
	CParserEnrichedStack Pile;
	bool found = false;
	bool bolding = false;
	bool italicing = false;
	bool underlining = false;
	bool coloring = false;
	bool fonting = false;
	bool aligning = false;
	bool excerpting = false;

	int unit = 2;
	int size = ((int)((::Fix2Long(styles.GetFontSize()) + 1) / unit)) * unit;
	cdstring tempString;

	if (!mStack)
		mStack = new CParserEnrichedStack();

	// Reverse current stack
	CParserEnrichedStackElement *currElement = mStack->pop();
	while(currElement)
	{
		Pile.push(currElement);
		currElement = mStack->pop();
	}

	// Now scan for first difference, pushing back those that are the same
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
		case E_EXCERPT:
			if (false)
			{
				excerpting = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_COLOR:
		{
			char myColor[15];
			RGBColor color = styles.GetColor();
			::snprintf(myColor, 15, "%.4x,%.4x,%.4x", color.red, color.green, color.blue);
			if (!::unistrcmp(currElement->mParam, myColor))
			{
				coloring = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		}
		case E_BIGGER:
			if (size < mCurrentSize)
				matched = false;
			break;
		case E_SMALLER:
			if (size > mCurrentSize)
				matched = false;
			break;
		case E_FONT:
			tempString = (long) styles.GetFontNum();
			if (!::unistrcmp(currElement->mParam, tempString))
			{
				fonting = true;
				mStack->push(currElement);
			}
			else
				matched = false;
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
	CParserEnrichedStack Pile2;
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
		case E_EXCERPT:
			if (false)
			{
				Pile.push(currElement);
				underlining = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_COLOR:
		{
			char myColor[15];
			RGBColor color = styles.GetColor();
			::snprintf(myColor, 15, "%.4x,%.4x,%.4x", color.red, color.green, color.blue);
			if (!::unistrcmp(currElement->mParam, myColor))
			{
				Pile.push(currElement);
				coloring = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		}
		case E_BIGGER:
			if (size == mCurrentSize)
			{
				Pile.push(currElement);
				matched = true;
			}
			mCurrentSize -= 2;
			RemoveElement(currElement);
			break;
		case E_SMALLER:
			if (size == mCurrentSize)
			{
				Pile.push(currElement);
				matched = true;
			}
			mCurrentSize += 2;
			RemoveElement(currElement);
			break;
		case E_FONT:
			tempString = (long) styles.GetFontNum();
			if (!::unistrcmp(currElement->mParam, tempString))
			{
				Pile.push(currElement);
				fonting = true;
				matched = true;
			}
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
		currElement = new CParserEnrichedStackElement(E_FONT, -1);
		tempString = (long) styles.GetFontNum();
		currElement->mParam = cdustring(tempString);
		AddElement(currElement);
		mStack->push(currElement);
	}

	RGBColor color = styles.GetColor();
	if (!coloring && !((color.red == 0) && (color.green == 0) && (color.blue == 0)))
	{
		char myColor[15];
		::snprintf(myColor, 15, "%.4x,%.4x,%.4x", color.red, color.green, color.blue);
		currElement = new CParserEnrichedStackElement(E_COLOR, -1);
		currElement->mParam = cdustring(myColor);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if (size > mCurrentSize)
	{
		for(;size > mCurrentSize; mCurrentSize += 2)
		{
			currElement = new CParserEnrichedStackElement(E_BIGGER, -1);
			AddElement(currElement);
			mStack->push(currElement);
		}
	}

	if (size < mCurrentSize)
	{
		for(;size > mCurrentSize; mCurrentSize -= 2)
		{
			currElement = new CParserEnrichedStackElement(E_SMALLER, -1);
			AddElement(currElement);
			mStack->push(currElement);
		}
	}

	if ((styles.GetStyle() & bold) && !bolding)
	{
		currElement = new CParserEnrichedStackElement(E_BOLD, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((styles.GetStyle() & italic) && !italicing)
	{
		currElement = new CParserEnrichedStackElement(E_ITALIC, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((styles.GetStyle() & underline) && !underlining)
	{
		currElement = new CParserEnrichedStackElement(E_UNDERLINE, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if (false && !excerpting)
	{
		currElement = new CParserEnrichedStackElement(E_EXCERPT, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}
}

void CEnrichedTransformer::AddElement(CParserEnrichedStackElement* element)
{
	switch(element->mTagid)
	{
	case E_BOLD:
		mOut << "<bold>";
		break;
	case E_ITALIC:
		mOut << "<italic>";
		break;
	case E_UNDERLINE:
		mOut << "<underline>";
		break;
	case E_EXCERPT:
		mOut << "<excerpt>";
		break;
	case E_COLOR:
		mOut << "<color><param>" << element->mParam.ToUTF8() << "</param>";
		break;
	case E_BIGGER:
		mOut << "<bigger>";
		break;
	case E_SMALLER:
		mOut << "<smaller>";
		break;
	case E_FONT:
		{
			Str255 fname;
			::GetFontName(::uniatol(element->mParam), fname);
			mOut << "<fontfamily><param>" << cdstring(fname) << "</param>";
		}
		break;
	case E_FLEFT:
		mOut << "<flushleft>";
		break;
	case E_CENTER:
		mOut << "<center>";
		break;
	case E_FRIGHT:
		mOut << "<flushright>";
		break;
	case E_FBOTH:
		mOut << "<flushboth>";
		break;
	default:;
	}
}

void CEnrichedTransformer::RemoveElement(CParserEnrichedStackElement* element)
{
	switch(element->mTagid)
	{
	case E_BOLD:
		mOut << "</bold>";
		break;
	case E_ITALIC:
		mOut << "</italic>";
		break;
	case E_UNDERLINE:
		mOut << "</underline>";
		break;
	case E_EXCERPT:
		mOut << "</excerpt>";
		break;
	case E_COLOR:
		mOut << "</color>";
		break;
	case E_BIGGER:
		mOut << "</bigger>";
		break;
	case E_SMALLER:
		mOut << "</smaller>";
		break;
	case E_FONT:
		mOut << "</fontfamily>";
		break;
	case E_FLEFT:
		mOut << "</flushleft>";
		break;
	case E_CENTER:
		mOut << "</center>";
		break;
	case E_FRIGHT:
		mOut << "</flushright>";
		break;
	case E_FBOTH:
		mOut << "</flushboth>";
		break;
	default:;
	}
}
