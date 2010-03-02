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


#ifndef __CHTMLTRANSFORMER__MULBERRY__
#define __CHTMLTRANSFORMER__MULBERRY__

#include "cdstring.h"
#include "cdustring.h"

#include <strstream>

class CParserHTMLStack;
class CParserHTMLStackElement;
class CLetterTextEditView;

class CHTMLUtils
{
public:
	struct SFontParam
	{
		bool	mFace;
		int		mSize;
		bool	mColor;
		
		SFontParam(bool face, int size, bool color) :
			mFace(face), mSize(size), mColor(color) {}
		SFontParam(const SFontParam& copy) :
			mFace(copy.mFace), mSize(copy.mSize), mColor(copy.mColor) {}
	};
	typedef std::vector<CHTMLUtils::SFontParam> CFontStack;

	static char* ConvertToEnriched(const char* htmlText);	// UTF8 in, UTF8 out

	static char* ToHTML(const char* text);					// UTF8 in, UTF8 out
	static char* ToHTML(const unichar_t* text);				// UTF16 in, UTF8 out

	static void Write(std::ostrstream& sout, const unichar_t* string, size_t length);	// UTF16 in, UTF8 out

private:

	static void ConvertFontTag(std::ostrstream& sout, const char* param, int& size, CFontStack& myStack);	// UTF8 in, UTF8 out

};

class CHTMLTransformer
{
public:
	CHTMLTransformer(CLetterTextEditView* display);
	~CHTMLTransformer();

	char* Transform();

private:
	std::ostrstream				mOut;
	CLetterTextEditView*	mText;
	int						mCurrentSize;
	int						mDefaultSize;
	cdstring				mDefaultFont;
	CParserHTMLStack*		mStack;

	void ExamineStack(CHARFORMAT2 format, PARAFORMAT paraFormat);
	void FlushStack();

	void AddElement(CParserHTMLStackElement* element);
	void RemoveElement(CParserHTMLStackElement* element);
	void RemoveAnchor();
};


#endif
