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


// CLetterPartProp.cp : implementation file
//


#include "CLetterPartProp.h"

#include "CAttachment.h"
#include "CMIMESupport.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CLetterPartProp dialog

BEGIN_MESSAGE_MAP(CLetterPartProp, CHelpDialog)
	//{{AFX_MSG_MAP(CLetterPartProp)
		// NOTE: the ClassWizard will add message map macros here
	ON_CBN_EDITCHANGE(IDC_LPARTSPROP_TYPE, OnEditChangeType)
	ON_CBN_SELCHANGE(IDC_LPARTSPROP_TYPE, OnSelChangeType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CLetterPartProp::CLetterPartProp(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CLetterPartProp::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLetterPartProp)
	mEncoding = -1;
	mDisposition = -1;
	mCharset = -1;
	mHasDisposition = true;
	//}}AFX_DATA_INIT
}

CLetterPartProp::~CLetterPartProp()
{
}

void CLetterPartProp::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLetterPartProp)
	DDX_Control(pDX, IDC_LPARTSPROP_TYPE, mTypeCtrl);
	DDX_Control(pDX, IDC_LPARTSPROP_SUBTYPE, mSubtypeCtrl);
	DDX_Control(pDX, IDC_LPARTSPROP_ENCODING, mEncodingCtrl);
	DDX_Control(pDX, IDC_LPARTSPROP_DISPOSITION, mDispositionCtrl);
	DDX_Control(pDX, IDC_LPARTSPROP_CHARSET, mCharsetCtrl);
	DDX_Control(pDX, IDC_LPARTSPROP_CHARSET_TITLE, mCharsetTitleCtrl);
	DDX_UTF8Text(pDX, IDC_LPARTSPROP_NAME, mName);
	DDX_CBString(pDX, IDC_LPARTSPROP_TYPE, mType);
	DDX_CBString(pDX, IDC_LPARTSPROP_SUBTYPE, mSubtype);
	DDX_CBIndex(pDX, IDC_LPARTSPROP_ENCODING, mEncoding);
	DDX_UTF8Text(pDX, IDC_LPARTSPROP_DESCRIPTION, mDescription);
	DDX_CBIndex(pDX, IDC_LPARTSPROP_DISPOSITION, mDisposition);
	DDX_CBIndex(pDX, IDC_LPARTSPROP_CHARSET, mCharset);
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CLetterPartProp message handlers

// Called during startup
BOOL CLetterPartProp::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Add type list
	for(int index = IDS_TYPE_APPLICATION; index <= IDS_TYPE_VIDEO; index++)
	{
		CString str;
		str.LoadString(index);

		// Add and set item
		mTypeCtrl.AddString(str);
	}
	
	// Force subtype list update
	SetSubtypeCombo(mType);
	mSubtypeCtrl.SetWindowText(mSubtype);

	// Add encoding to list
	for(int index = IDS_ENCODING_TEXT; index <= IDS_ENCODING_AD; index++)
	{
		CString str;
		str.LoadString(index);

		// Add and set item
		mEncodingCtrl.AddString(str);
	}
	CString str;
	str.LoadString(IDS_AUTOMATIC);
	mEncodingCtrl.AddString(str);
	mEncodingCtrl.SetCurSel(mEncoding);

	// Add dispositions to list
	for(int index = IDS_DISPOSITION_INLINE; index <= IDS_DISPOSITION_ATTACH; index++)
	{
		CString str;
		str.LoadString(index);

		// Add and set item
		mDispositionCtrl.AddString(str);
	}
	mDispositionCtrl.SetCurSel(mDisposition);

	// Add charset to list
	for(int index = IDS_CHARSET_AUTO; index <= IDS_CHARSET_ISO8858_10; index++)
	{
		CString str;
		str.LoadString(index);

		// Add and set item
		mCharsetCtrl.AddString(str);
	}
	mCharsetCtrl.SetCurSel(mCharset);

	return true;
}

