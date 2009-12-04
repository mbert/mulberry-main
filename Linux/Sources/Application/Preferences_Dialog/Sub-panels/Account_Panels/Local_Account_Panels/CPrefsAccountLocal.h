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


// Header for CPrefsAccountLocal class

#ifndef __CPREFSACCOUNTLOCAL__MULBERRY__
#define __CPREFSACCOUNTLOCAL__MULBERRY__

#include "CTabPanel.h"

#include "HPopupMenu.h"

// Classes
class CTextInputDisplay;
class JXStaticText;
class JXRadioGroup;
class JXTextButton;

class CPrefsAccountLocal : public CTabPanel
{
// Construction
public:
	CPrefsAccountLocal(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

			void	SetLocalAddress()
		{ mLocalAddress = true; }
			void	SetDisconnected()
		{ mDisconnected = true; }

protected:
// begin JXLayout1

    JXRadioGroup*      mLocationGroup;
    CTextInputDisplay* mPath;
    JXRadioGroup*      mRelativeGroup;
    JXTextButton*      mChooseBtn;
    JXStaticText*      mFileFormatTitle;
    HPopupMenu*        mFileFormatPopup;

// end JXLayout1
	bool			mLocalAddress;
	bool			mDisconnected;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void	SetUseLocal(bool use);
			void	SetPath(const char* path);
			void	DoChooseLocalFolder();				// Choose local folder using browser
			void	SetRelative();
			void	SetAbsolute();
};

#endif
