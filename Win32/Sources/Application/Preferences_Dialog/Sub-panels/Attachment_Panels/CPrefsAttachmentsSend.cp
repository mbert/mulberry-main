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


// CPrefsAttachmentsSend.cpp : implementation file
//


#include "CPrefsAttachmentsSend.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsSend property page

IMPLEMENT_DYNCREATE(CPrefsAttachmentsSend, CTabPanel)

CPrefsAttachmentsSend::CPrefsAttachmentsSend() : CTabPanel(CPrefsAttachmentsSend::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAttachmentsSend)
	mDefault_mode = -1;
	mDefault_Always = -1;
	//}}AFX_DATA_INIT
}

CPrefsAttachmentsSend::~CPrefsAttachmentsSend()
{
}

void CPrefsAttachmentsSend::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAttachmentsSend)
	DDX_Control(pDX, IDC_DEFAULTENCODING, mEncodingCtrl);

	DDX_CBIndex(pDX, IDC_DEFAULTENCODING, mDefault_mode);
	DDX_Radio(pDX, IDC_ENCODEALWAYS, mDefault_Always);
	DDX_Check(pDX, IDC_CHECKDEFAULTMAILTO, mCheckDefaultMailClient);
	DDX_Check(pDX, IDC_WARNMAILTOFILES, mWarnMailtoFiles);
	DDX_Check(pDX, IDC_CHECKDEFAULTWEBCAL, mCheckDefaultWebcalClient);
	DDX_Check(pDX, IDC_WARNMISSINGATTACHMENTS, mWarnMissingAttachments);
	DDX_Check(pDX, IDC_MISSINGATTACHMENTSUBJECT, mMissingAttachmentSubject);
	DDX_UTF8Text(pDX, IDC_MISSIGNATTACHMENTWORDS, mMissingAttachmentWords);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAttachmentsSend, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAttachmentsSend)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Set up params for DDX
void CPrefsAttachmentsSend::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Set values
	switch(copyPrefs->mDefault_mode.GetValue())
	{
	case eUUMode:
		mDefault_mode = 0;
		break;
	case eBinHex4Mode:
		mDefault_mode = 1;
		break;
	case eAppleSingleMode:
		mDefault_mode = 2;
		break;
	case eAppleDoubleMode:
		mDefault_mode = 3;
		break;
	}
	mDefault_Always = !copyPrefs->mDefault_Always.GetValue();

	// Add encoding to list
	CString str;
	str.LoadString(IDS_ENCODING_UU);
	mEncodingCtrl.AddString(str);
	str.LoadString(IDS_ENCODING_BINHEX4);
	mEncodingCtrl.AddString(str);
	str.LoadString(IDS_ENCODING_AS);
	mEncodingCtrl.AddString(str);
	str.LoadString(IDS_ENCODING_AD);
	mEncodingCtrl.AddString(str);
	mEncodingCtrl.SetCurSel(mDefault_mode);
	
	mCheckDefaultMailClient = copyPrefs->mCheckDefaultMailClient.GetValue();
	mWarnMailtoFiles = copyPrefs->mWarnMailtoFiles.GetValue();
	mCheckDefaultWebcalClient = copyPrefs->mCheckDefaultWebcalClient.GetValue();
	mWarnMissingAttachments = copyPrefs->mWarnMissingAttachments.GetValue();
	mMissingAttachmentSubject = copyPrefs->mMissingAttachmentSubject.GetValue();
	
	mMissingAttachmentWords.clear();
	for(cdstrvect::const_iterator iter = copyPrefs->mMissingAttachmentWords.GetValue().begin();
					iter != copyPrefs->mMissingAttachmentWords.GetValue().end(); iter++)
	{
		mMissingAttachmentWords += *iter + "\r\n";
	}
}

// Get params from DDX
bool CPrefsAttachmentsSend::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	switch(mDefault_mode)
	{
	case 0:
		copyPrefs->mDefault_mode.SetValue(eUUMode);
		break;
	case 1:
		copyPrefs->mDefault_mode.SetValue(eBinHex4Mode);
		break;
	case 2:
		copyPrefs->mDefault_mode.SetValue(eAppleSingleMode);
		break;
	case 3:
		copyPrefs->mDefault_mode.SetValue(eAppleDoubleMode);
		break;
	}
	copyPrefs->mDefault_Always.SetValue(!mDefault_Always);

	copyPrefs->mCheckDefaultMailClient.SetValue(mCheckDefaultMailClient);
	copyPrefs->mWarnMailtoFiles.SetValue(mWarnMailtoFiles);
	copyPrefs->mCheckDefaultWebcalClient.SetValue(mCheckDefaultWebcalClient);
	copyPrefs->mWarnMissingAttachments.SetValue(mWarnMissingAttachments);
	copyPrefs->mMissingAttachmentSubject.SetValue(mMissingAttachmentSubject);

	cdstrvect accumulate;
	cdstring temp(mMissingAttachmentWords);
	char* p = ::strtok(temp.c_str_mod(), "\r\n");
	while(p)
	{
		accumulate.push_back(p);
		p = ::strtok(NULL, "\r\n");
	}	
	copyPrefs->mMissingAttachmentWords.SetValue(accumulate);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachmentsSend message handlers

