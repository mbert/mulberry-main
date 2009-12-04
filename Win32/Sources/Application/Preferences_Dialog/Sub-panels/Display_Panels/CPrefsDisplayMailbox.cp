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


// CPrefsDisplayMailbox.cpp : implementation file
//

#include "CPrefsDisplayMailbox.h"

#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayMailbox dialog

IMPLEMENT_DYNAMIC(CPrefsDisplayMailbox, CTabPanel)

CPrefsDisplayMailbox::CPrefsDisplayMailbox()
	: CTabPanel(CPrefsDisplayMailbox::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplayMailbox)
	mRecentBold = FALSE;
	mUnseenBold = FALSE;
	mOpenBold = FALSE;
	mFavouriteBold = FALSE;
	mClosedBold = FALSE;
	mRecentItalic = FALSE;
	mUnseenItalic = FALSE;
	mOpenItalic = FALSE;
	mFavouriteItalic = FALSE;
	mClosedItalic = FALSE;
	mRecentStrike = FALSE;
	mUnseenStrike = FALSE;
	mOpenStrike = FALSE;
	mFavouriteStrike = FALSE;
	mClosedStrike = FALSE;
	mRecentUnderline = FALSE;
	mUnseenUnderline = FALSE;
	mOpenUnderline = FALSE;
	mFavouriteUnderline = FALSE;
	mClosedUnderline = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsDisplayMailbox::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplayMailbox)
	DDX_Check(pDX, IDC_MBOX_RECENTBOLD, mRecentBold);
	DDX_Check(pDX, IDC_MBOX_UNSEENBOLD, mUnseenBold);
	DDX_Check(pDX, IDC_MBOX_OPENBOLD, mOpenBold);
	DDX_Check(pDX, IDC_MBOX_FAVBOLD, mFavouriteBold);
	DDX_Check(pDX, IDC_MBOX_CLOSEDBOLD, mClosedBold);
	DDX_Check(pDX, IDC_MBOX_RECENTITALIC, mRecentItalic);
	DDX_Check(pDX, IDC_MBOX_UNSEENITALIC, mUnseenItalic);
	DDX_Check(pDX, IDC_MBOX_OPENITALIC, mOpenItalic);
	DDX_Check(pDX, IDC_MBOX_FAVITALIC, mFavouriteItalic);
	DDX_Check(pDX, IDC_MBOX_CLOSEDITALIC, mClosedItalic);
	DDX_Check(pDX, IDC_MBOX_RECENTSTRIKE, mRecentStrike);
	DDX_Check(pDX, IDC_MBOX_UNSEENSTRIKE, mUnseenStrike);
	DDX_Check(pDX, IDC_MBOX_OPENSTRIKE, mOpenStrike);
	DDX_Check(pDX, IDC_MBOX_FAVSTRIKE, mFavouriteStrike);
	DDX_Check(pDX, IDC_MBOX_CLOSEDSTRIKE, mClosedStrike);
	DDX_Check(pDX, IDC_MBOX_RECENTUNDER, mRecentUnderline);
	DDX_Check(pDX, IDC_MBOX_UNSEENUNDER, mUnseenUnderline);
	DDX_Check(pDX, IDC_MBOX_OPENUNDER, mOpenUnderline);
	DDX_Check(pDX, IDC_MBOX_FAVUNDER, mFavouriteUnderline);
	DDX_Check(pDX, IDC_MBOX_CLOSEDUNDER, mClosedUnderline);
	DDX_Check(pDX, IDC_MBOX_IGNORERECENT, mIgnoreRecent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplayMailbox, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsDisplayMailbox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayMailbox message handlers

BOOL CPrefsDisplayMailbox::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	mClosedColour.SubclassDlgItem(IDC_MBOX_CLOSEDCOLOUR, this);
	mFavouriteColour.SubclassDlgItem(IDC_MBOX_FAVCOLOUR, this);
	mOpenColour.SubclassDlgItem(IDC_MBOX_OPENCOLOUR, this);
	mUnseenColour.SubclassDlgItem(IDC_MBOX_UNSEENCOLOUR, this);
	mRecentColour.SubclassDlgItem(IDC_MBOX_RECENTCOLOUR, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsDisplayMailbox::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	mRecentColour.SetColor(copyPrefs->mMboxRecentStyle.GetValue().color);
	mUnseenColour.SetColor(copyPrefs->mMboxUnseenStyle.GetValue().color);
	mOpenColour.SetColor(copyPrefs->mMboxOpenStyle.GetValue().color);
	mFavouriteColour.SetColor(copyPrefs->mMboxFavouriteStyle.GetValue().color);
	mClosedColour.SetColor(copyPrefs->mMboxClosedStyle.GetValue().color);

	mRecentBold = (copyPrefs->mMboxRecentStyle.GetValue().style & bold) ? 1 : 0;
	mUnseenBold = (copyPrefs->mMboxUnseenStyle.GetValue().style & bold) ? 1 : 0;
	mOpenBold = (copyPrefs->mMboxOpenStyle.GetValue().style & bold) ? 1 : 0;
	mFavouriteBold = (copyPrefs->mMboxFavouriteStyle.GetValue().style & bold) ? 1 : 0;
	mClosedBold = (copyPrefs->mMboxClosedStyle.GetValue().style & bold) ? 1 : 0;

	mRecentItalic = (copyPrefs->mMboxRecentStyle.GetValue().style & italic) ? 1 : 0;
	mUnseenItalic = (copyPrefs->mMboxUnseenStyle.GetValue().style & italic) ? 1 : 0;
	mOpenItalic = (copyPrefs->mMboxOpenStyle.GetValue().style & italic) ? 1 : 0;
	mFavouriteItalic = (copyPrefs->mMboxFavouriteStyle.GetValue().style & italic) ? 1 : 0;
	mClosedItalic = (copyPrefs->mMboxClosedStyle.GetValue().style & italic) ? 1 : 0;

	mRecentStrike = (copyPrefs->mMboxRecentStyle.GetValue().style & strike_through) ? 1 : 0;
	mUnseenStrike = (copyPrefs->mMboxUnseenStyle.GetValue().style & strike_through) ? 1 : 0;
	mOpenStrike = (copyPrefs->mMboxOpenStyle.GetValue().style & strike_through) ? 1 : 0;
	mFavouriteStrike = (copyPrefs->mMboxFavouriteStyle.GetValue().style & strike_through) ? 1 : 0;
	mClosedStrike = (copyPrefs->mMboxClosedStyle.GetValue().style & strike_through) ? 1 : 0;

	mRecentUnderline = (copyPrefs->mMboxRecentStyle.GetValue().style & underline) ? 1 : 0;
	mUnseenUnderline = (copyPrefs->mMboxUnseenStyle.GetValue().style & underline) ? 1 : 0;
	mOpenUnderline = (copyPrefs->mMboxOpenStyle.GetValue().style & underline) ? 1 : 0;
	mFavouriteUnderline = (copyPrefs->mMboxFavouriteStyle.GetValue().style & underline) ? 1 : 0;
	mClosedUnderline = (copyPrefs->mMboxClosedStyle.GetValue().style & underline) ? 1 : 0;

	mIgnoreRecent = copyPrefs->mIgnoreRecent.GetValue();
}

// Force update of data
bool CPrefsDisplayMailbox::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	SStyleTraits mtraits;
	mtraits.color = mRecentColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mRecentBold ? bold : 0);
	mtraits.style |= (mRecentItalic ? italic : 0);
	mtraits.style |= (mRecentStrike ? strike_through : 0);
	mtraits.style |= (mRecentUnderline ? underline : 0);
	copyPrefs->mMboxRecentStyle.SetValue(mtraits);

	mtraits.color = mUnseenColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mUnseenBold ? bold : 0);
	mtraits.style |= (mUnseenItalic ? italic : 0);
	mtraits.style |= (mUnseenStrike ? strike_through : 0);
	mtraits.style |= (mUnseenUnderline ? underline : 0);
	copyPrefs->mMboxUnseenStyle.SetValue(mtraits);

	mtraits.color = mOpenColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mOpenBold ? bold : 0);
	mtraits.style |= (mOpenItalic ? italic : 0);
	mtraits.style |= (mOpenStrike ? strike_through : 0);
	mtraits.style |= (mOpenUnderline ? underline : 0);
	copyPrefs->mMboxOpenStyle.SetValue(mtraits);

	mtraits.color = mFavouriteColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mFavouriteBold ? bold : 0);
	mtraits.style |= (mFavouriteItalic ? italic : 0);
	mtraits.style |= (mFavouriteStrike ? strike_through : 0);
	mtraits.style |= (mFavouriteUnderline ? underline : 0);
	copyPrefs->mMboxFavouriteStyle.SetValue(mtraits);

	mtraits.color = mClosedColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mClosedBold ? bold : 0);
	mtraits.style |= (mClosedItalic ? italic : 0);
	mtraits.style |= (mClosedStrike ? strike_through : 0);
	mtraits.style |= (mClosedUnderline ? underline : 0);
	copyPrefs->mMboxClosedStyle.SetValue(mtraits);

	copyPrefs->mIgnoreRecent.SetValue(mIgnoreRecent);
	
	return true;
}
