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


// Header for CSizePopup class

#ifndef __CSIZEPOPUP__MULBERRY__
#define __CSIZEPOPUP__MULBERRY__

#include "CMenuPopup.h"

// Consts

enum
{
	eSizePopup8 = 1,
	eSizePopup9,
	eSizePopup10,
	eSizePopup11,
	eSizePopup12,
	eSizePopup13,
	eSizePopup14,
	eSizePopup16,
	eSizePopup18,
	eSizePopup20,
	eSizePopup24,
	eSizePopupDummy,
	eSizePopupOther
};

// Classes

class CSizePopup : public CMenuPopup
{

public:
	enum { class_ID = 'Size' };

					CSizePopup(LStream *inStream);
	virtual 		~CSizePopup();

	virtual void	SetSize(SInt16 size);
	virtual SInt16	GetSize(void) const;
	virtual void FinishCreateSelf();
	
protected:
	static MenuHandle sSizeMenu;
	static MenuHandle sSizePopup;

};

#endif
