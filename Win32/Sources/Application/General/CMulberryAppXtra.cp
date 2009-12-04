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


// CMulberryAppXtra.cp

// Special file to reduce number of includes in main file

//#define DEBUG_STARTUP

#include "CMulberryApp.h"

#include "CAdbkManagerDoc.h"
#include "CAdbkManagerFrame.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkSearchDoc.h"
#include "CAdbkSearchFrame.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookDoc.h"
#include "CAddressBookDocTemplate.h"
#include "CAddressBookFrame.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CCalendarStoreDoc.h"
#include "CCalendarStoreFrame.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarDoc.h"
#include "CCalendarFrame.h"
#include "CCalendarWindow.h"
#include "CCopyToMenu.h"
#include "CFontCache.h"
#include "CLetterDoc.h"
#include "CLetterFrame.h"
#include "CLetterHeaderView.h"
#include "CLetterWindow.h"
#include "CLog.h"
#include "CMailboxDoc.h"
#include "CMailboxInfoFrame.h"
#include "CMailboxInfoWindow.h"
#include "CMessageDoc.h"
#include "CMessageFrame.h"
#include "CMessageHeaderView.h"
#include "CMessageWindow.h"
#include "CMulberryWindow.h"
#include "CPeriodicCheck.h"
#include "CPreferences.h"
#include "CPreferencesFile.h"
#include "CRegistration.h"
#include "CRulesDoc.h"
#include "CRulesFrame.h"
#include "CRulesWindow.h"
#include "CSearchDoc.h"
#include "CSearchFrame.h"
#include "CSearchWindow.h"
#include "CServerDoc.h"
#include "CServerFrame.h"
#include "CServerWindow.h"
#include "CSMTPFrame.h"
#include "CSMTPWindow.h"
#include "CSpeechSynthesis.h"
#include "CSplashBox.h"
#include "CStringUtils.h"
#include "CTCPSocket.h"
#include "CUtils.h"
#include "CXStringResources.h"
#include "C3PaneDoc.h"
#include "C3PaneFrame.h"
#include "C3PaneWindow.h"
#include <afxrich.h>

#include "CICalendarManager.h"

#include <strstream.h>

#ifndef _AFX_NO_OCC_SUPPORT
//#include "occimpl.h"
#endif

#ifdef __TODO
extern "C" void _SetupArgs();
#endif
//extern char sExceptionMulberryVersion[];
char sExceptionMulberryVersion[32] = {0};
extern char sCrashLogDir[1024];

AFX_DATADEF PROCESS_LOCAL(_AFX_RICHEDIT_STATE, _afxRichEditState)

