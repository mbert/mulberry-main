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

// Header for CSpellChooseDictDialog class

#ifndef __CSPELLCHOOSEDICTDIALOG__MULBERRY__
#define __CSPELLCHOOSEDICTDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_SpellChooseDictDialog = 20500;
const	PaneIDT		paneid_SpellChooseDictDictionaries = 'DICT';

// Resources

// Classes
class	LPopupButton;
class	CSpellPlugin;

class	CSpellChooseDictDialog : public LDialogBox
{
public:
	enum { class_ID = 'SpCh' };

					CSpellChooseDictDialog();
					CSpellChooseDictDialog(LStream *inStream);
	virtual 		~CSpellChooseDictDialog();

	static bool PoseDialog(CSpellPlugin* speller);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

private:
	LPopupButton*	mDictionaries;

			void	SetSpeller(CSpellPlugin* speller);			// Set the speller
			void	GetOptions(CSpellPlugin* speller);			// Update options
			
			void	InitDictionaries(CSpellPlugin* speller);
};

#endif
