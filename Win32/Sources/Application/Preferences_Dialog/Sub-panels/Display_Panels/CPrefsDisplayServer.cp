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


// CPrefsDisplayServer.cpp : implementation file
//

#include "CPrefsDisplayServer.h"

#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayServer dialog

IMPLEMENT_DYNAMIC(CPrefsDisplayServer, CTabPanel)

CPrefsDisplayServer::CPrefsDisplayServer()
	: CTabPanel(CPrefsDisplayServer::IDD)
{
	//{{AFX_DATA_INIT(CPrefsDisplayServer)
	mOpenBold = FALSE;
	mClosedBold = FALSE;
	mOpenItalic = FALSE;
	mClosedItalic = FALSE;
	mOpenStrike = FALSE;
	mClosedStrike = FALSE;
	mOpenUnderline = FALSE;
	mClosedUnderline = FALSE;
	mUseServer = FALSE;
	mUseFavourite = FALSE;
	mUseHierarchy = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsDisplayServer::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsDisplayServer)
	DDX_Check(pDX, IDC_SERVER_LOGGEDIN_BOLD, mOpenBold);
	DDX_Check(pDX, IDC_SERVER_LOGGEDOUT_BOLD, mClosedBold);
	DDX_Check(pDX, IDC_SERVER_LOGGEDIN_ITALIC, mOpenItalic);
	DDX_Check(pDX, IDC_SERVER_LOGGEDOUT_ITALIC, mClosedItalic);
	DDX_Check(pDX, IDC_SERVER_LOGGEDIN_STRIKE, mOpenStrike);
	DDX_Check(pDX, IDC_SERVER_LOGGEDOUT_STRIKE, mClosedStrike);
	DDX_Check(pDX, IDC_SERVER_LOGGEDIN_UNDER, mOpenUnderline);
	DDX_Check(pDX, IDC_SERVER_LOGGEDOUT_UNDER, mClosedUnderline);
	DDX_Check(pDX, IDC_SERVER_USESERVER, mUseServer);
	DDX_Check(pDX, IDC_SERVER_USEFAVOURITE, mUseFavourite);
	DDX_Check(pDX, IDC_SERVER_USEHIERARCHY, mUseHierarchy);
	DDX_Control(pDX, IDC_SERVER_USESERVER, mUseServerBtn);
	DDX_Control(pDX, IDC_SERVER_USEFAVOURITE, mUseFavouriteBtn);
	DDX_Control(pDX, IDC_SERVER_USEHIERARCHY, mUseHierarchyBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsDisplayServer, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsDisplayServer)
	ON_BN_CLICKED(IDC_SERVER_USESERVER, OnUseServer)
	ON_BN_CLICKED(IDC_SERVER_USEFAVOURITE, OnUseFavourite)
	ON_BN_CLICKED(IDC_SERVER_USEHIERARCHY, OnUseHierarchy)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayServer message handlers

BOOL CPrefsDisplayServer::OnInitDialog() 
{
	CTabPanel::OnInitDialog();
	
	// Subclass buttons
	mOpenColour.SubclassDlgItem(IDC_SERVER_LOGGEDIN_COLOUR, this);
	mClosedColour.SubclassDlgItem(IDC_SERVER_LOGGEDOUT_COLOUR, this);
	mServerColour.SubclassDlgItem(IDC_SERVER_SERVER_COLOUR, this);
	mFavouriteColour.SubclassDlgItem(IDC_SERVER_CABINET_COLOUR, this);
	mHierarchyColour.SubclassDlgItem(IDC_SERVER_HIER_COLOUR, this);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set data
void CPrefsDisplayServer::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	mOpenColour.SetColor(copyPrefs->mServerOpenStyle.GetValue().color);
	mClosedColour.SetColor(copyPrefs->mServerClosedStyle.GetValue().color);
	
	mOpenBold = (copyPrefs->mServerOpenStyle.GetValue().style & bold) ? 1 : 0;
	mClosedBold = (copyPrefs->mServerClosedStyle.GetValue().style & bold) ? 1 : 0;
	
	mOpenItalic = (copyPrefs->mServerOpenStyle.GetValue().style & italic) ? 1 : 0;
	mClosedItalic = (copyPrefs->mServerClosedStyle.GetValue().style & italic) ? 1 : 0;
	
	mOpenStrike = (copyPrefs->mServerOpenStyle.GetValue().style & strike_through) ? 1 : 0;
	mClosedStrike = (copyPrefs->mServerClosedStyle.GetValue().style & strike_through) ? 1 : 0;
	
	mOpenUnderline = (copyPrefs->mServerOpenStyle.GetValue().style & underline) ? 1 : 0;
	mClosedUnderline = (copyPrefs->mServerClosedStyle.GetValue().style & underline) ? 1 : 0;

	mServerColour.SetColor(copyPrefs->mServerBkgndStyle.GetValue().color);
	mUseServer = copyPrefs->mServerBkgndStyle.GetValue().style;
	if (!mUseServer)
		mServerColour.EnableWindow(false);
	mFavouriteColour.SetColor(copyPrefs->mFavouriteBkgndStyle.GetValue().color);
	mUseFavourite = copyPrefs->mFavouriteBkgndStyle.GetValue().style;
	if (!mUseFavourite)
		mFavouriteColour.EnableWindow(false);
	mHierarchyColour.SetColor(copyPrefs->mHierarchyBkgndStyle.GetValue().color);
	mUseHierarchy = copyPrefs->mHierarchyBkgndStyle.GetValue().style;
	if (!mUseHierarchy)
		mHierarchyColour.EnableWindow(false);
}

// Force update of data
bool CPrefsDisplayServer::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Get values
	SStyleTraits mtraits;
	mtraits.color = mOpenColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mOpenBold ? bold : 0);
	mtraits.style |= (mOpenItalic ? italic : 0);
	mtraits.style |= (mOpenStrike ? strike_through : 0);
	mtraits.style |= (mOpenUnderline ? underline : 0);
	copyPrefs->mServerOpenStyle.SetValue(mtraits);

	mtraits.color = mClosedColour.GetColor();
	mtraits.style = 0;
	mtraits.style |= (mClosedBold ? bold : 0);
	mtraits.style |= (mClosedItalic ? italic : 0);
	mtraits.style |= (mClosedStrike ? strike_through : 0);
	mtraits.style |= (mClosedUnderline ? underline : 0);
	copyPrefs->mServerClosedStyle.SetValue(mtraits);

	mtraits.color = mServerColour.GetColor();
	mtraits.style = mUseServer;
	copyPrefs->mServerBkgndStyle.SetValue(mtraits);

	mtraits.color = mFavouriteColour.GetColor();
	mtraits.style = mUseFavourite;
	copyPrefs->mFavouriteBkgndStyle.SetValue(mtraits);

	mtraits.color = mHierarchyColour.GetColor();
	mtraits.style = mUseHierarchy;
	copyPrefs->mHierarchyBkgndStyle.SetValue(mtraits);

	return true;
}

void CPrefsDisplayServer::OnUseServer()
{
	mServerColour.EnableWindow(mUseServerBtn.GetCheck());
}

void CPrefsDisplayServer::OnUseFavourite()
{
	mFavouriteColour.EnableWindow(mUseFavouriteBtn.GetCheck());
}

void CPrefsDisplayServer::OnUseHierarchy()
{
	mHierarchyColour.EnableWindow(mUseHierarchyBtn.GetCheck());
}
