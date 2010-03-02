/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


#include "CParserHTML.h"

#include "CCharSpecials.h"
#if __dest_os == __win32_os
#include "CFontMenu.h"
#endif
#include "CFormatList.h"
#include "CClickList.h"
#include "CHTMLCharacterEntity.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CUTF8.h"

#include <math.h>
#include <memory>
#include <strstream>

#if __dest_os == __linux_os
#include <JFontManager.h>
#include <JXDisplay.h>
#include "CMulberryApp.h"
#endif 

const int cRulerWidth = 50;

CParserHTML::CParserHTML(const unichar_t* st, CFormatList* format, CClickList* click, CAnchorElementMap* anchor, bool use_styles)
#if __dest_os == __linux_os
	: mFontNames(JPtrArrayT::kDeleteAll)
#endif
{
	InitParserHTML();

	mTxt = ::unistrdup(st);
#if __dest_os == __win32_os
	// Must remove \n as RichEdit2.0 only uses \r
	::FilterOutLFs(mTxt);
#endif	
	mStrLen = mTxt ? ::unistrlen(mTxt) : 0;
	mFormatList = format;
	mClickList = click;
	mAnchorMap = anchor;
	mUseStyles = use_styles;
}

CParserHTML::CParserHTML(CFormatList* format, CClickList* click, CAnchorElementMap* anchor)
#if __dest_os == __linux_os
	: mFontNames(JPtrArrayT::kDeleteAll)
#endif
{
	InitParserHTML();

	mFormatList = format;
	mClickList = click;
	mAnchorMap = NULL;
}


CParserHTML::CParserHTML()
#if __dest_os == __linux_os
	: mFontNames(JPtrArrayT::kDeleteAll)
#endif
{
	InitParserHTML();
}


CParserHTML::~CParserHTML()
{
	delete mTxt;
}

void CParserHTML::InitParserHTML()
{
	mTxt = NULL;
	mFormatList = NULL;
	mClickList = NULL;
	mAnchorMap = NULL;
	mUseStyles = true;
	mDoAlignment = true;
	mStrLen = 0;
	mBaseFontSize = 3;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	mStyledFontSize = CPreferences::sPrefs->mHTMLTextTraits.GetValue().traits.size;
	mFixedFontSize = CPreferences::sPrefs->mFixedTextTraits.GetValue().traits.size;
	mFixedFontName = CPreferences::sPrefs->mFixedTextTraits.GetValue().traits.fontName;
#elif __dest_os == __win32_os
	HDC hDC = ::GetDC(NULL);
	mLogpix = ::GetDeviceCaps(hDC, LOGPIXELSY);
	if (mLogpix == 0)
		mLogpix = 1;
	::ReleaseDC(NULL, hDC);

	LOGFONT fontInfo = CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().logfont;
	int ht = fontInfo.lfHeight;
	int pt_size = ((2*-72*ht/mLogpix) + 1)/2;	// Round up to nearest int
	mStyledFontSize = pt_size;

	fontInfo = CPreferences::sPrefs->mFixedTextFontInfo.GetValue().logfont;
	ht = fontInfo.lfHeight;
	pt_size = ((2*-72*ht/mLogpix) + 1)/2;	// Round up to nearest int
	mFixedFontSize = pt_size;
	mFixedFontName = fontInfo.lfFaceName;
#elif __dest_os == __linux_os
	mStyledFontSize = CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().size;
	mFixedFontSize = CPreferences::sPrefs->mFixedTextFontInfo.GetValue().size;
	mFixedFontName = CPreferences::sPrefs->mFixedTextFontInfo.GetValue().fontname;
	CMulberryApp::sApp->GetCurrentDisplay()->GetFontManager()->GetFontNames(&mFontNames);
#else
#error __dest_os
#endif

	// Set default colors
#if __dest_os == __win32_os
	mBackground = RGB(0xFF,0xFF,0xFF);
#else
	mBackground.red = mBackground.green = mBackground.blue =  0xFFFF;
#endif
#if __dest_os == __win32_os
	mTextColor = RGB(0x00,0x00,0x00);
#else
	mTextColor.red = mTextColor.green = mTextColor.blue =  0x0000;
#endif
}

void CParserHTML::SetFontScale(long scale)
{
	mStyledFontSize = std::max(1.0, pow(1.2, scale) * mStyledFontSize);
	mFixedFontSize = std::max(1.0, pow(1.2, scale) * mFixedFontSize);
}

