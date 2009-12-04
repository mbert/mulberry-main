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


// Header for CMenuPopup class

#ifndef __CMENUPOPUP__MULBERRY__
#define __CMENUPOPUP__MULBERRY__

#include <LGAPopup.h>

// Classes

class CMenuPopup : public LGAPopup
{

public:
	enum { class_ID = 'mPop' };

					CMenuPopup(LStream *inStream);
	virtual 		~CMenuPopup();

	
		
	virtual	void		SetMacMenuH(MenuHandle inMacMenuH)				// Do nothing
							{ }
	virtual	MenuHandle	GetMacMenuH(void)
							{ return mPopup; }				// Use the common Menu handle
	virtual	MenuHandle	LoadPopupMenuH(void) const						// Do nothing
							{ return nil; }

	virtual	void	SetValue(Int32 inValue);

	void 				Ambiguate();
	void				SaveState();
	void 				RestoreState();
	virtual void		FinishCreateSelf();
protected:
	
	virtual	void	HandlePopupMenuSelect(Point inPopupLoc,
											Int16 inCurrentItem,
											Int16 &outMenuID,
											Int16 &outMenuItem );		// Delete mark after popup
	
	MenuHandle	mMenu;
	MenuHandle  mPopup;
	
	Int32 defaultValue;
	bool mAttachMenu;
	
private:
	Int32 savedValue;
	void 				ClearMarks();
};

#endif
