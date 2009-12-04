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


#include "CClickList.h"

CClickList::CClickList(){
	list = NULL;
}


CClickList::~CClickList()
{
	CClickElement* item = list;
	while(item)
	{
		CClickElement* item_next = item->next;
		delete item;
		item = item_next;
	}
}


void CClickList::addElement(CClickElement *element)
{
	CClickElement *curr = list;
	CClickElement *prev = NULL;

	int start = element->getStart();

	bool done=false;

	if (curr == NULL)
	{
		list = element;
		element->next = NULL;
	}
	else
	{
		while(curr != NULL && !done)
		{
			if(curr->getStart() < start)
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
			list = element;
	}
}

void CClickList::pop_front()
{
	if (list)
		list = list->next;
}

CClickElement *CClickList::findCursor(int offset)
{
	CClickElement *done = NULL;
	for(CClickElement *curr = list; curr && !done; curr = curr->next)
	{
		if(curr->findCursor(offset))
			done = curr;
	}
	return done;
}