// Initialization
BOOL CMulberryApp::InitInstance()
{
	// Get application directory and set as default
	{
		TCHAR tappl_dir[MAX_PATH];
		::GetModuleFileName(AfxGetInstanceHandle(), tappl_dir, MAX_PATH);
		cdstring appl_dir(tappl_dir);
		char* p = ::strrchr(appl_dir.c_str_mod(), '\\');
		if (p)
			*p = 0;
		::SetCurrentDirectory(appl_dir.win_str());

		// Ughh! This is horrible but we must reset CConnectionManager's application CWD
		// as it will be wrong
		const_cast<cdstring&>(CConnectionManager::sConnectionManager.GetApplicationCWD()) = appl_dir;
		const_cast<cdstring&>(CConnectionManager::sConnectionManager.GetApplicationCWD()) += os_dir_delim;
		rsrc::CXStringResources::sStringResources.FullReset();
		
#ifdef DEBUG_STARTUP
		::AfxMessageBox(appl_dir);
		::AfxMessageBox(CConnectionManager::sConnectionManager.GetApplicationCWD());
#endif
	}

	// Init preferences here as they depend on static initialisers	
	CPreferences::sPrefs = new CPreferences;

	// Init the version number in the exception handler
	::strncpy(sExceptionMulberryVersion, ::GetVersionText(GetVersionNumber()), 32);

	// Setup crash dump log path
	::strcpy(sCrashLogDir, CConnectionManager::sConnectionManager.GetApplicationCWD().c_str());

	if (AnotherInstance())
		return false;

	// common initialization
	WNDCLASS wndcls;
	memset(&wndcls, 0, sizeof(WNDCLASS));   // start with NULL defaults
	wndcls.lpfnWndProc = DefWindowProc;
	wndcls.hInstance = AfxGetInstanceHandle();
	wndcls.hCursor = afxData.hcurArrow;
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpszClassName = _T("mulberry_LTable");
	AfxRegisterClass(&wndcls);

#ifdef DEBUG_STARTUP
	::AfxMessageBox(GetCommandLine());
#endif

	// Initialize OLE 2.0 libraries
	if (!AfxOleInit())
	{
		::AfxMessageBox(_T("Could not load OLE 2.0 libraries"));
		return false;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	// Call this when using MFC in a shared DLL
	Enable3dControls();

	// First look for IE controls
#if 0
	{
		HINSTANCE hdll = ::LoadLibrary(_T("COMCTL32.DLL"));
		if (hdll)
		{
			// Use proc ptr instead of direct reference to proc to prevent dll link errors
			// when dll version is wrong
			typedef BOOL WINAPI (*InitCommonControlsExProcPtr)(LPINITCOMMONCONTROLSEX);
			InitCommonControlsExProcPtr proc = NULL;

			if ((proc = (InitCommonControlsExProcPtr) ::GetProcAddress(hdll, "InitCommonControlsEx")) != NULL)
			{
				INITCOMMONCONTROLSEX init;
				init.dwSize = sizeof(INITCOMMONCONTROLSEX);
				init.dwICC = ICC_DATE_CLASSES;
				//sCommonControlsEx = InitCommonControlsEx(&init);
				sCommonControlsEx = (*proc)(&init);
			}

			::FreeLibrary(hdll);
		}
	}
#endif

	// Load RichEdit library
	AfxInitRichEdit2();

	// Load localised resources
	rsrc::CXStringResources::sStringResources.LoadAllStrings();

	// Add speech menus if available
	CSpeechSynthesis::InstallSpeechSynthesis();

	// Specify company name
	SetRegistryKey(_T("Cyrusoft International, Inc."));

	// Must have doc manager to allow DDE processing on startup
	if (m_pDocManager == NULL)
		m_pDocManager = new CDocManager;

	// Determine name of system font
	CFont* sysfont = CFont::FromHandle((HFONT) ::GetStockObject(DEFAULT_GUI_FONT));
	LOGFONT lf;
	sysfont->GetLogFont(&lf);

	// Get standard font
	sAppFont = new CFont;
	sAppFont->CreatePointFont(80, lf.lfFaceName);
	LOGFONT info;
	sAppFont->GetLogFont(&info);

	info.lfWeight = FW_BOLD;
	sAppFontBold = new CFont;
	sAppFontBold->CreateFontIndirect(&info);

	// See if Tahoma is present and use that
	CFontPopup::InitFontList();
	bool has_tahoma = CFontPopup::FontExists("Tahoma");
	
	sAppSmallFont = new CFont;
	sAppSmallFont->CreatePointFont(70, has_tahoma ? _T("Tahoma") : _T("Small Fonts"));

	// Read default values
	ReadDefaults();
	//return false;

	// Do splash screen
	long dlu = ::GetDialogBaseUnits();
	{
		CSplashBox dlog;
		dlog.Create(((dlu & 0xFFFF0000) > 0x00100000) ? CSplashBox::IDD_LARGE : CSplashBox::IDD_SMALL);
		dlog.mSerial = CRegistration::sRegistration.GetSerialNumber();
		if (dlog.mSerial == "illegal copy")
			dlog.mSerial = "";
		CPreferences::sPrefs->vers = GetVersionNumber();
		dlog.mVersion = CPreferences::sPrefs->GetVersionText();
		dlog.mLicensee = CRegistration::sRegistration.GetLicensee();
		if (dlog.mLicensee == "illegal copy")
			dlog.mLicensee = "";
		if (CRegistration::sRegistration.GetOrganisation().length())
		{
			dlog.mLicensee += os_endl;
			dlog.mLicensee += CRegistration::sRegistration.GetOrganisation();
		}
		dlog.UpdateData(false);
		dlog.ShowWindow(SW_SHOW);
		dlog.RedrawWindow();

		// Wait for three seconds
		clock_t ticker = ::clock();

		while((::clock() - ticker) / CLOCKS_PER_SEC < 3) ;

		// Must destroy manually
		dlog.DestroyWindow();
	}

	// create main MDI Frame window
	CMulberryWindow* pMainFrame = new CMulberryWindow;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return false;
	pMainFrame->ShowWindow(SW_SHOW);
	m_pMainWnd = pMainFrame;

	// Make sure this window is registered for Winsock callbacks
	CTCPSocket::TCPSetAsyncHandler(pMainFrame->GetSafeHwnd());

	// Create Calendar manager
	new iCal::CICalendarManager;
	iCal::CICalendarManager::sICalendarManager->InitManager();

	// Enable file manager drag/drop and DDE Execute open
	m_pMainWnd->DragAcceptFiles();

	EnableShellOpen();
	RegisterClipboard();

	// Do protection first
	if (!Protect()) return false;

	// When turned on this is used to force an unhandled exception for testing
#if 0
	const char* killer = NULL;
	while(*killer)
		killer++;
#endif

	// Start logging
	CLog::StartLogging();

	// Create & start periosidc checking
	sMailCheck = new CPeriodicCheck;

	// Do open up action
	OpenUp(true);

	CFontPopup::LoadFontMenu();

	BOOL result = true;

	try
	{
		bool got_prefs_file = false;
		bool got_url = false;

		// Look for prefs file on command line
#ifdef __TODO
		_SetupArgs();
#endif
		CCommandLineInfo cmdLine;
		ParseCommandLine(cmdLine);
		cdstring cmd_line_file(cmdLine.m_strFileName);
#ifdef DEBUG_STARTUP
		::AfxMessageBox(cmd_line_file);
#endif

		// check for URL
		if (::strncmp(cmd_line_file, "mailto:", 7) == 0)
			got_url = true;


		// File name could be quoted
		cmd_line_file.unquote();
		got_prefs_file = !got_url && (cmdLine.m_nShellCommand == CCommandLineInfo::FileOpen) &&
							(::strcmpnocase(&cmd_line_file[::strlen(cmd_line_file) - 4], cPrefFileExtension) == 0);
		if (got_prefs_file)
		{
#ifdef DEBUG_STARTUP
			::AfxMessageBox("got prefs file");
			::AfxMessageBox(cmd_line_file);
#endif
			OpenDocumentFile(cmd_line_file.win_str());
		}

		// Only do this if not DDE
		else if (cmdLine.m_nShellCommand != CCommandLineInfo::FileDDE)
		{
			// Start up without docs
			StartUp();

			// Process URL
#ifdef DEBUG_STARTUP
			::AfxMessageBox("got_url processing");
#endif
			if (got_url)
			{
				// Only do it if there's no pending DDE
				if (sPostponeDDE.empty())
					ProcessURL(cmd_line_file);
				else
					sPostponeDDE = cdstring::null_str;
			}
			// Carry on with doc processing if not FileNew
			else if (cmdLine.m_nShellCommand != CCommandLineInfo::FileNew)
				ProcessShellCommand(cmdLine);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

#ifdef DEBUG_STARTUP
		::AfxMessageBox("throw during InitInstance");
#endif
		result = false;

		// Must close down application first then main window
		// as status bar may need to be active during logouts etc
		CloseDown();
		m_pMainWnd->DestroyWindow();
		delete m_pMainWnd;
	}

	return result;
}

const TCHAR* cNestedCallParam = _T("NestedCall");

class CMyCommandLineInfo : public CCommandLineInfo
{
public:
	bool mNestedCall;
	CMyCommandLineInfo() { mNestedCall = false; }
	virtual ~CMyCommandLineInfo() {};

	virtual void ParseParam(const TCHAR* pszParam,BOOL bFlag,BOOL bLast);
};

void CMyCommandLineInfo::ParseParam(const TCHAR* pszParam,BOOL bFlag,BOOL bLast)
{
	if (bFlag && ::lstrcmp(pszParam, cNestedCallParam) == 0)
		mNestedCall = true;
	else
		CCommandLineInfo::ParseParam(pszParam, bFlag, bLast);
}

HANDLE instance_lock = NULL;
const TCHAR* instance_id = _T("MulberrySingleInstance");

// Check for another instance
BOOL CMulberryApp::AnotherInstance()
{
	HANDLE instance_check = ::OpenMutex(MUTEX_ALL_ACCESS, false, instance_id);

	if (instance_check)
	{
		// Send current command line as DDE to existing app
#ifdef _TODO
		_SetupArgs();
#endif
		CMyCommandLineInfo cmdLine;
		ParseCommandLine(cmdLine);

		// Check for nested call and exit
		if (cmdLine.mNestedCall)
		{
			::AfxMessageBox(_T("Recursive Launch"));
			return true;
		}

		// Look at command line data
		cdstring cmd_line_file(cmdLine.m_strFileName);
		if (cmd_line_file.empty())
			cmd_line_file = ::GetCommandLine();
#ifdef DEBUG_STARTUP
		::AfxMessageBox(GetCommandLine());
		::AfxMessageBox(cmd_line_file);
#endif

		cdstring temp = cmd_line_file;
		if (::strchr(temp.c_str(), ' '))
		{
			ostrstream out;
			const char* p = temp.c_str();
			while(*p)
			{
				switch(*p)
				{
				case ' ':
					out << "%20";
					p++;
					break;
				default:
					out << *p++;
					break;
				}
			}
			out << ends;
			temp = out.str();
		}

		// Add nested call marker
		temp += " -";
		temp += cNestedCallParam;

#ifdef DEBUG_STARTUP
		::AfxMessageBox(temp);
#endif

		TCHAR dir[MAX_PATH];
		if (::GetCurrentDirectory(MAX_PATH, dir))
		{
			HINSTANCE hresult = ::ShellExecute(NULL, _T("open"), temp.win_str(), NULL, dir, SW_SHOWNORMAL);
#ifdef DEBUG_STARTUP
			if ((int) hresult <= 32)
			{
				char strtemp[256];
				::sprintf(strtemp, "Failed ShellExecute result = %d", hresult);
				::AfxMessageBox(strtemp);
			}
#endif
		}
		return true;
	}
	else
	{
		instance_lock = ::CreateMutex(NULL, true, instance_id);
		return false;
	}
}

// Terminate
int CMulberryApp::ExitInstance()
{
	// Clear single istance lock
	if (instance_lock)
		::ReleaseMutex(instance_lock);
	instance_lock = NULL;

	// Stop network and force prefs update
	if (CPreferences::sPrefs)
		OnClose();

	// Delete other globals
	delete sAppFont;
	delete sAppFontBold;
	delete sAppSmallFont;

	// Clear some fonts
	CFontCache::ResetFonts(NULL);

	// Terminate logging
	CLog::StopLogging();

	// Stop periodics
	delete sMailCheck;
	sMailCheck = NULL;

	// Hide task bar icon (if any)
	HideNotification();

	// Remove speech
	CSpeechSynthesis::RemoveSpeechSynthesis();

	// Do default
	return CWinApp::ExitInstance();
}

// Init window state (SDI/MDI)
void CMulberryApp::InitWindowState()
{
	// Only do this once
	static int done_once = false;

	if (done_once)
		return;
	else
		done_once = true;

#ifdef DEBUG_STARTUP
	::AfxMessageBox("InitWindowState");
#endif

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.

	// Depends on MDI/SDI status
	sMultiTaskbar = CPreferences::sPrefs->mMultiTaskbar.GetValue();

	int template_index = 0;
	C3PaneWindow::s3PaneDocTemplate = new CMultiDocTemplate(
		IDR_MAINFRAME,	// Use resources from MDI frame
		RUNTIME_CLASS(C3PaneDoc),
		RUNTIME_CLASS(C3PaneFrameMDI),
		RUNTIME_CLASS(C3PaneWindow));
	AddDocTemplate(C3PaneWindow::s3PaneDocTemplate);
	MyRegisterShellFileType(C3PaneWindow::s3PaneDocTemplate, template_index++, false);

	CServerWindow::sServerDocTemplate = new CMultiDocTemplate(
		IDR_SERVERDOCSDI,
		RUNTIME_CLASS(CServerDoc),
		RUNTIME_CLASS(CServerFrameSDI),
		RUNTIME_CLASS(CServerWindow));
	AddDocTemplate(CServerWindow::sServerDocTemplate);
	MyRegisterShellFileType(CServerWindow::sServerDocTemplate, template_index++, false);

	CMailboxInfoWindow::sMailboxDocTemplate = new CMultiDocTemplate(
		IDR_MAILBOXDOCSDI,
		RUNTIME_CLASS(CMailboxDoc),
		RUNTIME_CLASS(CMailboxInfoFrameSDI),
		RUNTIME_CLASS(CMailboxInfoWindow));
	AddDocTemplate(CMailboxInfoWindow::sMailboxDocTemplate);
	MyRegisterShellFileType(CMailboxInfoWindow::sMailboxDocTemplate, template_index++, false);

	CSMTPWindow::sSMTPDocTemplate = new CMultiDocTemplate(
		IDR_MAILBOXDOCSDI,
		RUNTIME_CLASS(CMailboxDoc),
		RUNTIME_CLASS(CSMTPFrameSDI),
		RUNTIME_CLASS(CSMTPWindow));
	AddDocTemplate(CSMTPWindow::sSMTPDocTemplate);
	MyRegisterShellFileType(CSMTPWindow::sSMTPDocTemplate, template_index++, false);

	CMessageWindow::sMessageDocTemplate = new CMultiDocTemplate(
		IDR_MESSAGEDOCSDI,
		RUNTIME_CLASS(CMessageDoc),
		RUNTIME_CLASS(CMessageFrameSDI),
		RUNTIME_CLASS(CMessageHeaderView));
	AddDocTemplate(CMessageWindow::sMessageDocTemplate);
	MyRegisterShellFileType(CMessageWindow::sMessageDocTemplate, template_index++, false);

	CLetterWindow::sLetterDocTemplate = new CMultiDocTemplate(
		IDR_LETTERDOCSDI,
		RUNTIME_CLASS(CLetterDoc),
		RUNTIME_CLASS(CLetterFrameSDI),
		RUNTIME_CLASS(CLetterHeaderView));
	AddDocTemplate(CLetterWindow::sLetterDocTemplate);
	MyRegisterShellFileType(CLetterWindow::sLetterDocTemplate, template_index++, true);

	CAdbkManagerWindow::sAdbkManagerDocTemplate = new CMultiDocTemplate(
		IDR_ADDRESSBOOKMANAGERDOCSDI,
		RUNTIME_CLASS(CAdbkManagerDoc),
		RUNTIME_CLASS(CAdbkManagerFrameSDI),
		RUNTIME_CLASS(CAdbkManagerWindow));
	AddDocTemplate(CAdbkManagerWindow::sAdbkManagerDocTemplate);
	MyRegisterShellFileType(CAdbkManagerWindow::sAdbkManagerDocTemplate, template_index++, false);

	CAdbkSearchWindow::sAdbkSearchDocTemplate = new CMultiDocTemplate(
		IDR_ADDRESSSEARCHDOCSDI,
		RUNTIME_CLASS(CAdbkSearchDoc),
		RUNTIME_CLASS(CAdbkSearchFrameSDI),
		RUNTIME_CLASS(CAdbkSearchWindow));
	AddDocTemplate(CAdbkSearchWindow::sAdbkSearchDocTemplate);
	MyRegisterShellFileType(CAdbkSearchWindow::sAdbkSearchDocTemplate, template_index++, false);

	CAddressBookWindow::sAddressBookDocTemplate = new CAddressBookDocTemplate(
		IDR_ADDRESSBOOKDOCSDI,
		RUNTIME_CLASS(CAddressBookDoc),
		RUNTIME_CLASS(CAddressBookFrameSDI),
		RUNTIME_CLASS(CAddressBookWindow));
	AddDocTemplate(CAddressBookWindow::sAddressBookDocTemplate);
	MyRegisterShellFileType(CAddressBookWindow::sAddressBookDocTemplate, template_index++, false);

	CCalendarStoreWindow::sCalendarStoreDocTemplate = new CMultiDocTemplate(
		IDR_CALENDARSTOREDOCSDI,
		RUNTIME_CLASS(CCalendarStoreDoc),
		RUNTIME_CLASS(CCalendarStoreFrameSDI),
		RUNTIME_CLASS(CCalendarStoreWindow));
	AddDocTemplate(CCalendarStoreWindow::sCalendarStoreDocTemplate);
	MyRegisterShellFileType(CCalendarStoreWindow::sCalendarStoreDocTemplate, template_index++, false);

	CCalendarWindow::sCalendarDocTemplate = new CMultiDocTemplate(
		IDR_CALENDARDOCSDI,
		RUNTIME_CLASS(CCalendarDoc),
		RUNTIME_CLASS(CCalendarFrameSDI),
		RUNTIME_CLASS(CCalendarWindow));
	AddDocTemplate(CCalendarWindow::sCalendarDocTemplate);
	MyRegisterShellFileType(CCalendarWindow::sCalendarDocTemplate, template_index++, false);

	CSearchWindow::sSearchDocTemplate = new CMultiDocTemplate(
		IDR_SEARCHDOCSDI,
		RUNTIME_CLASS(CSearchDoc),
		RUNTIME_CLASS(CSearchFrameSDI),
		RUNTIME_CLASS(CSearchWindow));
	AddDocTemplate(CSearchWindow::sSearchDocTemplate);
	MyRegisterShellFileType(CSearchWindow::sSearchDocTemplate, template_index++, false);

	CRulesWindow::sRulesDocTemplate = new CMultiDocTemplate(
		IDR_RULESDOCSDI,
		RUNTIME_CLASS(CRulesDoc),
		RUNTIME_CLASS(CRulesFrameSDI),
		RUNTIME_CLASS(CRulesWindow));
	AddDocTemplate(CRulesWindow::sRulesDocTemplate);
	MyRegisterShellFileType(CRulesWindow::sRulesDocTemplate, template_index++, false);

	CDocTemplate* templ = new CMultiDocTemplate(IDR_PREFSDOC, NULL, NULL, NULL);
	AddDocTemplate(templ);
	MyRegisterShellFileType(templ, template_index++, false);

	templ = new CMultiDocTemplate(IDR_MBXDOC, NULL, NULL, NULL);
	AddDocTemplate(templ);
	MyRegisterShellFileType(templ, template_index++, false);

	templ = new CMultiDocTemplate(IDR_MBIDOC, NULL, NULL, NULL);
	AddDocTemplate(templ);
	MyRegisterShellFileType(templ, template_index++, false);

	templ = new CMultiDocTemplate(IDR_MBCDOC, NULL, NULL, NULL);
	AddDocTemplate(templ);
	MyRegisterShellFileType(templ, template_index++, false);

	templ = new CMultiDocTemplate(IDR_MBRDOC, NULL, NULL, NULL);
	AddDocTemplate(templ);
	MyRegisterShellFileType(templ, template_index++, false);

	// Do partial registartion now
	RegisterShellFileTypes(false);

	// Remove Windows menu items
	CMenu* main_menu = AfxGetMainWnd()->GetMenu();
	int wnd_pos = main_menu->GetMenuItemCount() - 2;
	CMenu* wnd_menu = main_menu->GetSubMenu(wnd_pos);
	for(int i = 0; i < 4; i++)
		wnd_menu->RemoveMenu(0, MF_BYPOSITION);
}


static const TCHAR my_szShellOpenFmt[] = _T("%s\\shell\\open\\%s");
static const TCHAR my_szShellPrintFmt[] = _T("%s\\shell\\print\\%s");
static const TCHAR my_szShellPrintToFmt[] = _T("%s\\shell\\printto\\%s");
static const TCHAR my_szDefaultIconFmt[] = _T("%s\\DefaultIcon");
static const TCHAR my_szShellNewFmt[] = _T("%s\\ShellNew");

#define DEFAULT_ICON_INDEX 0

static const TCHAR my_szIconIndexFmt[] = _T(",%d");
static const TCHAR my_szCommand[] = _T("command");
static const TCHAR my_szOpenArg[] = _T(" \"%1\"");
static const TCHAR my_szPrintArg[] = _T(" /p \"%1\"");
static const TCHAR my_szPrintToArg[] = _T(" /pt \"%1\" \"%2\" \"%3\" \"%4\"");
static const TCHAR my_szDDEArg[] = _T(" /dde");

static const TCHAR my_szDDEExec[] = _T("ddeexec");
static const TCHAR my_szDDEOpen[] = _T("[open(\"%1\")]");
static const TCHAR my_szDDEPrint[] = _T("[print(\"%1\")]");
static const TCHAR my_szDDEPrintTo[] = _T("[printto(\"%1\",\"%2\",\"%3\",\"%4\")]");

static const TCHAR my_szShellNewValueName[] = _T("NullFile");
static const TCHAR my_szShellNewValue[] = _T("");

static BOOL AFXAPI MySetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName = NULL);
static BOOL AFXAPI MySetRegKey(LPCTSTR lpszKey, LPCTSTR lpszValue, LPCTSTR lpszValueName)
{
	if (lpszValueName == NULL)
	{
		if (::RegSetValue(HKEY_CLASSES_ROOT, lpszKey, REG_SZ,
			  lpszValue, lstrlen(lpszValue) * sizeof(TCHAR)) != ERROR_SUCCESS)
		{
			TRACE1("Warning: registration database update failed for key '%s'.\n",
				lpszKey);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		HKEY hKey;

		if(::RegCreateKey(HKEY_CLASSES_ROOT, lpszKey, &hKey) == ERROR_SUCCESS)
		{
			LONG lResult = ::RegSetValueEx(hKey, lpszValueName, 0, REG_SZ,
				(CONST BYTE*)lpszValue, (lstrlen(lpszValue) + 1) * sizeof(TCHAR));

			if(::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS)
				return TRUE;
		}
		TRACE1("Warning: registration database update failed for key '%s'.\n", lpszKey);
		return FALSE;
	}
}

void CMulberryApp::MyRegisterShellFileType(CDocTemplate* templ, int nTemplateIndex, bool shell_new)
{
	BOOL bCompat = true;

	//ASSERT(!m_templateList.IsEmpty());  // must have some doc templates

	CString strPathName, strTemp;

	AfxGetModuleShortFileName(AfxGetInstanceHandle(), strPathName);

	CDocTemplate* pTemplate = templ;

	CString strOpenCommandLine = strPathName;
	CString strPrintCommandLine = strPathName;
	CString strPrintToCommandLine = strPathName;
	CString strDefaultIconCommandLine = strPathName;

	if (bCompat)
	{
		CString strIconIndex;
		HICON hIcon = ::ExtractIcon(AfxGetInstanceHandle(), strPathName, nTemplateIndex);
		if (hIcon != NULL)
		{
			strIconIndex.Format(my_szIconIndexFmt, nTemplateIndex);
			DestroyIcon(hIcon);
		}
		else
		{
			strIconIndex.Format(my_szIconIndexFmt, DEFAULT_ICON_INDEX);
		}
		strDefaultIconCommandLine += strIconIndex;
	}

	CString strFilterExt, strFileTypeId, strFileTypeName;
	if (pTemplate->GetDocString(strFileTypeId,
	   CDocTemplate::regFileTypeId) && !strFileTypeId.IsEmpty())
	{
		// enough info to register it
		if (!pTemplate->GetDocString(strFileTypeName,
		   CDocTemplate::regFileTypeName))
			strFileTypeName = strFileTypeId;    // use id name

		ASSERT(strFileTypeId.Find(' ') == -1);  // no spaces allowed

		// first register the type ID of our server
		if (!MySetRegKey(strFileTypeId, strFileTypeName))
			return;       // just skip it

		if (bCompat)
		{
			// path\DefaultIcon = path,1
			strTemp.Format(my_szDefaultIconFmt, (LPCTSTR)strFileTypeId);
			if (!MySetRegKey(strTemp, strDefaultIconCommandLine))
				return;       // just skip it
		}

		// If MDI Application
		if (!pTemplate->GetDocString(strTemp, CDocTemplate::windowTitle) ||
			strTemp.IsEmpty())
		{
			// path\shell\open\ddeexec = [open("%1")]
			strTemp.Format(my_szShellOpenFmt, (LPCTSTR)strFileTypeId,
				(LPCTSTR)my_szDDEExec);
			if (!MySetRegKey(strTemp, my_szDDEOpen))
				return;       // just skip it

			if (bCompat)
			{
				// path\shell\print\ddeexec = [print("%1")]
				strTemp.Format(my_szShellPrintFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)my_szDDEExec);
				if (!MySetRegKey(strTemp, my_szDDEPrint))
					return;       // just skip it

				// path\shell\printto\ddeexec = [printto("%1","%2","%3","%4")]
				strTemp.Format(my_szShellPrintToFmt, (LPCTSTR)strFileTypeId,
					(LPCTSTR)my_szDDEExec);
				if (!MySetRegKey(strTemp, my_szDDEPrintTo))
					return;       // just skip it

				// path\shell\open\command = path /dde
				// path\shell\print\command = path /dde
				// path\shell\printto\command = path /dde
				strOpenCommandLine += my_szDDEArg;
				strPrintCommandLine += my_szDDEArg;
				strPrintToCommandLine += my_szDDEArg;
			}
			else
			{
				strOpenCommandLine += my_szOpenArg;
			}
		}
		else
		{
			// path\shell\open\command = path filename
			// path\shell\print\command = path /p filename
			// path\shell\printto\command = path /pt filename printer driver port
			strOpenCommandLine += my_szOpenArg;
			if (bCompat)
			{
				strPrintCommandLine += my_szPrintArg;
				strPrintToCommandLine += my_szPrintToArg;
			}
		}

		// path\shell\open\command = path filename
		strTemp.Format(my_szShellOpenFmt, (LPCTSTR)strFileTypeId,
			(LPCTSTR)my_szCommand);
		if (!MySetRegKey(strTemp, strOpenCommandLine))
			return;       // just skip it

		if (bCompat)
		{
			// path\shell\print\command = path /p filename
			strTemp.Format(my_szShellPrintFmt, (LPCTSTR)strFileTypeId,
				(LPCTSTR)my_szCommand);
			if (!MySetRegKey(strTemp, strPrintCommandLine))
				return;       // just skip it

			// path\shell\printto\command = path /pt filename printer driver port
			strTemp.Format(my_szShellPrintToFmt, (LPCTSTR)strFileTypeId,
				(LPCTSTR)my_szCommand);
			if (!MySetRegKey(strTemp, strPrintToCommandLine))
				return;       // just skip it
		}

		pTemplate->GetDocString(strFilterExt, CDocTemplate::filterExt);
		if (!strFilterExt.IsEmpty())
		{
			ASSERT(strFilterExt[0] == '.');

			LONG lSize = _MAX_PATH * 2;
			LONG lResult = ::RegQueryValue(HKEY_CLASSES_ROOT, strFilterExt,
				strTemp.GetBuffer(lSize), &lSize);
			strTemp.ReleaseBuffer();

			if (lResult != ERROR_SUCCESS || strTemp.IsEmpty() ||
				strTemp == strFileTypeId)
			{
				// no association for that suffix
				if (!MySetRegKey(strFilterExt, strFileTypeId))
					return;

				if (bCompat && shell_new)
				{
					strTemp.Format(my_szShellNewFmt, (LPCTSTR)strFilterExt);
					(void)MySetRegKey(strTemp, my_szShellNewValue, my_szShellNewValueName);
				}
			}
		}
	}
}
