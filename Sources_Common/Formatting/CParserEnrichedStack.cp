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


#include "CParserEnrichedStack.h"

CParserEnrichedStackElement::CParserEnrichedStackElement(ETag tag, int s)
{
	mTagid = tag;
	mStart = s;
	mNext = NULL;
}

CParserEnrichedStackElement::~CParserEnrichedStackElement()
{
}


CParserEnrichedStack::CParserEnrichedStack()
{
	mStack = NULL;
}

CParserEnrichedStack::~CParserEnrichedStack()
{
	deleteStack(mStack);
}

void CParserEnrichedStack::deleteStack(CParserEnrichedStackElement *member)
{
	if (member)
	{
		deleteStack(member->mNext);
		delete member;
	}
}

void CParserEnrichedStack::push(CParserEnrichedStackElement *element)
{
	if(element)
	{
		element->mNext = mStack;
		mStack = element;
	}
}

CParserEnrichedStackElement *CParserEnrichedStack::pop()
{
	CParserEnrichedStackElement *give = mStack;
	if (mStack)
		mStack = mStack->mNext;
	else
		mStack = NULL;
	return give;
}

CParserEnrichedStackElement *CParserEnrichedStack::pop(ETag desired)
{
	CParserEnrichedStackElement *currElement, *prevElement = nil, *give;


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
	return nil;
}
