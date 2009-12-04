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


// Header for CSMTPAccountPopup class

#ifndef __CSMTPACCOUNTPOPUP__MULBERRY__
#define __CSMTPACCOUNTPOPUP__MULBERRY__

#include <LView.h>

// Classes
class LIconControl;
class LPopupButton;

class CSMTPAccountPopup : public LView
{
public:
	enum { class_ID = 'SMaP' };

					CSMTPAccountPopup(
							const SPaneInfo& inPaneInfo,
							const SViewInfo& inViewInfo,
							MessageT		inValueMessage,
							ResIDT			inMENUid,
							ResIDT			inICONid,
							ConstStringPtr	inTitle);
	virtual 		~CSMTPAccountPopup();

	LPopupButton*	GetPopup() const
	{
		return mAccountsPopup;
	}
	LIconControl*	GetSecureIndicator() const
	{
		return mSecure;
	}

private:
	LPopupButton*	mAccountsPopup;
	LIconControl*	mSecure;
	
	void InitControls(const SPaneInfo& inPaneInfo,
							MessageT		inValueMessage,
							ResIDT			inMENUid,
							ResIDT			inICONid,
							ConstStringPtr	inTitle);

	void InitAccountPopup();
};

#endif