ETag CParserHTML::GetTag(unichar_t* format, long &offset)
{
	cdustring lformat = format;
	::unistrlower(lformat);

	if(!::unistrcmp(lformat, "b"))
	{
		offset = -1;
		return E_BOLD;
	}
	else if(!::unistrcmp(lformat, "i"))
	{
		offset = -1;
		return E_ITALIC;
	}
	else if(!::unistrcmp(lformat, "u"))
	{
		offset = -1;
		return E_UNDERLINE;
	}
	else if(!::unistrcmp(lformat, "center"))
	{
		offset = -1;
		return E_CENTER;
	}
	else if(!::unistrncmp(lformat, "font", 4))
	{
		offset = 4;
		while(isuspace(*(format + offset)))
			offset++;
		return E_FONT;
	}
	else if(!::unistrcmp(lformat, "a") || (*lformat.c_str() == 'a') && isuspace(*(lformat.c_str() + 1)))
	{
		offset = 1;
		while(isuspace(*(format + offset)))
			offset++;
		return E_ANCHOR;
	}

	else if(!::unistrncmp(lformat, "h1", 2))
	{
		offset = -1;
		return E_H1;
	}

	else if(!::unistrncmp(lformat, "h2", 2))
	{
		offset = -1;
		return E_H2;
	}

	else if(!::unistrncmp(lformat, "h3", 2))
	{
		offset = -1;
		return E_H3;
	}

	else if(!::unistrncmp(lformat, "h4", 2))
	{
		offset = -1;
		return E_H4;
	}

	else if(!::unistrncmp(lformat, "h5", 2))
	{
		offset = -1;
		return E_H5;
	}

	else if(!::unistrncmp(lformat, "h6", 2))
	{
		offset = -1;
		return E_H6;
	}

	else if(!::unistrcmp(lformat, "em"))
	{
		offset = -1;
		return E_EM;
	}

	else if(!::unistrcmp(lformat, "strong"))
	{
		offset = -1;
		return E_STRONG;
	}

	else if(!::unistrcmp(lformat, "cite"))
	{
		offset = -1;
		return E_CITE;
	}

	else if(!::unistrcmp(lformat, "code"))
	{
		offset = -1;
		return E_CODE;
	}

	else if(!::unistrcmp(lformat, "samp"))
	{
		offset = -1;
		return E_SAMP;
	}

	else if(!::unistrcmp(lformat, "kbd"))
	{
		offset = -1;
		return E_KBD;
	}

	else if(!::unistrcmp(lformat, "var"))
	{
		offset = -1;
		return E_VAR;
	}

	else if(!::unistrcmp(lformat, "tt"))
	{
		offset = -1;
		return E_TT;
	}

	else if(!::unistrcmp(lformat, "pre"))
	{
		offset = -1;
		return E_TT;
	}

	else if(!::unistrcmp(lformat, "big"))
	{
		offset = -1;
		return E_BIG;
	}

	else if(!::unistrcmp(lformat, "small"))
	{
		offset = -1;
		return E_SMALL;
	}
	else if(!::unistrncmp(lformat, "blockquote", 10))
	{
		offset = -1;
		return E_BLOCKQUOTE;
	}
	else
	{
		offset = -1;
		return E_NOTHING;
	}
}

long CParserHTML::FindParam(const unichar_t* param, cdustring& response, bool quoted)
{
	unsigned short state = 0;
	std::ostrstream output;
	bool done = false;
	long eaten = 0;
	bool quoting = false;

	int i = ::unistrlen(param);

	for( ; eaten < i && !done; param++)
	{
		eaten++;
		if (state == 0)
		{
			// finding the =
			if (isuspace(*param))
			{
			}
			else if (*param == '=')
				state = 1;
			else
				done = true;
		}
		else if (state == 1)
		{
			// finding the first char for param
			if (isuspace(*param))
			{
			}
			else if (*param == '\"')
				quoting = true;
			else if ((*param < 0x80) && cUSASCIIChar[*param])
			{
				output.write((const char*)param, sizeof(unichar_t));
				state = 2;
			}
			else
				done = true;
		}
		else if (state == 2)
		{
			if (*param == '\"')
			{
				if (quoted || quoting)
				{
					eaten++;
					done = true;
				}
			}
			else if (isuspace(*param))
			{
				if (!quoted && !quoting)
					done = true;
				else
					output.write((const char*)param, sizeof(unichar_t));
			}
			else if ((*param < 0x80) && cUSASCIIChar[*param])
				output.write((const char*)param, sizeof(unichar_t));
			else
				done = true;
		}
	}

	if (state == 2)
	{
		output << std::ends << std::ends;
		response.steal(reinterpret_cast<unichar_t*>(output.str()));
	}

	return eaten;
}

bool CParserHTML::GetLatinChar(unichar_t* format, std::ostream* out, unsigned long* added)
{
	// Look for XML style hex/decimal notation
	wchar_t charNum = ' ';
	if ((*format == 'x') || (*format == 'X'))
		charNum = ::unihextoul(format + 1);
	else
		charNum = ::uniatoul(format);

	return GetLatinChar(charNum, out, added);
}

bool CParserHTML::GetLatinChar(wchar_t charNum, std::ostream* out, unsigned long* added)
{
	// Never allow NULLs
	if (!charNum)
		charNum = ' ';

	// Just add utf16 character
	unichar_t c = charNum;
	if (out)
		out->write((const char*)&c, sizeof(unichar_t));
	if (added)
		*added += 1;
			
	return true;
}

bool CParserHTML::HandleAmpChar(unichar_t* format, std::ostream* out, unsigned long* added)
{
	// Valid input?
	if (!format)
	{
		if (out)
		{
			unichar_t uc = '?';
			out->write((const char*)&uc, sizeof(unichar_t));
			if (added)
				*added += 1;
		}
		return true;
	}

	// Look for decimal character code
	else if (*format == '#')
		return GetLatinChar(format + 1, out, added);

	// Get utf8 version of string
	cdustring temp;
	temp.steal(format);
	cdstring format_utf8 = temp.ToUTF8();
	temp.release();

	// Scan map for matching character
	for(const SMapHTML* map = cHTMLCharacterEntity; *map->html; map++)
	{
		// Must be case sensitive compare
		if (!::strcmp(format_utf8, map->html))
			return GetLatinChar(map->mapped, out, added);
	}

	return false;
}

long CParserHTML::FindParamUTF8(const char* param, cdstring &response, bool quoted)
{
	unsigned short state = 0;
	std::ostrstream output;
	bool done = false;
	long eaten = 0;
	bool quoting = false;

	int i = ::strlen(param);

	for( ; eaten < i && !done; param++)
	{
		eaten++;
		if (state == 0)
		{
			// finding the =
			if(isspace(*param))
			{
			}
			else if(*param == '=')
				state = 1;
			else
				done = true;
		}
		else if (state == 1)
		{
			// finding the first char for param
			if(isspace(*param))
			{
			}
			else if(*param == '\"')
				quoting = true;
			else if(cUSASCIIChar[(unsigned char) *param])
			{
				output << param[0];
				state = 2;
			}
			else
				done = true;
		}
		else if (state == 2)
		{
			if (*param == '\"')
			{
				if (quoted || quoting)
				{
					eaten++;
					done = true;
				}
			}
			else if (isspace(*param))
			{
				if (!quoted && !quoting)
					done = true;
				else
					output << param[0];
			}
			else if(cUSASCIIChar[(unsigned char) *param])
				output << param[0];
			else
				done = true;
		}
	}

	if (state == 2)
	{
		output << std::ends;
		response.steal(output.str());
	}

	return eaten;
}

