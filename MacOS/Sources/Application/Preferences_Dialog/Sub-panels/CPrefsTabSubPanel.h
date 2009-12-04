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


// Header for CPrefsTabSubPanel class

#ifndef __CPREFSTABSUBPANEL__MULBERRY__
#define __CPREFSTABSUBPANEL__MULBERRY__


// Constants

// Classes
class CPrefsTabSubPanel : public LView
{
public:
	enum { class_ID = 'Dpan' };

					CPrefsTabSubPanel() {}
					CPrefsTabSubPanel(LStream *inStream) : LView(inStream) {}
	virtual 		~CPrefsTabSubPanel() {}

	virtual void	SetState(unsigned long state)
		{ mState = state; }

	virtual void	ToggleICDisplay(bool IC_on) {}		// Toggle display of IC
	virtual void	SetData(void* data) = 0;			// Set data
	virtual void	UpdateData(void* data) = 0;			// Force update of data

	virtual void	SavePlace(LStream *outPlace)
		{ /*LPane::SavePlace(outPlace);*/ }
	virtual void	RestorePlace(LStream *inPlace)
		{ /*LPane::RestorePlace(inPlace);*/ }

protected:
	unsigned long mState;
};

#endif
