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

#include "CCharSpecials.h"
#include "CHTMLCharacterEntity.h"
#include "CLetterTextEditView.h"
#include "CParserEnrichedStack.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CUStringUtils.h"
#include "CUTF8.h"

#pragma mark ________________________________CEnrichedUtils

// UTF8 in, UTF8 out
char* CEnrichedUtils::ConvertToHTML(const char* enrichedText)
{
	int nofill=0;
	ostrstream output;
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

	output << ends;
	return output.str();
}

// UTF8 in, UTF8 out
void CEnrichedUtils::WriteHTML(char ch, ostream &output)
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
	ostrstream output;
	Write(output, text, ::unistrlen(text));
	
	output << ends;
	return output.str();
}

// UTF16 in, UTF8 out
void CEnrichedUtils::Write(ostrstream& sout, const unichar_t* string, size_t length)
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
void CEnrichedUtils::WriteNoEndlDouble(ostrstream& sout, const unichar_t* string, size_t length)
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

CEnrichedTransformer::CEnrichedTransformer(CLetterTextEditView* letter)
{
	mTextPane = letter;
	mStack = new CParserEnrichedStack;
	LOGFONT font = CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().logfont;
	HDC hDC = ::GetDC(nil);
	int logpix = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if (logpix == 0)
		logpix = 1;
	mCurrentSize = ((2*-72*font.lfHeight/logpix) + 1)/2;	// Round up to nearest int
	mDefaultSize = -72*font.lfHeight/logpix;	// Round up to nearest int
	mDefaultFont = font.lfFaceName;
}

CEnrichedTransformer::~CEnrichedTransformer()
{
	delete mStack; 
	mStack = NULL;
}

