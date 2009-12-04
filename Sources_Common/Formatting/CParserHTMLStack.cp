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


#include "CParserHTMLStack.h"

CParserHTMLStackElement::CParserHTMLStackElement(ETag tag, int s)
{
	mTagid = tag;
	mStart = s;
	mNext = NULL;
}

CParserHTMLStackElement::~CParserHTMLStackElement()
{
}


CParserHTMLStack::CParserHTMLStack()
{
	mStack = NULL;
}

CParserHTMLStack::~CParserHTMLStack()
{
	deleteStack(mStack);
}

void CParserHTMLStack::deleteStack(CParserHTMLStackElement *member)
{
	if (member)
	{
		deleteStack(member->mNext);
		delete member;
	}
}

void CParserHTMLStack::push(CParserHTMLStackElement *element)
{
	if (element)
	{
		element->mNext = mStack;
		mStack = element;
	}
}

CParserHTMLStackElement *CParserHTMLStack::pop()
{
	CParserHTMLStackElement* give = mStack;
	if (mStack)
		mStack = mStack->mNext;
	else
		mStack = NULL;
	return give;
}

CParserHTMLStackElement* CParserHTMLStack::pop(ETag desired)
{
	CParserHTMLStackElement* currElement = NULL;
	CParserHTMLStackElement* prevElement = NULL;
	CParserHTMLStackElement* give = NULL;


	for(currElement = mStack; currElement; currElement = currElement->mNext)
	{
		if (currElement->mTagid == desired)
		{
			give = currElement;
			if (prevElement)
				prevElement->mNext = currElement->mNext;
			else
				mStack = currElement->mNext;
			return give;
		}
		prevElement = currElement;
	}
	return NULL;
}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
CParserHTMLFontStackElement::CParserHTMLFontStackElement(ETag tag, int start, RGBColor color, int size, int font) : CParserHTMLStackElement(tag, start)
#elif __dest_os == __win32_os || __dest_os == __linux_os
CParserHTMLFontStackElement::CParserHTMLFontStackElement(ETag tag, int start, RGBColor color, int size, const char* font) : CParserHTMLStackElement(tag, start)
#else
#error __dest_os
#endif
{
	mColor = color;
	mSize = size;
	mFont = font;

}

CParserHTMLFontStackElement::~CParserHTMLFontStackElement()
{
}