bool CParserHTML::GetLatinCharUTF8(char* format, std::ostream* sout, unsigned long* added)
{
	// Look for XML style hex/decimal notation
	wchar_t charNum = ' ';
	if ((*format == 'x') || (*format == 'X'))
		charNum = ::strtol(format + 1, NULL, 16);
	else
		charNum = ::atol(format);

	return GetLatinCharUTF8(charNum, sout, added);
}

bool CParserHTML::GetLatinCharUTF8(wchar_t charNum, std::ostream* sout, unsigned long* added)
{
	// Never allow NULLs
	if (!charNum)
		charNum = ' ';

	if (charNum < 128)
	{
		if (sout)
		{
			char c = charNum;
			*sout << c;
			if (added)
				*added += 1;
		}
		return true;
	}
	else
	{
		// Convert some entities to utf8 equivalents
		i18n::CUTF8 utf8;
		char buffer[16];
		int count = utf8.w_2_c(charNum, buffer);
		if (sout)
		{
			sout->write(buffer, count);
			if (added)
				*added += count;
		}
		return true;
	}
}

bool CParserHTML::HandleAmpCharUTF8(char* format, std::ostream* sout, unsigned long* added)
{
	// Valid input?
	if (!format)
	{
		if (sout)
		{
			*sout << '&';
			if (added)
				*added += 1;
		}
		return true;
	}

	// Look for decimal character code
	else if (*format == '#')
		return GetLatinCharUTF8(format + 1, sout, added);

	// Scan map for matching character
	for(const SMapHTML* map = cHTMLCharacterEntity; *map->html; map++)
	{
		// Must be case sensitive compare
		if (!::strcmp(format, map->html))
			return GetLatinCharUTF8(map->mapped, sout, added);
	}

	return false;
}

void CParserHTML::HandleBody(unichar_t* param)
{
	bool done = false;

	for( ; ::unistrlen(param) > 0 && !done; param++)
	{
		if (!::unistrncmpnocase(param, "bgcolor", 7))
		{
			// Make sure its a valid parameter
			const unichar_t* p = param + 7;
			while(*p == ' ') p++;
			if (*p != '=')
			{
				param += 7;
				continue;
			}

			// Got valid bgcolor parameter
			cdustring colorStr;
			param += 7;
			param += FindParam(param, colorStr) - 1;

			// Only do styles if requested by user
			if (mUseStyles)
			{
				cdstring colorStr_utf8 = colorStr.ToUTF8();
				CColorFormatElement celement(0, 1, colorStr_utf8, true);
				mBackground = celement.getColor();
			}
		}

		if (!::unistrncmpnocase(param, "text", 4))
		{
			// Make sure its a valid parameter
			const unichar_t* p = param + 4;
			while(*p == ' ') p++;
			if (*p != '=')
			{
				param += 4;
				continue;
			}

			// Got valid text parameter
			cdustring colorStr;
			param += 4;
			param += FindParam(param, colorStr) - 1;

			// Only do styles if requested by user
			if (mUseStyles)
			{
				cdstring colorStr_utf8 = colorStr.ToUTF8();
				CColorFormatElement celement(0, 1, colorStr_utf8);
				mTextColor = celement.getColor();
			}
		}
	}
}


void CParserHTML::HandleHeading(long start, long stop, ETag tag, unichar_t* param)
{
	cdustring alignment;
	for( ; ::unistrlen(param) > 0; param++)
	{
		if (!::unistrncmpnocase(param, "align", 5))
		{
			param += 5;
			param += FindParam(param, alignment) - 1;
		}
	}

	int relsize;
	switch(tag)
	{
	case E_H1:
		relsize = 6;
		break;
	case E_H2:
		relsize = 5;
		break;
	case E_H3:
	default:;
		relsize = 4;
		break;
	case E_H4:
		relsize = 3;
		break;
	case E_H5:
		relsize = 2;
		break;
	case E_H6:
		relsize = 1;
		break;
	}
	HandleScaledSize(start, stop, relsize);

	// Only do styles if requested by user
	if (mUseStyles)
	{
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_BOLD));

		if (!alignment.empty() && mDoAlignment)
		{
			::unistrlower(alignment);
			if(!::unistrcmp(alignment, "left"))
				mFormatList->addElement(new CAlignmentFormatElement(start, stop, E_FLEFT));
			else if(!::unistrcmp(alignment, "center"))
				mFormatList->addElement(new CAlignmentFormatElement(start, stop, E_CENTER));
			else if(!::unistrcmp(alignment, "right"))
				mFormatList->addElement(new CAlignmentFormatElement(start, stop, E_FRIGHT));
		}
	}
}

