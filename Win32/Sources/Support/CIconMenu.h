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


// CIconMenu

#ifndef __CICONMENU__MULBERRY__
#define __CICONMENU__MULBERRY__

class CIconMenu : public CMenu
{
public:
	
	struct SIconMenuData
	{
		cdstring	mTxt;
		UINT		mIcon;

		SIconMenuData(const cdstring& txt, UINT icon) :
			mTxt(txt), mIcon(icon) {}
		
	};
	CIconMenu();
	~CIconMenu();
	
	SIconMenuData* AddData(SIconMenuData* data)
	{
		mData.push_back(data);
		return data;
	}

protected:
	ptrvector<SIconMenuData> mData;

	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );
};

#endif
