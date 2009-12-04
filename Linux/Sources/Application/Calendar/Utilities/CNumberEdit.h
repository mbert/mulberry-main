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

#ifndef H_CNUMBEREDIT
#define H_CNUMBEREDIT

#include "CTextField.h"

// Classes
class JXMultiImageButton;

class	CNumberEdit : public CTextInputField
{
public:
				CNumberEdit(const JCharacter* text, JXContainer* enclosure,
								JXTextMenu* menu,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);

				CNumberEdit(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h);
	virtual		~CNumberEdit() {}

	void OnCreate(long min, long max, unsigned long filler = 0);					// Do odds & ends
	void SetRange(long min, long max, unsigned long filler = 0);

	long		GetNumberValue() const;
	void		SetNumberValue(long value);
	
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual void	Show();					// must call inherited
	virtual void	Hide();					// must call inherited
	virtual void	Activate();				// must call inherited
	virtual void	Deactivate();			// must call inherited

protected:
	JXMultiImageButton*			mSpinUp;
	JXMultiImageButton*			mSpinDown;
	long						mMin;
	long						mMax;
	unsigned long				mFiller;

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
	void Nudge(bool up);
	void ValidNumber();
};

#endif