void CParserHTML::HandleFormat(unichar_t* format, int index)
{
	long offset = -1;

	if (format[0] == '/')
	{
		ETag tag = GetTag(format + 1, offset);
		std::auto_ptr<CParserHTMLStackElement> element(mHStack.pop(tag));
		if (element.get() != NULL)
		{
			// Some tags only make sense when they actuall enclose some valid data
			if (element->mStart < index)
			{
				switch(tag)
				{
				case E_BOLD:
				case E_ITALIC:
				case E_UNDERLINE:
					if (mUseStyles)
						mFormatList->addElement(new CFaceFormatElement(element->mStart, index, element->mTagid));
					break;
				case E_CENTER:
					if (mUseStyles && mDoAlignment)
						mFormatList->addElement(new CAlignmentFormatElement(element->mStart, index, element->mTagid));
					break;
				case E_ANCHOR:
					HandleAnchor(element->mStart, index, element->mParam);
					break;
				case E_FONT:
					HandleFont(element->mStart, index, element->mParam);
					break;
				case E_H1:
				case E_H2:
				case E_H3:
				case E_H4:
				case E_H5:
				case E_H6:
					HandleHeading(element->mStart, index, tag, element->mParam);
					break;
				case E_EM:
					if (mUseStyles)
						mFormatList->addElement(new CFaceFormatElement(element->mStart, index, E_ITALIC));
					break;
				case E_STRONG:
					if (mUseStyles)
						mFormatList->addElement(new CFaceFormatElement(element->mStart, index, E_BOLD));
					break;
				case E_CITE:
					if (mUseStyles)
						mFormatList->addElement(new CFaceFormatElement(element->mStart, index, E_ITALIC));
					break;
				case E_CODE:
				case E_SAMP:
				case E_KBD:
				case E_VAR:
				case E_TT:
					{
						if (mUseStyles)
							mFormatList->addElement(new CFontFormatElement(element->mStart, index, mFixedFontName));
						HandleScaledSize(element->mStart, index, 3, true);
					}
					break;
				case E_BIG:
					HandleScaledSize(element->mStart, index, 5);
					break;
				case E_SMALL:
					HandleScaledSize(element->mStart, index, 2);
					break;
				case E_BLOCKQUOTE:
					if (mUseStyles)
						mFormatList->addElement(new CExcerptFormatElement(element->mStart, index));
					break;
				default:;
				}
			}
			
			// Other tags may work with no content
			else
			{
				switch(tag)
				{
				case E_ANCHOR:
					HandleAnchor(element->mStart, index, element->mParam);
					break;
				default:;
				}
			}
		}
	}
	else
	{
		ETag tag = GetTag(format, offset);

		CParserHTMLStackElement* element = new CParserHTMLStackElement(tag, index);
		if (offset>= 0)
			element->mParam = format + offset;

		mHStack.push(element);
	}
}

void CParserHTML::HandleImage(unichar_t* param, std::ostream* out, unsigned long* added)
{
	// Look for ALT param in images
	bool done = false;
	bool got_alt = false;
	cdustring src;
	for( ; ::unistrlen(param) > 0 && !done; param++)
	{
		if(!::unistrncmpnocase(param, "alt", 3))
		{
			// Get ALT value
			cdustring altStr;
			param += 3;
			param += FindParam(param, altStr) - 1;
			
			// Output the alt text if any available
			if (!altStr.empty() && out)
			{
				cdustring temp;
				temp.append_ascii(" [Image: \"");
				temp += altStr;
				temp.append_ascii("\"] ");
				
				out->write((const char*)temp.c_str(), temp.length() * sizeof(unichar_t));
				if (added)
					*added += temp.length();
				got_alt = true;
			}
		}

		else if(!::unistrncmpnocase(param, "src", 3))
		{
			// Get SRC value
			param += 3;
			param += FindParam(param, src) - 1;
		}
	}
	
	// Add some indicator of image
	if (!got_alt)
	{
		if (src.empty())
		{
			cdustring temp;
			temp.append_ascii(" [Image] ");
			
			out->write((const char*)temp.c_str(), temp.length() * sizeof(unichar_t));
			if (added)
				*added += temp.length();
		}
		else
		{
			// Find last component of image URL
			cdustring src_end;
			const unichar_t* p = ::unistrrchr(src.c_str(), '/');
			if (p != NULL)
			{
				src_end = p + 1;
			}
			
			// Strip off any extended data after '?'
			if (src_end.length())
				p = ::unistrchr(src_end.c_str(), '?');
			if (p != NULL)
				*const_cast<unichar_t*>(p) = 0;
				
			cdustring temp;
			temp.append_ascii(" [Image: \"");
			temp += src_end;
			temp.append_ascii("\"] ");
			
			out->write((const char*)temp.c_str(), temp.length() * sizeof(unichar_t));
			if (added)
				*added += temp.length();
		}
	}
}

void CParserHTML::HandleFont(long start, long stop, unichar_t* param)
{
	bool done = false;
	cdustring colorStr;
	cdustring sizeStr;
	cdustring faceStr;

	unichar_t* p = param;
	while(*p)
	{
		// Punt space
		while(isuspace(*p)) p++;
	
		if (!::unistrncmpnocase(p, "color", 5))
		{
			p += 5;
			p += FindParam(p, colorStr) - 1;
		}

		else if (!::unistrncmpnocase(p, "size", 4))
		{
			p += 4;
			p += FindParam(p, sizeStr) - 1;
		}

		else if (!::unistrncmpnocase(p, "face", 4))
		{
			p += 4;
			p += FindParam(p, faceStr, true) - 1;
		}
		else
			while(*p && !isuspace(*p)) p++;
	}

	if ((start < stop) && !colorStr.empty())
	{
		// Only do styles if requested by user
		if (mUseStyles)
		{
			cdstring colorStr_utf8 = colorStr.ToUTF8();
			mFormatList->addElement(new CColorFormatElement(start, stop, colorStr_utf8.c_str()));
		}
	}

	if ((start < stop) && !sizeStr.empty())
	{
		long amount = ::uniatol(sizeStr);
		if (sizeStr[(cdstring::size_type)0] == '+' || sizeStr[(cdstring::size_type)0] == '-')
			amount += mBaseFontSize;

		if (amount > 7)
			amount = 7;
		else if (amount <= 0)
			amount = 1;

		HandleScaledSize(start, stop, amount);
	}

	if ((start < stop) && !faceStr.empty())
	{
		done = false;
		unichar_t* tok_temp = NULL;
		unichar_t tok_tokens[] = { ',', 0 };
		unichar_t* tmpStr = ::unistrtok(faceStr.c_str_mod(), tok_tokens, &tok_temp);

#if __dest_os == __linux_os
		unichar_t* firstFont = tmpStr;
#endif

		for(; tmpStr && !done; tmpStr = ::unistrtok(NULL, tok_tokens, &tok_temp))
		{
			cdustring temp;
			temp.steal(tmpStr);
			cdstring tmpStr_utf8 = temp.ToUTF8();
			temp.release();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			LStr255 fontname;
			TextStyle ts;

			fontname = tmpStr_utf8;

			::GetFNum(fontname, &ts.tsFont);
			if (ts.tsFont != 0)
			{
				done = true;
				// Only do styles if requested by user
				if (mUseStyles)
				{
					mFormatList->addElement(new CFontFormatElement(start, stop, tmpStr_utf8));
				}
			}
#elif __dest_os == __win32_os
			if (CFontPopup::FontExists(tmpStr_utf8))
			{
				done = true;
				// Only do styles if requested by user
				if (mUseStyles)
				{
					mFormatList->addElement(new CFontFormatElement(start, stop, tmpStr_utf8));
				}
			}
#elif __dest_os == __linux_os
			JSize elementCount = mFontNames.GetElementCount();
			for (JIndex i = 1; i <= elementCount; i++)
			{
				//case insensative string compare
				if (!(JStringCompare(*mFontNames.NthElement(i), tmpStr_utf8, kFalse)))
				{
					done = true;
					// Only do styles if requested by user
					if (mUseStyles)
					{
						mFormatList->addElement(new CFontFormatElement(start, stop, tmpStr_utf8));
					}
					break;
				}
			}
#else
#error __dest_os
#endif
		}
#if __dest_os == __linux_os
		// Set to user default
		if (!done)
		{
			// Only do styles if requested by user
			if (mUseStyles)
			{
				cdstring styledFont(CPreferences::sPrefs->mHTMLTextFontInfo.GetValue().fontname);						
				mFormatList->addElement(new CFontFormatElement(start, stop, styledFont));
			}
		}
#endif											 
	}
}

