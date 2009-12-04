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


#include "CFormatList.h"

CFormatList::CFormatList()
{
	mList = NULL;
}

CFormatList::~CFormatList()
{
	CFormatElement* item = mList;
	while(item)
	{
		CFormatElement* item_next = item->next;
		delete item;
		item = item_next;
	}
}

void CFormatList::addElement(CFormatElement* element)
{
	CFormatElement *curr = mList;
	CFormatElement *prev = NULL;

	int start = element->getStart();

	bool done=false;

	if (curr == NULL)
	{
		mList = element;
		element->next = NULL;
	}
	else
	{

		while(curr != NULL && !done)
		{
			if (curr->getStart() <= start)
			{
				prev = curr;
				curr = curr->next;
			}
			else
				done = true;
		}

		if (prev)
			prev->next = element;

		element->next = curr;

		if (prev == NULL)
			mList = element;
	}
}

CFormatElement *CFormatList::GetElement()
{
	return mList;
}

void CFormatList::draw(CDisplayFormatter *display)
{
	for(CFormatElement *curr = mList; curr; curr=curr->next)
		curr->draw(display);
}
