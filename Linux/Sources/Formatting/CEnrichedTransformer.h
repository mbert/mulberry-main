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


#ifndef __CENRICHEDTRANSFORMER__MULBERRY__
#define __CENRICHEDTRANSFORMER__MULBERRY__

#include "cdstring.h"
#include "cdustring.h"

#include <strstream>
#include <JTextEditor16.h>

class CParserEnrichedStack;
class CParserEnrichedStackElement;
class CFormattedTextDisplay;

class CEnrichedUtils
{
public:
	static char* ConvertToHTML(const char* enrichedText);				// UTF8 in, UTF8 out

	static char* ToEnriched(const char* text);							// UTF8 in, UTF8 out
	static char* ToEnriched(const unichar_t* text);						// UTF16 in, UTF8 out

	static void Write(std::ostrstream& sout, const unichar_t* string, size_t length);				// UTF8 in, UTF8 out
	static void WriteNoEndlDouble(std::ostrstream& sout, const unichar_t* string, size_t length);	// UTF8 in, UTF8 out

	static void WriteHTML(char ch, std::ostream &out);
};

class CEnrichedTransformer
{
public:
	CEnrichedTransformer(CFormattedTextDisplay* display);
	~CEnrichedTransformer();

	char* Transform();

private:
	std::ostrstream 		mOut;
	CFormattedTextDisplay*	mTextPane;
	int						mCurrentSize;
	int						mDefaultSize;
	cdstring				mDefaultFont;
	CParserEnrichedStack*	mStack;

	void ExamineStack(const JTextEditor16::Font& wRunInfo);
	void FlushStack();

	void AddElement(CParserEnrichedStackElement* element);
	void RemoveElement(CParserEnrichedStackElement* element);
};


#endif