void CParserHTML::HandleAnchor(long start, long stop, unichar_t* param)
{
	cdustring scheme;
	cdustring location;

	// Only do if anchor contains 'HREF'
	if (ParseURL(param, scheme, location))
	{
		// Only bother if there is something to highlight
		if (start == stop)
			return;

		// Allow URLs without schemes provided they are local
		if (mClickList && !location.empty() && (!scheme.empty() || location[(cdstring::size_type)0] == '#'))
		{
			// Create click element
			cdstring scheme_utf8 = scheme.ToUTF8();
			cdstring location_utf8 = location.ToUTF8();
			CURLClickElement* clicker = new CURLClickElement(start, stop, scheme_utf8.grab_c_str(), location_utf8.grab_c_str());
			mClickList->addElement(clicker);
		}

		// Do formatting for URL range
		DrawURL(start, stop);
	}
	else if (ParseAnchor(param, location))
	{
		if (mAnchorMap && !location.empty())
		{
			// Must prefix # to location for anchor map to work properly
			cdstring item_utf8("#");
			item_utf8 += location.ToUTF8();

			// Create click element
			CAnchorClickElement anchor(start, stop, item_utf8.grab_c_str());
			mAnchorMap->insert(CAnchorElementMap::value_type(anchor.GetDescriptor(), anchor));
		}
	}
}

void CParserHTML::HandleScaledSize(long start, long stop, long relsize, bool fixed)
{
	// Only if some text available
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

void CParserHTML::FlushStack(int index)
{
	std::auto_ptr<CParserHTMLStackElement> element(mHStack.pop());
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
				case E_ITALIC:
				case E_UNDERLINE:
					mFormatList->addElement(new CFaceFormatElement(element->mStart, index, element->mTagid));
					break;
				case E_CENTER:
					if (mDoAlignment)
						mFormatList->addElement(new CAlignmentFormatElement(element->mStart, index, element->mTagid));
					break;
				case E_ANCHOR:
					HandleAnchor(element->mStart, index, element->mParam);
					break;
				case E_FONT:
					HandleFont(element->mStart, index, element->mParam);
					break;
				case E_H1:
				case E_H2:
				case E_H3:
				case E_H4:
				case E_H5:
				case E_H6:
					HandleHeading(element->mStart, index, element->mTagid, element->mParam);
					break;
				case E_EM:
					mFormatList->addElement(new CFaceFormatElement(element->mStart, index, E_ITALIC));
					break;
				case E_STRONG:
					mFormatList->addElement(new CFaceFormatElement(element->mStart, index, E_BOLD));
					break;
				case E_CITE:
					mFormatList->addElement(new CFaceFormatElement(element->mStart, index, E_ITALIC));
					break;
				case E_CODE:
				case E_SAMP:
				case E_KBD:
				case E_VAR:
				case E_TT:
					{
						mFormatList->addElement(new CFontFormatElement(element->mStart, index, mFixedFontName));
						HandleScaledSize(element->mStart, index, 3, true);
					}
					break;
				case E_BIG:
					mFormatList->addElement(new CFontSizeFormatElement(element->mStart, index, 24, false));
					break;
				case E_SMALL:
					mFormatList->addElement(new CFontSizeFormatElement(element->mStart, index, 9, false));
					break;
				case E_BLOCKQUOTE:
					mFormatList->addElement(new CExcerptFormatElement(element->mStart, index));
					break;
				default:;
				}
			}
		}

		element.reset(mHStack.pop());
	}
}

void CParserHTML::ConvertAmpChar(cdustring& txt)
{
	std::ostrstream out;
	const unichar_t* p = txt.c_str();
	while(*p)
	{
		switch(*p)
		{
		case '&':
		{
			// Sniff what follows to see if it is valid
			const unichar_t* start = p + 1;
			const unichar_t* stop = start;
			while (*stop && (*stop != ';') && !isuspace(*stop))
				stop++;
			cdustring format(start, stop - start);
			
			if (HandleAmpChar(format, &out, NULL))
			{
				// Update input pointer
				p = stop;
				if (*p == ';')
					p++;
			}
			else
			{
				// Invalid & char - just write it out
				out.write((const char*)p, sizeof(unichar_t));
				p++;
			}
			break;
		}
		default:
			out.write((const char*)p, sizeof(unichar_t));
			p++;
			break;
		}
	}
	out << std::ends << std::ends;
	
	txt.steal(reinterpret_cast<unichar_t*>(out.str()));
}

