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


// CMessagePartProp.cp : implementation file
//


#include "CMessagePartProp.h"

#include "CAttachment.h"
#include "CMIMESupport.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMessagePartProp dialog

BEGIN_MESSAGE_MAP(CMessagePartProp, CHelpDialog)
	//{{AFX_MSG_MAP(CMessagePartProp)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


CMessagePartProp::CMessagePartProp(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CMessagePartProp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessagePartProp)
	//}}AFX_DATA_INIT
}

CMessagePartProp::~CMessagePartProp()
{
}

void CMessagePartProp::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CMessagePartProp)
	DDX_Control(pDX, IDCANCEL, mCancelAll);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_NAME, mName);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_TYPE, mType);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_ENCODING, mEncoding);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_ID, mID);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_DISPOSITION, mDisposition);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_DESCRIPTION, mDescription);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_PARAMS, mParams);
	DDX_UTF8Text(pDX, IDC_MPARTSPROP_OPENWITH, mOpenWith);
	//}}AFX_DATA_MAP
	
	if (!mMultiview)
		mCancelAll.ShowWindow(SW_HIDE);
}


/////////////////////////////////////////////////////////////////////////////
// CMessagePartProp message handlers
void CMessagePartProp::SetFields(CAttachment& attach, bool multi)
{
	CMIMEContent& content = attach.GetContent();

	mMultiview = multi;

	mName = attach.GetMappedName(true, true);
	mType = content.GetContentTypeText() + "/" + content.GetContentSubtypeText();
	mEncoding = content.GetTransferEncodingText();
	mID = content.GetContentId();
	mDisposition = content.GetContentDispositionText();
	mDescription = content.GetContentDescription();
	for(int i = 0; i < content.CountContentParameters(); i++)
	{
		mParams += content.GetContentParameterText(i);
		mParams += "\r\n";
	}
	mOpenWith = CMIMESupport::MapMIMEToApp(attach);
}