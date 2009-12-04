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


#include "CParserPlain.h"

#include "CClickList.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CURL.h"

#include <strstream>

CParserPlain::CParserPlain(const unichar_t* st, CFormatList* format, CClickList* click, bool delete_p)
{
	mQuoteDepth = -1;

	// Cache copy/original pointer - if copy set flag to have it deleted
	mTxt = (delete_p ? ::unistrdup(st) : st);
	mDeleteData = delete_p;
	
#if __dest_os == __win32_os
	// If copy - must remove \n as RichEdit2.0 only uses \r
	if (delete_p)
		::FilterOutLFs(const_cast<unichar_t*>(mTxt));
#endif	
	
	mStrLen = mTxt ? ::unistrlen(mTxt) : 0;

	mFormatList = format;
	mClickList = click;
}

CParserPlain::~CParserPlain()
{
	if (mDeleteData)
		delete mTxt;
}

const unichar_t* CParserPlain::Parse(int offset)
{
	std::ostrstream sout;

	const unichar_t* p = mTxt;
	if (p != NULL)
	{
		// Filter out lines with quote level higher than the quote depth
		if ((mQuoteDepth >= 0) && CPreferences::sPrefs->mRecognizeQuotes.GetValue().size())
		{
			// Get quotes and pre-calculate their sizes
			const cdstrvect& quotes = CPreferences::sPrefs->mRecognizeQuotes.GetValue();
			cdustrvect uquotes;
			ulvector usizes;
			for(cdstrvect::const_iterator iter = quotes.begin(); iter != quotes.end(); iter++)
			{
				uquotes.push_back(cdustring(*iter));
				usizes.push_back(uquotes.back().length());
			}

			const unichar_t* line = p;
			for(; *p; p++)
			{
				// Find line end
				switch(*p)
				{
				case lendl1:
					{	
					// Now look at start of line and see if a quote
					long depth = GetQuoteDepth(line, uquotes, usizes);
					if (depth <= mQuoteDepth)
						// Write the line (+ CR)
						sout.write((const char*)line, (p - line + 1) * sizeof(unichar_t));
					line = p + 1;
					break;
					}
				default:;
				}
			}

			// Treat remainder as line end
			long depth = GetQuoteDepth(line, uquotes, usizes);
			if (depth <= mQuoteDepth)
				// Write the line
				sout.write((const char*)line, (p - line) * sizeof(unichar_t));
		}
		else
			// Copy input to output
			sout.write((const char*)p, ::unistrlen(p) * sizeof(unichar_t));
	}

	sout << std::ends << std::ends;
	return (unichar_t*)sout.str();
}


long CParserPlain::GetQuoteDepth(const unichar_t* line, const cdustrvect& quotes, const ulvector& sizes)
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
			if (!::unistrncmp(*iter1, p, *iter2))
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

void CParserPlain::LookForURLs(int offset)
{
	const unichar_t* p = mTxt;
	if (p == NULL)
		return;

	int i = 0;
	for(; *p; p++, i++)
	{
		if (i == 0)
			Probe(i, offset, 0);
		else
		{
			const unichar_t c = *(p-1);
			switch(c)
			{
			// Match space and controls
			case ' ':
			case '\n':
			case '\r':
			case '\t':
				// No delimiter match
				Probe(i, offset, 0);
				break;

			// Match brackets
			//case '[':			// RFC2732 uses '[...]' for Ipv6 addressing
			case '{':
			case '(':
			case '<':
			 	Probe(i, offset, ::getbracketmatch(c));
			 	break;

			// Match quotes
			case '\"':
			case '\'':
				Probe(i, offset, c);
				break;
			}
		}
	}
}

void CParserPlain::AddURL(const char* value)
{
	mFindURLS.push_back(cdustring(value));
}

void CParserPlain::DrawURL(SInt32 start, SInt32 stop)
{
	// Maybe NULL when checking for outgoing A HREFs
	if ((start < stop) && mFormatList)
	{
		mFormatList->addElement(new CColorFormatElement(start, stop, CPreferences::sPrefs->mURLStyle.GetValue().color));

		mFormatList->addElement(new CFaceFormatElement(start, stop, E_PLAIN));

		if (CPreferences::sPrefs->mURLStyle.GetValue().style & bold)
			mFormatList->addElement(new CFaceFormatElement(start, stop, E_BOLD));

		if (CPreferences::sPrefs->mURLStyle.GetValue().style & underline)
			mFormatList->addElement(new CFaceFormatElement(start, stop, E_UNDERLINE));

		if (CPreferences::sPrefs->mURLStyle.GetValue().style & italic)
			mFormatList->addElement(new CFaceFormatElement(start, stop, E_ITALIC));
	}
}

void CParserPlain::Probe(int char_offset, int offset, char delimiter)
{
	int scheme_offset = 0;

	CURLClickElement* clicker = NULL;

	static cdustring urlmainscheme(cURLMainScheme);

	for(cdustrvect::const_iterator iter = mFindURLS.begin(); iter != mFindURLS.end(); iter++)
	{
		// Do comparison with element
		int i = 0;
		if (mStrLen - char_offset >= (*iter).length())
		{
			if (!::unistrncmpnocase(&mTxt[char_offset], *iter, (*iter).length()))
				i = (*iter).length();
			else if (!::unistrncmpnocase(&mTxt[char_offset], urlmainscheme, urlmainscheme.length()) &&
					 !::unistrncmpnocase(&mTxt[char_offset] + urlmainscheme.length(), *iter, (*iter).length()))
			{
				i = (*iter).length() + urlmainscheme.length();
				scheme_offset = urlmainscheme.length();
			}
		}

		if (i)
		{
			int start = char_offset + scheme_offset;
			int middle = char_offset + i;
			while((i + char_offset < mStrLen) &&
					((delimiter && (mTxt[i + char_offset] != delimiter)) || !delimiter) &&		// Check delimiter first
					((delimiter && isuspace(mTxt[i + char_offset])) ||							// Allow whtespace if looking for delimiter
				 	 (mTxt[i + char_offset] < 0x0100) && (cURLXCharacter[(unsigned char) mTxt[i + char_offset]] == 1)))
			{
				i++;
			}
			int last = char_offset + i;
			
			// Bump down if ends in punctuation and not bracketed
			if((mTxt[last] != delimiter) && isupunct(mTxt[last-1]) &&
						(mTxt[last-1] != ':') && (mTxt[last-1] != '/') && (mTxt[last-1] != '~'))
				last--;

			// Only highlight the URL if there is a location
			if (last > middle)
			{
				DrawURL(start+offset, last+offset);

				// Get parts
				cdustring uscheme(mTxt+start, middle-start);
				cdstring scheme = uscheme.ToUTF8();
				cdustring ulocation(mTxt+middle, last-middle);
				cdstring location = ulocation.ToUTF8();

				// Remove possible whitespace
				char* location1 = (char*) location.c_str();
				char* location2 = (char*) location.c_str();
				while(*location1)
				{
					if (!isspace(*location1))
						*location2++ = *location1++;
					else
						location1++;
				}
				*location2 = 0;

				clicker = new CURLClickElement(start + offset, last + offset, scheme.grab_c_str(), location.grab_c_str());
				mClickList->addElement(clicker);
			}
		}
	}
}
