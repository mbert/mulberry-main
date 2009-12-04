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

#ifndef __CCOLORMENU__MULBERRY__
#define __CCOLORMENU__MULBERRY__

#include "CPopupButton.h"

// Classes

class CColorPopup : public CPopupButton
{

	DECLARE_DYNCREATE(CColorPopup)

public:
						CColorPopup();
	virtual 			~CColorPopup();

	void 				OnUpdateColor(CCmdUI* pCmdUI, bool enable);
	void 				DrawContent(LPDRAWITEMSTRUCT lpDIS);

	void				SetColor(RGBColor color);
	RGBColor 			RunPicker(void);

private:
	RGBColor 			mColor;
	
protected:
	DECLARE_MESSAGE_MAP()
};

#endif
