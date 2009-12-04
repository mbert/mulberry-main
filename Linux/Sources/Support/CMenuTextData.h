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


// CMenuTextData.h - UI widget that implements a menu in the menu bar

#ifndef __CMENUTEXTDATA__MULBERRY__
#define __CMENUTEXTDATA__MULBERRY__

#include <JXTextMenuData.h>

class CMenuTextData : public JXTextMenuData
{
public:

	CMenuTextData(JXTextMenu* menu) : JXTextMenuData(menu)
		{}

	virtual ~CMenuTextData() {}

	JBoolean	GetImageID(const JIndex index, const JSize* imageID) const;
	void		SetImageID(const JIndex index, JSize imageID,
						 const JBoolean menuOwnsImage);
	void		ClearImage(const JIndex index);

private:

	// not allowed

	CMenuTextData(const CMenu& source);
	const CMenuTextData& operator=(const CMenuTextData& source);
};

#endif
