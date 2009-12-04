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


// Header for CPrefsAccountAuth class

#ifndef __CPREFSACCOUNTAUTH__MULBERRY__
#define __CPREFSACCOUNTAUTH__MULBERRY__

#include "CTabPanel.h"

#include "CAuthenticator.h"

#include "HPopupMenu.h"

// Classes
class CPrefsAuthPanel;
class CINETAccount;
class JXCardFile;
class JXStaticText;
class JXTextCheckbox;

class CPrefsAccountAuth : public CTabPanel
{
// Construction
public:
	CPrefsAccountAuth(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) { mCurrentPanel = NULL; }

	virtual void OnCreate();

	virtual void SetData(void* data);			// Set data
	virtual bool UpdateData(void* data);		// Force update of data

protected:
// begin JXLayout1

    HPopupMenu*     mAuthPopup;
    JXCardFile*     mAuthSubPanel;
    JXStaticText*   mTLSPopupTitle;
    HPopupMenu*     mTLSPopup;
    JXTextCheckbox* mUseTLSClientCert;
    HPopupMenu*     mTLSClientCert;

// end JXLayout1
	cdstring			mAuthType;
	CPrefsAuthPanel*	mCurrentPanel;

	cdstrvect			mCertSubjects;
	cdstrvect			mCertFingerprints;

	virtual void Receive(JBroadcaster* sender, const Message& message);

			void AddPanel(CPrefsAuthPanel* panel);
			void SetAuthPanel(const cdstring& auth_type);		// Set auth panel
			void BuildAuthPopup(CINETAccount* account);
			void InitTLSPopup(CINETAccount* account);
			void BuildCertPopup();
			void OnAuthPopup();
			void TLSItemsState();
};

#endif