void CLetterPartProp::SetSubtypeCombo(CString& sel)
{
	bool encoding = true;
	bool disposition = true;
	bool charset = false;

	// Look for different types
	mSubtypeCtrl.ResetContent();

	if (sel == cMIMEContentTypes[eContentText])
	{
		for(int index = IDS_SUBTYPE_PLAIN; index <= IDS_SUBTYPE_HTML; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
		
		// Force charset display
		charset = true;
	}
	else if (sel == cMIMEContentTypes[eContentMultipart])
	{
		for(int index = IDS_SUBTYPE_MIXED; index <= IDS_SUBTYPE_ALTERNATIVE; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
		
		// Turn off encoding and disposition
		encoding = false;
		disposition = false;
	}
	else if (sel == cMIMEContentTypes[eContentApplication])
	{
		for(int index = IDS_SUBTYPE_OCTET; index <= IDS_SUBTYPE_APPLEFILE; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
	}
	else if (sel == cMIMEContentTypes[eContentMessage])
	{
		for(int index = IDS_SUBTYPE_RFC822; index <= IDS_SUBTYPE_EXTERNAL; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
		
		// Turn off encoding and disposition
		encoding = false;
		disposition = false;
	}
	else if (sel == cMIMEContentTypes[eContentImage])
	{
		for(int index = IDS_SUBTYPE_GIF; index <= IDS_SUBTYPE_JPEG; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
	}
	else if (sel == cMIMEContentTypes[eContentAudio])
	{
		for(int index = IDS_SUBTYPE_BASIC; index <= IDS_SUBTYPE_BASIC; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
	}
	else if (sel == cMIMEContentTypes[eContentVideo])
	{
		for(int index = IDS_SUBTYPE_MPEG; index <= IDS_SUBTYPE_QT; index++)
		{
			CString str;
			str.LoadString(index);

			// Add and set item
			mSubtypeCtrl.AddString(str);
		}
	}
	
	// Handle encoding display
	mEncodingCtrl.EnableWindow(encoding);

	// Handle disposition display
	mDispositionCtrl.EnableWindow(disposition);
	mHasDisposition = disposition;

	// Handle charset display
	mCharsetTitleCtrl.ShowWindow(charset);
	mCharsetCtrl.ShowWindow(charset);
}

void CLetterPartProp::OnEditChangeType()
{
	// Process new text
	CString str;
	mTypeCtrl.GetWindowText(str);
	SetSubtypeCombo(str);

	// Force to first item
	mSubtypeCtrl.SetCurSel(0);

}

void CLetterPartProp::OnSelChangeType()
{
	// Process text of selected item
	int item = mTypeCtrl.GetCurSel();
	CString str;
	mTypeCtrl.GetLBText(item, str);
	SetSubtypeCombo(str);

	// Force to first item
	mSubtypeCtrl.SetCurSel(0);

}

void CLetterPartProp::SetFields(CMIMEContent& content)
{
	mName = content.GetMappedName();
	mType = content.GetContentTypeText();
	mSubtype = content.GetContentSubtypeText();
	mEncoding = content.GetTransferMode() - 1;
	mDescription = content.GetContentDescription();
	mDisposition = content.GetContentDisposition() ? content.GetContentDisposition() - 1 : 0;
	mCharset = content.GetCharset();
}

void CLetterPartProp::GetFields(CMIMEContent& content)
{
	// Do this first to ensure type/subtype not replaced
	ETransferMode mode = (ETransferMode) (mEncoding + 1);
	content.SetTransferMode(mode);

	content.SetMappedName(mName);
	
	// Only do these if not special transfer mode
	switch(mode)
	{
	case eNoTransferMode:
	case eTextMode:
	case eMIMEMode:
		{
			content.SetContentType(cdstring(mType));
			content.SetContentSubtype(cdstring(mSubtype));
		}
		break;
	default:;
	}
	content.SetContentDescription(mDescription);

	if (mHasDisposition)
		content.SetContentDisposition((EContentDisposition) (mDisposition + 1));

	i18n::ECharsetCode charset = (i18n::ECharsetCode) mCharset;
	content.SetCharset(charset);
}