bool CParserHTML::ParseURL(unichar_t* param, cdustring &outScheme, cdustring &outLocation)
{
	unichar_t* p = param;
	while(*p && ::unistrncmpnocase(p, "href", 4))
		p++;

	if (*p)
	{
		p += 4;
		while(isuspace(*p) || (*p == '=')) p++;

		// Parse out quoted string
		const unichar_t* url = NULL;
		char start = *p;
		if ((start == '\"') || (start == '\''))
		{
			// Bump paste opening quote
			url = ++p;

			// Parse to end of quote (no \ escape characters in HTML - %xx is used instead)
			while(*p && (*p != start)) p++;

			// Terminate and step over
			if (*p)
				*p++ = 0;
		}
		else if (*p)
		{
			// URL starts here
			url = p;
			
			// Parse to end or whitespace
			while(*p && !isuspace(*p)) p++;

			// Terminate and step over
			if (*p)
				*p++ = 0;
		}

		if (url && *url)
		{
			const unichar_t* r = ::unistrchr(url, ':');
			if (r)
			{
				outScheme.assign(url, r++ - url + 1);
				outLocation = r;
			}
			else
			{
				// Add base scheme if releative
				outScheme = mBaseScheme;
				outLocation = url;
			}
			ConvertAmpChar(outLocation);

			// Look for relative URL
			if (!outLocation.empty() && (outLocation[(cdstring::size_type)0] != '/'))
			{
				// Short circuit the common ones
				if (!::unistrncmpnocase(outScheme, "http:", 5) ||
					!::unistrncmpnocase(outScheme, "ftp:", 4) ||
					::unistrncmpnocase(outScheme, "mailto:", 7) &&
					::unistrncmpnocase(outScheme, "mid:", 4) &&
					::unistrncmpnocase(outScheme, "cid:", 4))
				{
					// Add base location
					outLocation = mBaseLocation + outLocation;
				}
			}
			return true;
		}
	}

	return false;
}

bool CParserHTML::ParseAnchor(unichar_t* param, cdustring &location)
{
	unichar_t* p = param;
	while(*p && ::unistrncmpnocase(p, "name", 4))
		p++;

	if (*p)
	{
		p += 4;
		while(isuspace(*p) || (*p == '=')) p++;

		// Parse out quoted string
		const unichar_t* name = NULL;
		if (*p == '\"')
		{
			// Bump paste opening quote
			name = ++p;

			// Parse to end of quote (no \ escape characters in HTML - %xx is used instead)
			while(*p && (*p != '\"')) p++;

			// Terminate and step over
			if (*p)
				*p++ = 0;
		}
		else if (*p)
		{
			// name starts here
			name = p;
			
			// Parse to end or whitespace
			while(*p && !isuspace(*p)) p++;

			// Terminate and step over
			if (*p)
				*p++ = 0;
		}

		location = name;
		ConvertAmpChar(location);
		return true;
	}

	return false;
}

void CParserHTML::DrawURL(long start, long stop)
{
	// Only if something present
	if (start == stop)
		return;

	mFormatList->addElement(new CColorFormatElement(start, stop, CPreferences::sPrefs->mURLStyle.GetValue().color));

	mFormatList->addElement(new CFaceFormatElement(start, stop, E_PLAIN));

	if (CPreferences::sPrefs->mURLStyle.GetValue().style & bold)
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_BOLD));

	if (CPreferences::sPrefs->mURLStyle.GetValue().style & underline)
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_UNDERLINE));

	if (CPreferences::sPrefs->mURLStyle.GetValue().style & italic)
		mFormatList->addElement(new CFaceFormatElement(start, stop, E_ITALIC));
}

void CParserHTML::DrawTag(long start, long stop)
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

