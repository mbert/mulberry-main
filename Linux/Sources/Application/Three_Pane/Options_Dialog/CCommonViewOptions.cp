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


// Source for CCommonViewOptions class

#include "CCommonViewOptions.h"

#include "CUserActionOptions.h"

// Static members

void CCommonViewOptions::SetCommonViewData(const CUserAction& listPreview,
											const CUserAction& listFullView,
											const CUserAction& itemsPreview,
											const CUserAction& itemsFullView,
											bool is3pane)
{
	mListPreview->SetData(listPreview);
	mListFullView->SetData(listFullView);
	mItemsPreview->SetData(itemsPreview);
	mItemsFullView->SetData(itemsFullView);
	
	if (!is3pane)
		mListPreview->DisableItems();
}

void CCommonViewOptions::GetCommonViewData(CUserAction& listPreview,
											CUserAction& listFullView,
											CUserAction& itemsPreview,
											CUserAction& itemsFullView)
{
	mListPreview->GetData(listPreview);
	mListFullView->GetData(listFullView);
	mItemsPreview->GetData(itemsPreview);
	mItemsFullView->GetData(itemsFullView);
}
