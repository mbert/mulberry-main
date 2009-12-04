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


// Header for CFontPopup class

#ifndef __CFONTPOPUP__MULBERRY__
#define __CFONTPOPUP__MULBERRY__

#include "CMenuPopup.h"

// Classes

class CFontPopup : public CMenuPopup
{

public:
	enum { class_ID = 'Font' };

					CFontPopup(LStream *inStream);
	virtual 		~CFontPopup();

	virtual void	SetFontName(const Str255 fontName);
	virtual void	GetFontName(Str255 fontName);

protected:
	virtual void 	FinishCreateSelf(void);
	
private:
	static MenuHandle	sFontMenu;
	static MenuHandle	sFontPopup;
	
};

#endif