const unichar_t* CParserHTML::Parse(int offset, bool for_display, bool quote, bool forward)
{
	unsigned long finalCount = 0;
	bool spaced = true;
	bool preformatted = false;
	int list_indent = 0;
	bool line_break = false;
	bool convert_CR_Space = false;
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
	const unichar_t* start;
	const unichar_t* stop;

	if (p)
	{
		while(*p)
		{
			switch(*p)
			{
			case '&':
				{
					// Sniff what follows to see if it is valid
					start = p + 1;
					stop = start;
					while (*stop && (*stop != ';') && !isuspace(*stop))
						stop++;
					std::auto_ptr<unichar_t> format(::unistrndup(start, stop - start));
					
					if (HandleAmpChar(format.get(), &out, &finalCount))
					{
						// Converted char already output
						spaced = false;
						line_break = false;
						convert_CR_Space = true;
						
						// Update input pointer
						p = stop;
						if (*p == ';')
							p++;
					}
					else
					{
						// Invalid & char - just write it out
						out.write((const char*)p, sizeof(unichar_t));
						p++;
						finalCount++;
						line_break = false;
						convert_CR_Space = true;
					}
					
					break;
				}
			case '\r':
			case '\n':
			case '\t':
				if (preformatted)
				{
					out.write((const char*)p, sizeof(unichar_t));
					finalCount++;
					line_break = (*p != '\t');
				}
				else if (convert_CR_Space && !spaced)
				{
					unichar_t space = ' ';
					out.write((const char*)&space, sizeof(unichar_t));
					finalCount++;
					spaced = true;
					line_break = false;
				}
				p++;
				break;
			case ' ':
				if(!spaced || preformatted)
				{
					out.write((const char*)p, sizeof(unichar_t));
					finalCount++;
					spaced = true;
					line_break = false;
				}
				p++;
				break;
			case '<':
			{
				p++;
				start = p;

				while(*p && (*p != '>')) p++;

				stop = p;
				if (*p)
					p++;

				std::auto_ptr<unichar_t> format(::unistrndup(start, stop - start));
				if (format.get() != NULL)
				{
					std::auto_ptr<unichar_t> lformat(::unistrdup(format.get()));
					::unistrlower(lformat.get());

					// Trim to starting tag
					{
						unichar_t* pformat = lformat.get();
						while(*pformat && !isuspace(*pformat))
							pformat++;
						*pformat = 0;
					}
					if (!::unistrcmp(lformat.get(), "p"))
					{
						// Must have line break before
						if (!line_break && finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
						}

						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;
					}

					else if (!::unistrcmp(lformat.get(), "br") || !::unistrcmp(lformat.get(), "div"))
					{
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;
					}

					else if (!::unistrcmp(lformat.get(), "hr"))
					{
						// Must have line break before
						if (finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
						}

						unichar_t ruler = '_';
						for(int hr = 0; hr < cRulerWidth; hr++)
							out.write((const char*)&ruler, sizeof(unichar_t));
						finalCount += cRulerWidth;

						if (mFormatList && mDoAlignment)
						{
							CAlignmentFormatElement* alignment = new CAlignmentFormatElement(finalCount + offset - cRulerWidth, finalCount + offset, E_CENTER);
							mFormatList->addElement(alignment);
						}

						// Must have line break after
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;
					}

					else if (!::unistrcmp(lformat.get(), "pre"))
					{
						preformatted = true;
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;

						// Do formatting change after CR added
						if (mFormatList)
							HandleFormat(format.get(), finalCount + offset);
					}
					else if (!::unistrcmp(lformat.get(), "/pre"))
					{
						// Do formatting change before CR added
						if (mFormatList)
							HandleFormat(format.get(), finalCount + offset);

						preformatted = false;
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;
					}


					// Must have breaks around blockquote
					else if (!::unistrcmp(lformat.get(), "blockquote"))
					{
						if (!line_break && finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
						}

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
						line_break = true;
						convert_CR_Space = false;
					}
					else if (!::unistrcmp(lformat.get(), "/blockquote"))
					{
						// Do formatting change before CR added
						if (mFormatList)
							HandleFormat(format.get(), finalCount + offset);

						// Switch to plain endls when depth goes to zero
						if ((quote_depth > 0) && (--quote_depth == 0))
						{
							end_line = plain_endl.c_str();
							end_line_len = plain_endl.length();
						}

						if (!line_break && finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
						}
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;
					}

					else if (!::unistrcmp(lformat.get(), "head"))
					{
						// Punt to /HEAD or BODY tag as that is where displayable items start
						while(*p)
						{
							if (*p == '<')
							{
								// Test for BODY or /HEAD tag
								if ((::unistrncmpnocase(p + 1, "body", 4) == 0) && ((*(p+5) == '>') || isuspace(*(p+5))))
									break;
									else if (::unistrncmpnocase(p + 1, "/head>", 6) == 0)
								{
									p += 7;
									break;
								}
							}
							
							p++;
						}
					}

					// These are keys that contain blocks of text that should be ignored
					else if (!::unistrcmp(lformat.get(), "applet"))
					{
						p = CommentTag(p, "applet", 6);
					}
					else if (!::unistrcmp(lformat.get(), "comment"))
					{
						p = CommentTag(p, "comment", 7);
					}
					else if (!::unistrcmp(lformat.get(), "script"))
					{
						p = CommentTag(p, "script", 6);
					}
					else if (!::unistrcmp(lformat.get(), "style"))
					{
						p = CommentTag(p, "style", 5);
					}
					else if (!::unistrcmp(lformat.get(), "title"))
					{
						p = CommentTag(p, "title", 5);
					}
					else if (!::unistrcmp(lformat.get(), "xml"))
					{
						p = CommentTag(p, "xml", 3);
					}
					
					// Special <!-- ...>...<... --> style comment
					else if (!::unistrncmp(lformat.get(), "!--", 3))
					{
						// See whether current tag ends with --
						if ((*(stop-1) != '-') || (*(stop-2) != '-'))
						{
							// Bump up to start of negating tag -->
							while(*p)
							{
								while(*p && (*p != '>'))
									p++;
								if (*p && (*(p-1) == '-') && (*(p-2) == '-'))
								{
									p++;
									break;
								}
								else
									p++;
							}
						}
					}

					// Back to normal key processing
					else if (!::unistrcmp(lformat.get(), "body"))
					{
						unichar_t* param = format.get();
						HandleBody(param);
					}
					else if (!::unistrcmp(lformat.get(), "img"))
					{
						unichar_t* param = format.get();
						HandleImage(param, &out, &finalCount);
					}
					else if (!::unistrcmp(lformat.get(), "basefont"))
					{
						unichar_t* param = format.get();
						cdustring mySize;

						while(*param)
						{
							if (!::unistrncmpnocase(param, "size", 4))
							{
								param += 4;
								FindParam(param, mySize);
								break;
							}
							else
								param++;
						}

						if (!mySize.empty())
						{
							//mBaseFontSize = atoi(mySize.c_str());
							CParserHTMLStackElement* element = new CParserHTMLStackElement(E_FONT, 0);
							cdustring myCopy("SIZE=");
							myCopy += mySize;
							element->mParam = myCopy;
							mHStack.push(element);
						}
					}

					else if (!::unistrcmp(lformat.get(), "base"))
					{
						if (ParseURL(format.get() + 4, mBaseScheme, mBaseLocation))
						{
							if (!mBaseScheme.empty() && !mBaseLocation.empty())
							{
								// Process base location
								const unichar_t* p = &mBaseLocation[mBaseLocation.length() - 1];
								if (*p != '/')
								{
									while((*--p != '/') && (p != mBaseLocation.c_str())) {}
									if (*p == '/')
										mBaseLocation = cdustring(mBaseLocation, p - mBaseLocation.c_str() + 1);
								}
							}
						}
					}

					// Handle tables - break before/end, break at each row
					else if (!::unistrcmp(lformat.get(), "table") || !::unistrcmp(lformat.get(), "tr"))
					{
						// Must have line break before
						if (!line_break && finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
						}
					}
					else if (!::unistrcmp(lformat.get(), "/table") || !::unistrcmp(lformat.get(), "/tr"))
					{
						// Must have line break after
						if (!line_break && finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
						}
					}

					// Handle start of lists
					else if (!::unistrcmp(lformat.get(), "ul") || !::unistrcmp(lformat.get(), "ol") || !::unistrcmp(lformat.get(), "dir"))
					{
						list_indent++;
						convert_CR_Space = false;
					}

					else if (!::unistrcmp(lformat.get(), "/ul") || !::unistrcmp(lformat.get(), "/ol") || !::unistrcmp(lformat.get(), "/dir"))
					{
						list_indent--;

						// Must have line break before
						if (!line_break && !list_indent)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
							convert_CR_Space = false;
						}
					}

					// Handle list item
					else if (!::unistrcmp(lformat.get(), "li"))
					{
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						for(int indent = 0; indent < list_indent; indent++)
						{
							unichar_t space2[] = { ' ', ' ' };
							out.write((const char*)space2, 2 * sizeof(unichar_t));
							finalCount += 2;
						}
						unichar_t bullet[2] = { 0x2022, ' ' };
						out.write((const char*)bullet, 2 * sizeof(unichar_t));
						finalCount += 2;
						line_break = false;
						convert_CR_Space = false;
					}

					// Handle definition list items
					else if (!::unistrcmp(lformat.get(), "dt") || !::unistrcmp(lformat.get(), "dl"))
					{
						out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
						finalCount += end_line_len;
						line_break = true;
						convert_CR_Space = false;
					}

					// Must have line breaks around headers
					else if ((*lformat.get() == 'h') && (format.get()[1] >= '1') && (format.get()[1] <= '6'))
					{
						// Must have line break before
						if (!line_break && finalCount)
						{
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
						}

						char c = format.get()[1];
						switch(c)
						{
						case '1':
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += 2*end_line_len;
							line_break = true;
							break;
						case '2':
						case '3':
						case '4':
						case '5':
						case '6':
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
							break;
						default:;
						}

						// Do formatting change after CR added
						if (mFormatList)
							HandleFormat(format.get(), finalCount + offset);

						convert_CR_Space = false;
					}
					else if ((*lformat.get() == '/') && (lformat.get()[1] == 'h') && (format.get()[1] >= '1') && (format.get()[1] <= '6'))
					{
						// Do formatting change before CR added
						if (mFormatList)
							HandleFormat(format.get(), finalCount + offset);

						unichar_t c = format.get()[2];
						switch(c)
						{
						case '1':
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += 2*end_line_len;
							line_break = true;
							break;
						case '2':
						case '3':
						case '4':
							out.write((const char*)end_line, end_line_len * sizeof(unichar_t));
							finalCount += end_line_len;
							line_break = true;
							break;
						default:;
						}
						convert_CR_Space = false;
					}
					else if (mFormatList)
						HandleFormat(format.get(), finalCount + offset);
				}
				break;
			}
			default:
				out.write((const char*)p, sizeof(unichar_t));
				p++;
				finalCount++;
				spaced = false;
				line_break = false;
				convert_CR_Space = true;
				break;
			}
		}
	}

	if (mFormatList)
		FlushStack(finalCount + offset);
	out << std::ends << std::ends;
	return reinterpret_cast<const unichar_t*>(out.str());
}

