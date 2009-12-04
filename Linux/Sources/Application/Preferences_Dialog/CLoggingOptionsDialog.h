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


/////////////////////////////////////////////////////////////////////////////
// CLoggingOptionsDialog1 dialog

#ifndef __CLOGGINGOPTIONSDIALOG__MULBERRY__
#define __CLOGGINGOPTIONSDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CLog.h"

class JXTextButton;
class JXTextCheckbox;

class CLoggingOptionsDialog : public CDialogDirector
{
// Construction
public:
	CLoggingOptionsDialog(JXDirector* supervisor);

	static bool PoseDialog(CLog::SLogOptions& options);

protected:
	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void	SetOptions(const CLog::SLogOptions& options);		// Set options in dialog
	void	GetOptions(CLog::SLogOptions& options);				// Get options from dialog

// begin JXLayout

    JXTextCheckbox* mActivate;
    JXTextCheckbox* mIMAP;
    JXTextCheckbox* mPOP3;
    JXTextCheckbox* mSMTP;
    JXTextCheckbox* mIMSP;
    JXTextCheckbox* mACAP;
    JXTextCheckbox* mHTTP;
    JXTextButton*   mOKBtn;
    JXTextButton*   mCancelBtn;
    JXTextCheckbox* mAuthentication;
    JXTextCheckbox* mPlayback;
    JXTextCheckbox* mPlugins;
    JXTextCheckbox* mFiltering;
    JXTextCheckbox* mErrors;
    JXTextCheckbox* mOverwrite;
    JXTextButton*   mClearBtn;
    JXTextButton*   mFlushBtn;

// end JXLayout
	JXTextCheckbox*	mLogs[CLog::eLogTypeLast];
};

#endif
