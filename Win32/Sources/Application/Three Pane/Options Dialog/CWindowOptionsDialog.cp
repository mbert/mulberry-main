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


// Source for CWindowOptionsDialog class

#include "CWindowOptionsDialog.h"

#include "CAdminLock.h"
#include "CMessagePaneOptions.h"
#include "CAddressPaneOptions.h"
#include "CCalendarPaneOptions.h"
#include "CMessageView.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUserActionOptions.h"
#include "C3PaneWindow.h"

/////////////////////////////////////////////////////////////////////////////
// CWindowOptionsDialog dialog

CWindowOptionsDialog::CWindowOptionsDialog(CWnd* pParent /*=NULL*/)
	: CHelpPropertySheet(_T("Window Options:"), pParent)
{
}

// Default destructor
CWindowOptionsDialog::~CWindowOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

BEGIN_MESSAGE_MAP(CWindowOptionsDialog, CHelpPropertySheet)
	//{{AFX_MSG_MAP(CWindowOptionsDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

#if 0
// Specially hacked to prevent sheet != page font problems
void CWindowOptionsDialog::BuildPropPageArray()
{
	// delete existing prop page array
	delete[] (PROPSHEETPAGE*)m_psh.ppsp;
	m_psh.ppsp = NULL;

	// build new prop page array
	AFX_OLDPROPSHEETPAGE* ppsp = new AFX_OLDPROPSHEETPAGE[m_pages.GetSize()];
	m_psh.ppsp = (LPPROPSHEETPAGE)ppsp;
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		memcpy(&ppsp[i], &pPage->m_psp, sizeof(pPage->m_psp));

		LPCDLGTEMPLATE pTemplate = NULL;
		PROPSHEETPAGE& psp = (PROPSHEETPAGE&)ppsp[i];

		if (psp.dwFlags & PSP_DLGINDIRECT)
		{
			pTemplate = psp.pResource;
		}
		else
		{
			HRSRC hResource = ::FindResource(psp.hInstance,
				psp.pszTemplate, RT_DIALOG);
			HGLOBAL hTemplate = LoadResource(psp.hInstance,
				hResource);
			pTemplate = (LPCDLGTEMPLATE)LockResource(hTemplate);
		}

		ASSERT(pTemplate != NULL);

		// WINBUG: Windows currently does not support DIALOGEX resources!
		// Assert that the template is *not* a DIALOGEX template.
		// DIALOGEX templates are not supported by the PropertySheet API.

		// To change a DIALOGEX template back to a DIALOG template,
		// remove the following:
		//  1. Extended styles on the dialog
		//  2. Help IDs on any control in the dialog
		//  3. Control IDs that are DWORDs
		//  4. Weight, italic, or charset attributes on the dialog's font

		ASSERT(((DLGTEMPLATEEX*)pTemplate)->signature != 0xFFFF); // See above!
/*																				<-- The big hack
#ifndef _AFX_NO_OCC_SUPPORT
		// if the dialog could contain OLE controls, deal with them now
		if (afxOccManager != NULL)
			pTemplate = pPage->InitDialogInfo(pTemplate);
#endif

#ifndef _MAC
		// set font of property page to same font used by property sheet
		HGLOBAL hTemplate = _ChangePropPageFont(pTemplate,
			(m_psh.dwFlags & PSH_WIZARD));

		if (pPage->m_hDialogTemplate != NULL)
		{
			GlobalFree(pPage->m_hDialogTemplate);
			pPage->m_hDialogTemplate = NULL;
		}

		if (hTemplate != NULL)
		{
			pTemplate = (LPCDLGTEMPLATE)hTemplate;
			pPage->m_hDialogTemplate = hTemplate;
		}
#endif //!_MAC
*/

		psp.pResource = pTemplate;
		psp.dwFlags |= PSP_DLGINDIRECT;
	}

	m_psh.nPages = m_pages.GetSize();
}
#endif

/////////////////////////////////////////////////////////////////////////////
// CWindowOptionsDialog message handlers