char* CEnrichedTransformer::Transform()
{
	// Get the text - NB get the raw CR-only text from RichEdit2.0 control
	// as format ranges are for the raw text
	cdustring utxt;
	mTextPane->GetRawText(utxt);
	int length = utxt.length();

	int continuos_length = 0;			// Length of continuosly styled text
	unichar_t* p = utxt.c_str_mod();			// Start of next continuos style run
	unichar_t* q = utxt.c_str_mod();			// Next character to examine
	
	CHARFORMAT2 old_format;				// Current format to check for continuos run
	PARAFORMAT old_paraFormat;			// Current format to check for continuos run
	old_format.dwEffects = 0;
	old_format.yHeight = mDefaultSize;	// Start with default
	old_format.crTextColor = 0;
	::lstrcpyn(old_format.szFaceName, mDefaultFont.win_str(), LF_FACESIZE);	// Start with default
	old_paraFormat.wAlignment = 0;

	bool runCRLF = false;				// Keeps track of CRLF runs

	for(long i = 0; i < length; i++)
	{
		CHARFORMAT2 format;
		PARAFORMAT paraFormat;

		mTextPane->SetSelectionRange(i,i + 1);
		mTextPane->GetRichEditCtrl().GetSelectionCharFormat(format);
		mTextPane->GetRichEditCtrl().GetParaFormat(paraFormat);
		
		bool is_continuos = ((format.dwEffects == old_format.dwEffects) &&
							 (format.yHeight == old_format.yHeight) &&
							 (format.crTextColor == old_format.crTextColor) &&
							 (::lstrcmp(format.szFaceName, old_format.szFaceName) == 0) &&
							 (paraFormat.wAlignment == old_paraFormat.wAlignment));

		if (!is_continuos)
		{
			// Flush existing text to output stream as style changing
			if (continuos_length)
				CEnrichedUtils::WriteNoEndlDouble(mOut, p, continuos_length);

			// Always examine stack since run changed change
			ExamineStack(format, paraFormat);
			
			// Reset text range and CR runs
			p = q;
			continuos_length = 0;
			runCRLF = false;
			
			// Reset formats
			old_format.dwEffects = format.dwEffects;
			old_format.yHeight = format.yHeight;
			old_format.crTextColor = format.crTextColor;
			::lstrcpy(old_format.szFaceName, format.szFaceName);
			old_paraFormat.wAlignment = paraFormat.wAlignment;
		}

		// Must double up CRs into CRLFs
		if (*q == '\r')
		{
			// Force output to buffer with extra CRLF
			if (continuos_length)
				CEnrichedUtils::WriteNoEndlDouble(mOut, p, continuos_length);
			
			// If first CR in a run double it up
			if (!runCRLF)
				mOut << os_endl;
			mOut << os_endl;
			p = q + 1;
			continuos_length = -1;
			runCRLF = true;
		}
		else if ((*q != '\r') && runCRLF)
			runCRLF = false;

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

	mOut << os_endl << ends;
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
	

void CEnrichedTransformer::ExamineStack(CHARFORMAT2 format, PARAFORMAT paraFormat)
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
	cdstring tempString;
	
	int unit = 2;
	int size = (((int)((format.yHeight  / 20 + 1) / unit)) * unit);
	
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
			if (format.dwEffects & CFM_BOLD)
			{
				bolding = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_ITALIC:
			if (format.dwEffects & CFM_ITALIC)
			{
				italicing = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_UNDERLINE:
			if (format.bUnderlineType == CFU_UNDERLINE)
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
			char myColor[15];
			::snprintf(myColor, 15, "%.4x,%.4x,%.4x", GetRValue(format.crTextColor) * 256, 
												  GetGValue(format.crTextColor) * 256, 
												  GetBValue(format.crTextColor) * 256);
			if (::unistrcmp(currElement->mParam, myColor) == 0)
			{
				coloring = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break; 
		case E_BIGGER:
			if (size < mCurrentSize)
				matched = false;
			break;
		case E_SMALLER:
			if (size > mCurrentSize)
				matched = false;
			break;
		case E_FONT:
			tempString = format.szFaceName;
			if (::unistrcmp(currElement->mParam, tempString) == 0)
			{
				fonting = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_FLEFT:
			if ((paraFormat.dwMask & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_LEFT))
			{
				aligning = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_CENTER:
			if ((paraFormat.dwMask & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_CENTER))
			{
				aligning = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
		case E_FRIGHT:
			if ((paraFormat.dwMask & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_RIGHT))
			{
				aligning = true;
				mStack->push(currElement);
			}
			else
				matched = false;
			break;
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
			if (format.dwEffects & CFM_BOLD)
			{
				Pile.push(currElement);
				bolding = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_ITALIC:
			if (format.dwEffects & CFM_ITALIC)
			{
				Pile.push(currElement);
				italicing = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_UNDERLINE:
			if (format.bUnderlineType == CFU_UNDERLINE)
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
			char myColor[15];
			::snprintf(myColor, 15, "%.4x,%.4x,%.4x", GetRValue(format.crTextColor) * 256, 
											  GetGValue(format.crTextColor) * 256, 
											  GetBValue(format.crTextColor) * 256);
			if (::unistrcmp(currElement->mParam, myColor) == 0)
			{
				Pile.push(currElement);
				coloring = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
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
			tempString = format.szFaceName;
			if (!::unistrcmp(currElement->mParam, tempString))
			{
				Pile.push(currElement);
				fonting = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_FLEFT:
			if ((paraFormat.dwMask & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_LEFT))
			{
				Pile.push(currElement);
				aligning = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_CENTER:
			if ((paraFormat.dwMask & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_CENTER))
			{
				Pile.push(currElement);
				aligning = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
		case E_FRIGHT:
			if ((paraFormat.dwMask & PFM_ALIGNMENT) && (paraFormat.wAlignment == PFA_RIGHT))
			{
				Pile.push(currElement);
				aligning = true;
				matched = true;
			}
			RemoveElement(currElement);
			break;
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
		// Only add if not default
		if (mDefaultFont != cdstring(format.szFaceName))
		{
			currElement = new CParserEnrichedStackElement(E_FONT, -1);
			currElement->mParam = cdustring(format.szFaceName);
			AddElement(currElement);
			mStack->push(currElement);
		}
	}

	if (!aligning)
	{
		if (paraFormat.wAlignment == PFA_LEFT)
			currElement = new CParserEnrichedStackElement(E_FLEFT, -1);
		else if (paraFormat.wAlignment == PFA_CENTER)
			currElement = new CParserEnrichedStackElement(E_CENTER, -1);
		else if (paraFormat.wAlignment == PFA_RIGHT)
			currElement = new CParserEnrichedStackElement(E_FRIGHT, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if (!coloring && !(GetRValue(format.crTextColor) == 0 && GetGValue(format.crTextColor) == 0 && GetBValue(format.crTextColor) == 0))
	{
		char myColor[15];
		::snprintf(myColor, 15, "%.4x,%.4x,%.4x", GetRValue(format.crTextColor) * 256, 
										  GetGValue(format.crTextColor) * 256, 
										  GetBValue(format.crTextColor) * 256);
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

	if ((format.dwMask & CFM_BOLD) && (format.dwEffects & CFE_BOLD) && !bolding)
	{
		currElement = new CParserEnrichedStackElement(E_BOLD, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((format.dwMask & CFM_ITALIC) && (format.dwEffects & CFE_ITALIC) && !italicing)
	{
		currElement = new CParserEnrichedStackElement(E_ITALIC, -1);
		AddElement(currElement);
		mStack->push(currElement);
	}

	if ((format.dwMask & CFM_UNDERLINETYPE) && (format.bUnderlineType == CFU_UNDERLINE) && !underlining)
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
		mOut << "<fontfamily><param>" << element->mParam.ToUTF8() << "</param>";
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
	}
}