// Bump over tag pairs that contain comments
const unichar_t* CParserHTML::CommentTag(const unichar_t* start, const char* tag, size_t len)
{
	// Bump up to start of negating tag
	while(*start && ((*start != '<') || (*(start + 1) != '/') || (::unistrncmpnocase(start + 2, tag, len) != 0)))
		start++;

	// Bump to end of negating tag
	while(*start && (*start != '>'))
		start++;
	if (*start)
		start++;

	return start;
}

void CParserHTML::RawParse(int offset)
{
	const unichar_t* p = mTxt;
	if (!p)
		return;

	int currPos = offset;

	while(*p)
	{
		if (*p == '<')
		{
			int startPos = currPos;

			// Special case '<!--' tags
			if ((p[1] == '!') && (p[2] == '-') && (p[3] == '-'))
			{
				while(*p && ((*p != '>') || (p[-1] != '-') || (p[-2] != '-')))
				{
					p++;
					currPos++;
				}
			}
			else
			{
				while(*p && (*p != '>'))
				{
					p++;
					currPos++;
				}
			}

			if (*p)
			{
				p++;
				currPos++;
			}

			int stopPos = currPos;

			if (mFormatList)
				DrawTag(startPos, stopPos);
		}
		else if ((*p == '&') && !isuspace(p[1]))
		{
			// Sniff what follows to see if it is valid
			const unichar_t* start = p + 1;
			const unichar_t* stop = start;
			while (*stop && (*stop != ';') && !isuspace(*stop))
				stop++;
			unichar_t* format = ::unistrndup(start, stop - start);

			bool valid_amp = HandleAmpChar(format, NULL, NULL);
			delete format;
			if (valid_amp)
			{
				int startPos;
				int stopPos;
				startPos = currPos;
				while(*p && (*p != ';'))
				{
					p++;
					currPos++;
				}

				if (*p)
				{
					p++;
					currPos++;
				}

				stopPos = currPos;

				if (mFormatList)
					DrawTag(startPos, stopPos);
			}
			else
			{
				p++;
				currPos++;
			}
		}
		else
		{
			p++;
			currPos++;
		}
	}
}