BOOL CWindowOptionsDialog::OnInitDialog()
{
	const short cBtnWidth = 98;
	const short cBtnHeight = 22;
	const short cBtnGap = 30;
	const short cBtnExtraGap = 20;

	BOOL result = CHelpPropertySheet::OnInitDialog();

	// Set flags
	m_psh.dwFlags &= ~PSH_HASHELP;			// Disable help button
	m_psh.dwFlags &= ~PSH_NOAPPLYNOW;		// Disable Apply Now button
	ModifyStyleEx(0, WS_EX_CONTEXTHELP);	// Add context help

	// Get width of OK button
	CWnd* btn = GetDlgItem(IDOK);
	CRect rect;
	btn->GetWindowRect(rect);
	ScreenToClient(rect);
	int offset = rect.Width();

	// Move OK & Cancel
	btn->GetWindowRect(rect);
	ScreenToClient(rect);
	rect.OffsetRect(2*offset, 0);
	btn->MoveWindow(rect);

	btn = GetDlgItem(IDCANCEL);
	btn->GetWindowRect(rect);
	ScreenToClient(rect);
	rect.OffsetRect(2*offset, 0);
	btn->MoveWindow(rect);

	// Hide Apply/Help
	btn = GetDlgItem(ID_APPLY_NOW);
	btn->ShowWindow(SW_HIDE);
	btn = GetDlgItem(IDHELP);
	btn->ShowWindow(SW_HIDE);

	return result;
}

// Set the details
void CWindowOptionsDialog::SetDetails(C3PaneOptions* options, bool is3pane)
{
	mIs3Pane = is3pane;

	// Copy details into local cache of user actions
	for(int i = 0; i < N3Pane::eView_Total; i++)
	{
		for(int j = 0; j < C3PaneOptions::C3PaneViewOptions::eUserAction_Total; j++)
			mUserActions[i][j] = options->GetViewOptions((N3Pane::EViewType) i).GetUserAction((C3PaneOptions::C3PaneViewOptions::EUserAction) j);
	}
	mMailOptions = options->GetMailViewOptions();
	mAddressOptions = options->GetAddressViewOptions();
	mCalendarOptions = options->GetCalendarViewOptions();

	AddPage(&mMessage);
	mMessage.SetData(mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mMailOptions, mIs3Pane);

	AddPage(&mAddress);
	mAddress.SetData(mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mAddressOptions, mIs3Pane);

	// Remove unwanted prefs panels by hiding buttons
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		AddPage(&mCalendar);
		mCalendar.SetData(mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mCalendarOptions, mIs3Pane);
	}
}

// Get the details
void CWindowOptionsDialog::GetDetails(C3PaneOptions* options)
{
	// Force update of data from current panel
	mMessage.GetData(mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Mailbox][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mMailOptions);
	mAddress.GetData(mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eListFullView],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
						mUserActions[N3Pane::eView_Contacts][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
						mAddressOptions);
	// Remove unwanted prefs panels by hiding buttons
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		mCalendar.GetData(mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListPreview],
							mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eListFullView],
							mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsPreview],
							mUserActions[N3Pane::eView_Calendar][C3PaneOptions::C3PaneViewOptions::eItemsFullView],
							mCalendarOptions);
	}

	// Copy local cached data into details
	for(int i = 0; i < N3Pane::eView_Total; i++)
	{
		for(int j = 0; j < C3PaneOptions::C3PaneViewOptions::eUserAction_Total; j++)
			options->GetViewOptions((N3Pane::EViewType) i).GetUserAction((C3PaneOptions::C3PaneViewOptions::EUserAction) j) = mUserActions[i][j];
	}
	
	// Update message views if mail view options have changed
	if (!(options->GetMailViewOptions() == mMailOptions))
	{
		options->GetMailViewOptions() = mMailOptions;

		// Close all message views belonging to server - in reverse
		{
			cdmutexprotect<CMessageView::CMessageViewList>::lock _lock(CMessageView::sMsgViews);
			for(CMessageView::CMessageViewList::iterator iter = CMessageView::sMsgViews->begin();
					iter != CMessageView::sMsgViews->end(); iter++)
			{
				(*iter)->ResetOptions();
			}
		}
		
		// Reset use of substitute
		if (mIs3Pane && C3PaneWindow::s3PaneWindow)
			C3PaneWindow::s3PaneWindow->SetUseSubstitute(mMailOptions.GetUseTabs());
	}

	options->GetAddressViewOptions() = mAddressOptions;
}

bool CWindowOptionsDialog::PoseDialog()
{
	bool result = false;

	// Create the dialog
	CWindowOptionsDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(&CPreferences::sPrefs->Get3PaneOptions().Value(), CPreferences::sPrefs->mUse3Pane.GetValue());

	// Let DialogHandler process events
	if (dlog.DoModal() == msg_OK)
	{
		dlog.GetDetails(&CPreferences::sPrefs->Get3PaneOptions().Value());
		CPreferences::sPrefs->Get3PaneOptions().SetDirty();
		result = true;
	}

	return result;
}
