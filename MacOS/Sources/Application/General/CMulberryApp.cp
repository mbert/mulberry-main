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


// Source for CMulberryApp class


#define MAKE_APP			1

//#define MEMDEBUG			1

#ifdef MEMDEBUG
#include "memdebug.h"
#endif

#include "CMulberryApp.h"

#include "CAboutDialog.h"
#include "CAboutPluginsDialog.h"
#include "CActionManager.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CApplyRulesMenu.h"
#include "CAttachmentManager.h"
#include "CBalloonDialog.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarWindow.h"
#include "CChangePswdAcctDialog.h"
#include "CClickElement.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CDisconnectDialog.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CHelpMenuAttachment.h"
#include "CICSupport.h"
#include "CLetterWindow.h"
#include "CMacroEditDialog.h"
#include "CMailAccountManager.h"
#include "CMulberryAE.h"
#include "CMulberryCocoaApp.h"
#include "COptionsProtocol.h"
#include "CPeriodicCheck.h"
#include "CPluginManager.h"
#include "CPostponeGURL.h"
#include "CPreferencesDialog.h"
#include "CPreferencesFile.h"
#include "CRemotePrefsSets.h"
#include "CResources.h"
#include "CRulesWindow.h"
#include "CSearchWindow.h"
#include "CServerWindow.h"
#include "CSMTPAccountManager.h"
#include "CSMTPWindow.h"
#include "CSoundManager.h"
#include "CStatusWindow.h"
#include "CStringUtils.h"
#include "CTCPSocket.h"
#include "CToolbarView.h"
#include "CTooltip.h"
#include "CWindowsMenu.h"
#include "CWindowOptionsDialog.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include "CICalendarManager.h"

#include "CCalendarStoreManager.h"

#include <LMouseTracker.h>
#include <LThread.h>

#include <UStandardDialogs.h>
#include <UThread.h>

//#define DATA_VIEWER
#ifdef DATA_VIEWER
	#include <DataViewer.h>		// Custom data viewers for source debugger
#endif

#undef CR
#include <MachineExceptions.h>

#include <signal.h>

#ifndef __GNUC__
union sigval {
	/* Members as suggested by Annex C of POSIX 1003.1b. */
	int	sigval_int;
	void	*sigval_ptr;
};

typedef struct __siginfo {
	int	si_signo;		/* signal number */
	int	si_errno;		/* errno association */
	int	si_code;		/* signal code */
	int	si_pid;			/* sending process */
	unsigned int si_uid;		/* sender's ruid */
	int	si_status;		/* exit value */
	void	*si_addr;		/* faulting instruction */
	union sigval si_value;		/* signal value */
	long	si_band;		/* band event for SIGPOLL */
	unsigned int	pad[7];		/* Reserved for Future Use */
} siginfo_t;

/* union for signal handlers */
union __sigaction_u {
	void    (*__sa_handler)(int);
	void    (*__sa_sigaction)(int, struct __siginfo *,
		       void *);
};

/* Signal vector template for Kernel user boundary */
struct	__sigaction {
	union __sigaction_u __sigaction_u;  /* signal handler */
	void    (*sa_tramp)(void *, int, int, siginfo_t *, void *);
	sigset_t sa_mask;		/* signal mask to apply */
	int	sa_flags;		/* see signal options below */
};

/*
 * Signal vector "template" used in sigaction call.
 */
struct	sigaction {
	union __sigaction_u __sigaction_u;  /* signal handler */
	sigset_t sa_mask;		/* signal mask to apply */
	int	sa_flags;		/* see signal options below */
};
/* if SA_SIGINFO is set, sa_sigaction is to be used instead of sa_handler. */
#define	sa_handler	__sigaction_u.__sa_handler
#define	sa_sigaction	__sigaction_u.__sa_sigaction
#endif

// === Static Members ===

// ===========================================================================
//		¥ Main Program
// ===========================================================================

static void runapp(void);

int main()
{

#ifdef DATA_VIEWER
	DataViewLibInit();				// View opaque types in source debugger
#endif

									// Set Debugging options
#if MAKE_APP
	SetDebugThrow_(debugAction_Nothing);
#else
#ifdef LEAKS
	SetDebugThrow_(debugAction_Nothing);
#else
	SetDebugThrow_(debugAction_SourceDebugger);
#endif
#endif

#if MAKE_APP
	SetDebugSignal_(debugAction_Nothing);
#else
	SetDebugSignal_(debugAction_SourceDebugger);
#endif

	// Ignoe OS X Mach-o signals
	struct sigaction act;
	act.sa_handler = SIG_IGN;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	::sigaction(SIGPIPE, &act, NULL);

	// Load localised resources
	rsrc::CXStringResources::sStringResources.LoadAllStrings();

	// Init preferences here as they depend on static initialisers	
	CPreferences::sPrefs = new CPreferences;


	InitializeHeap(15);				// Initialize Memory Manager
									// Parameter is number of Master Pointer
									//   blocks to allocate

									// Initialize standard Toolbox managers
	UQDGlobals::InitializeToolbox();
	CSoundManager::sSoundManager.Initialize();	// Must do after toolbox init as this may have to init Quicktime

    InitializeCocoa(runapp);
    
#ifdef LEAKS
	DebugNewReportLeaks();
#endif
	
	return 0;
}

static void runapp()
{
	try
	{
		CMulberryApp	theApp;			// Create instance of your Application
#ifdef LEAKS
		DebugNewForgetLeaks();
#endif
		theApp.Run();					//   class and run it
	}
	catch(...)
	{
	}
    return;
}


// ===========================================================================
//		¥ CMulberryApp Class
// ===========================================================================

// Static variables

CMulberryApp*		CMulberryApp::sApp = NULL;					// This application
bool				CMulberryApp::sSuspended = false;			// Indiactes if App in background
COptionsProtocol*	CMulberryApp::sOptionsProtocol = NULL;		// The Options client
CRemotePrefsSet*	CMulberryApp::sRemotePrefs = NULL;			// Set of remote prefs
CPreferencesFile*	CMulberryApp::sCurrentPrefsFile = NULL;		// The current prefs file
CPeriodicCheck*		CMulberryApp::sMailCheck = NULL;			// Mail check
CPeriodicReset*		CMulberryApp::sMailCheckReset = NULL;		// Reset key check
VersRecHndl			CMulberryApp::sVersion1 = NULL;				// App version rsrc 1
VersRecHndl			CMulberryApp::sVersion2 = NULL;				// App version rsrc 2
ProcessSerialNumber	CMulberryApp::sPSN;							// Serial number of this process
ProcessInfoRec		CMulberryApp::sPInfo;						// Process info
FSSpec				CMulberryApp::sPSpec;						// Process file spec

// Sleep handler
static SleepQRec sSleep;	// Sleep queue record
static bool sSleepQInstalled = false;
static NMRec sWakeNotification;
static bool sWakeNotificationPending = false;
static bool sWakePending = false;
static Handle sWakeIconSuite = NULL;

pascal long MySleepProc(long message, SleepQRecPtr qRecPtr);
pascal long MySleepProc(long message, SleepQRecPtr qRecPtr)
{
	// Must have app
	if (CMulberryApp::sApp == NULL)
		return 1;
	
	switch(message)
	{
	case kSleepRequest:
	case kDozeRequest:
	case kSuspendRequest:
		// Simply give positive response to the request. We will handle the actual sleep
		// when the demand arrives.
		return 1;
	case kSleepDemand:
	case kDozeDemand:
	case kSuspendDemand:
		CMulberryApp::sApp->GoToSleep(true);
		return 1;
	case kSleepWakeUp:
	case kDozeWakeUp:
	case kSuspendWakeUp:
		CMulberryApp::sApp->WakeFromSleep();
		return 1;
	}
	
	return 1;
}

// Exception handler
#if 0
static ExceptionHandlerTPP sPreviousExHandler = NULL;

OSStatus MyExceptionHandler(ExceptionInformation *theException);
OSStatus MyExceptionHandler(ExceptionInformation *theException)
{
	// Remove the sleep queue item when we crash
	if (sSleepQInstalled)
	{
		::SleepQRemove(&sSleep);
		DisposeSleepQUPP(sSleep.sleepQProc);
		sSleepQInstalled = false;
	}
	
	if (sPreviousExHandler)
		return InvokeExceptionHandlerUPP(theException, sPreviousExHandler);
	else
		return -1;	
}
#endif

// ---------------------------------------------------------------------------
//		¥ CMulberryApp
// ---------------------------------------------------------------------------
//	Constructor

#pragma push
#pragma segment Init
CMulberryApp::CMulberryApp()
{
	// Install exception handler here
#if 0
	sPreviousExHandler = ::InstallExceptionHandler(NewExceptionHandlerUPP(MyExceptionHandler));
#endif

	mDoneProtect = false;
	mFailedProtect = false;
	mStartInit = false;
	mPrefsLoaded = false;
	mMUPrefsUsed = false;
	mRunDemo = false;
	mPaused = false;
	mSleeping = false;
	mTrackMouse = false;
	mHelpMenu = NULL;
	CLog::StartLogging();

	// Create the main thread.
	new UMainThread;

	// Add a yield attachment.
	AddAttachment(new LYieldAttachment(-1));

	// Add mouse tracker periodic
	LMouseTracker* track = new LMouseTracker;
	track->StartIdling();

	// Install sleep queue
	sSleep.sleepQLink = NULL;
	sSleep.sleepQType = sleepQType;
	sSleep.sleepQProc = NewSleepQUPP(MySleepProc);
	sSleep.sleepQFlags = 0;
	::SleepQInstall(&sSleep);
	sSleepQInstalled = true;

	if (sWakeIconSuite == NULL)
	{
		if (GetIconSuite(&sWakeIconSuite, 128, svAllSmallData) != noErr)
			sWakeIconSuite = NULL;
	}

#ifdef MEMDEBUG
	start_leak_check();
#endif
		
}
#pragma pop


// ---------------------------------------------------------------------------
//		¥ ~CMulberryApp
// ---------------------------------------------------------------------------
//	Destructor

CMulberryApp::~CMulberryApp()
{
	// Do standard close action
	CloseDown();

	// Stop InternetConfig
	CICSupport::ICStop();

	delete LMenuBar::GetCurrentMenuBar();

	CPluginManager::sPluginManager.CleanPlugins();

	// Remove sleep queue
	::SleepQRemove(&sSleep);
	DisposeSleepQUPP(sSleep.sleepQProc);
	sSleepQInstalled = false;

#ifdef MEMDEBUG
	report_leaks("CMulberryApp::~CMulberryApp()");
#endif
}

// Put protection alert if required
void CMulberryApp::DoQuit(SInt32 inSaveOption)
{
	// Check for held messages in SMTP queue while still connected
	if (CConnectionManager::sConnectionManager.IsConnected()&&
		CSMTPAccountManager::sSMTPAccountManager &&
		CSMTPAccountManager::sSMTPAccountManager->PendingItems())
	{
		// Ask user
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::ReallyQuitSMTPHeldItems") == CErrorHandler::Cancel)
			return;
	}

	// Try to clear out temp attachments
	cdstrvect still_open;
	if (!CAttachmentManager::sAttachmentManager.CanQuit(still_open))
	{
		// Create one string with all attachment names
		cdstring names;
		for(cdstrvect::const_iterator iter = still_open.begin(); iter != still_open.end(); iter++)
		{
			if (names.length())
				names += ", ";
			names += *iter;
		}

		// Display alert with list of open files
		if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::General::ViewAttachmentsStillOpen", names) == CErrorHandler::Cancel)
			return;
	}

	if (mFailedProtect)
	{
		// Force quit if over time limit
		//CErrorHandler::PutStopAlertRsrc("Alerts::General::IllegalCopy");
	}

	// Look for locked quit
	else if (CAdminLock::sAdminLock.mNoQuit)
	{
		// Check for option key
		UInt32 modifiers = ::GetCurrentKeyModifiers();
		bool option_key = modifiers & optionKey;
		bool shift_key = modifiers & shiftKey;
		bool cmd_key = modifiers & cmdKey;

		// See if allowed to quit by existing docs
		if ((!option_key || !shift_key || !cmd_key) && AttemptQuit(inSaveOption))
		{
			// First close it
			CloseDown();

			// Now reopen it
			OpenUp(false);

			// Now do initial startup, but allow quit to proceed
			//CAdminLock::sAdminLock.mNoQuit = false;
			StartUp();
			//CAdminLock::sAdminLock.mNoQuit = true;

			// Done
			return;
		}
	}

	CBalloonApp::DoQuit(inSaveOption);
}

// Get version
VersRecHndl CMulberryApp::GetVersion(short versID)
{
	return (versID == 1) ? sVersion1 : sVersion2;
}

// Get version
NumVersion CMulberryApp::GetVersionNumber(short versID)
{
	if (versID == 1)
	{
		if (sVersion1)
			return (**sVersion1).numericVersion;
	}
	else
	{
		if (sVersion2)
			return (**sVersion2).numericVersion;
	}

	NumVersion dummy = {0, 0, 0, 0};
	return dummy;
}

void CMulberryApp::EventResume(const EventRecord& inMacEvent)
{
	sSuspended = false;
	LApplication::EventResume(inMacEvent);
	
	// Do wake actions if pending
	if (sWakePending)
	{
		if (sWakeNotificationPending)
			::NMRemove(&sWakeNotification);
		WakeFromSleepAction();
		sWakeNotificationPending = false;
		sWakePending = false;
	}
}

void CMulberryApp::EventSuspend(const EventRecord& inMacEvent)
{
	sSuspended = true;
	LApplication::EventSuspend(inMacEvent);
}

void CMulberryApp::AdjustCursor(const EventRecord&	inMacEvent)
{
	// Special cursor if tracking
	if (mTrackMouse)
		::SetThemeCursor(kThemeOpenHandCursor);
	else
		CBalloonApp::AdjustCursor(inMacEvent);
}

// Computer is being put to sleep
bool CMulberryApp::GoToSleep(bool force)
{
	// Not if already in sleep state
	if (mSleeping)
		return true;

	// Log it
	if (force)
		CLOG_LOGCATCH(Sleep Demand);
	else
		CLOG_LOGCATCH(Sleep Request);

	// Tell it to suspend checks/sending etc
	CConnectionManager::sConnectionManager.Suspend();
	
	// Turn off periodic processing
	if (sMailCheck)
		sMailCheck->Suspend();

	mSleeping = true;
	return true;
}

// Computer woken from sleep
void CMulberryApp::WakeFromSleep()
{
	// Not if already out of sleep state
	if (!mSleeping)
		return;

	// Log it
	CLOG_LOGCATCH(Wake Request);

	// Check whether app is currently active
#if 0
	if (!CMulberryApp::sSuspended)
#endif
	{
		// Do immediate wake actions
		if (sMailCheck)
			sMailCheck->PwrSignal();
		else
			WakeFromSleepAction();
	}
#if 0
	else
	{
		// Mark pending wake
		sWakePending = true;

		// Only do notification when there are active TCP connections
		if (CTCPSocket::CountConnected())
		{
			// Post notification alert and process when next active
			sWakeNotificationPending = true;

			sWakeNotification.qType = nmType;					// set queue type
			sWakeNotification.nmMark = true;					// put mark in Application menu
			sWakeNotification.nmIcon = sWakeIconSuite;			// alternating icon
			sWakeNotification.nmSound = (Handle)-1;				// play system alert sound if requested
			sWakeNotification.nmStr = NULL;						// do not display alert box
			sWakeNotification.nmResp = NULL;					// no response procedure
			sWakeNotification.nmRefCon = 0;						// not needed

			OSErr err = ::NMInstall(&sWakeNotification);		// Install it
		}
	}
#endif
}

// Computer woken from sleep
void CMulberryApp::WakeFromSleepAction()
{
	// Turn onperiodic processing - this will check the connection state etc
	if (sMailCheck)
		sMailCheck->Resume();
	
	// Tell it to resume checks/sending etc
	CConnectionManager::sConnectionManager.Resume();
	
	mSleeping = false;
}

// Initialise connection
void CMulberryApp::InitConnection(CPreferences& prefs)
{
	// Force connection if locked
	if (CAdminLock::sAdminLock.mNoDisconnect)
	{
		// Now set disconnected state
		prefs.mDisconnected.SetValue(false);
		CConnectionManager::sConnectionManager.SetConnected(true);
	}
	else
	{
		// Check for disconnect request via keyboard

		// Look for option key
		bool cmd_key = ::GetCurrentKeyModifiers() & cmdKey;
		
		// Possible disconnect prompt
		if (!prefs.mDisconnected.GetValue() && prefs.mPromptDisconnected.GetValue() && CTCPSocket::WillDial() ||
			cmd_key)
		{
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::Disconnected",
																			"ErrorDialog::Btn::Connected",
																			NULL,
																			NULL,
																			"ErrorDialog::Text::PromptDisconnect");
			prefs.mDisconnected.SetValue(result == CErrorDialog::eBtn1);
		}

		// Force connection now
		if (!prefs.mDisconnected.GetValue())
			prefs.mDisconnected.SetValue(!CConnectionManager::sConnectionManager.DoConnection(true));

		// Now set disconnected state
		CConnectionManager::sConnectionManager.SetConnected(!prefs.mDisconnected.GetValue());
	}
}

// Init after prefs set
void CMulberryApp::DoPostPrefsStartup()
{
	try
	{
		// Set flag to indicate one prefs read in
		mPrefsLoaded = true;

		// Start up InternetConfig
		//if (!CICSupport::ICInstalled())
			// Warn user about no IC
		//	CErrorHandler::PutNoteAlertRsrc("Alerts::General::NoInternetConfig");

		// Pause postponed GURL as periodical may execute during this startup sequence, but we don't want it to
		CPostponeGURL::Pause(true);

		// Check map control panel
		{
			MachineLocation loc;
			::ReadLocation(&loc);
			if ((loc.latitude == 0) &&
				(loc.longitude == 0))
				// Warn user about incorrect map
				CErrorHandler::PutNoteAlertRsrc("Alerts::General::NoSetMap");
		}

		// Check for default mail/webcal client
		CheckDefaultMailClient();
		CheckDefaultWebcalClient();

		// Possible disconnect prompt
		InitConnection(*CPreferences::sPrefs);

		// Start address books
		StartAddressBooks();

		// Start network mail ops
		StartNetworkMail();

		// Start calendars
		StartCalendars();

		// Must do initial address book open at start for 3-pane after starting mail
		CActionManager::Open3PaneAddressBook();

		// Unpause postponed GURL
		CPostponeGURL::Pause(false);

		// Recover temporary file
		CLetterDoc::ReadTemporary();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Failure during start up - quit
		DoQuit();
	}

} // CMulberryApp::DoPostPrefsStartup

// Start the network mail controllers
void CMulberryApp::StartNetworkMail()
{
	// Only if not already there
	if (CStatusWindow::sStatusWindow)
		CStatusWindow::sStatusWindow->ResetState();

	// Do mail accounts
	if (!CMailAccountManager::sMailAccountManager)
	{
		// Start IMAP accounts
		new CMailAccountManager;
	}
	else
	{
		// Always sync accounts
		CMailAccountManager::sMailAccountManager->UpdateFavourites();
	}

	// Do SMTP accounts
	bool smtp_init_required = false;
	if (!CSMTPAccountManager::sSMTPAccountManager)
	{
		// Start SMTP accounts
		new CSMTPAccountManager;
		CSMTPAccountManager::sSMTPAccountManager->SyncSMTPSenders(CPreferences::sPrefs->mSMTPAccounts.GetValue());
		smtp_init_required = true;
	}
	else
	{
		// Always sync accounts
		CSMTPAccountManager::sSMTPAccountManager->SyncSMTPSenders(CPreferences::sPrefs->mSMTPAccounts.GetValue());
	}

	// Always do these AFTER main objects have been created
	CMailAccountManager::sMailAccountManager->SyncAccounts(CPreferences::sPrefs->mMailAccounts.GetValue());
	if (smtp_init_required)
	{
		
		// Check to see whether there are messages available and force queue window on screen
		// Check default window state for matching
		if (!CAdminLock::sAdminLock.mNoSMTPQueues && CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected() &&
			(CSMTPAccountManager::sSMTPAccountManager->ItemsHeld() ||
			 (CPreferences::sPrefs->mSMTPWindowDefault.Value().GetViewMode() == NMbox::eViewMode_ShowMatch)))
		{
			CSMTPWindow::OpenSMTPWindow();
			if (CMailAccountManager::sMailAccountManager->GetMainView()->GetServerWindow())
				FRAMEWORK_WINDOW_TO_TOP(CMailAccountManager::sMailAccountManager->GetMainView()->GetServerWindow())
		}
	}

} // CMulberryApp::StartNetworkMail

// Stop the network mail controllers
void CMulberryApp::StopNetworkMail(bool quitting)
{
	// Must kill menu first to prevent unwanted updates (only if really quitting)
	if (quitting)
	{
		delete CCopyToMenu::sMailboxMainMenu;
		CCopyToMenu::sMailboxMainMenu = NULL;
		delete CApplyRulesMenu::sApplyRules;
		CApplyRulesMenu::sApplyRules = NULL;
	}

	// Check whether SMTP queue is still running
	if (CSMTPAccountManager::sSMTPAccountManager)
	{
		if (!CSMTPAccountManager::sSMTPAccountManager->TestHaltQueues())
		{
			CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																			"ErrorDialog::Btn::Wait",
																			"ErrorDialog::Btn::Postpone",
																			NULL,
																			NULL,
																			"ErrorDialog::Text::PromptWaitPostpone");

			// Wait for queues to finish
			CSMTPAccountManager::sSMTPAccountManager->SuspendAll(result == CErrorDialog::eBtn1);
		}
	}

	// Just delete the manager
	delete CSMTPAccountManager::sSMTPAccountManager;
	delete CMailAccountManager::sMailAccountManager;
}

// Start address book management
void CMulberryApp::StartAddressBooks()
{
	// Only do once!
	if (!CAddressBookManager::sAddressBookManager)
	{
		// Create address book manager
		new CAddressBookManager;

		// Force manager to update all accounts
		CAddressBookManager::sAddressBookManager->SyncAccounts();
	}
	else
	{
		// Force manager to update all accounts
		CAddressBookManager::sAddressBookManager->SyncAccounts();
	}
}

// Stop address book managment
void CMulberryApp::StopAddressBooks(bool quitting)
{
	// Just delete the manager & window
	delete CAddressBookManager::sAddressBookManager;
	CAddressBookManager::sAddressBookManager = NULL;
	delete CAdbkSearchWindow::sAdbkSearch;
	CAdbkSearchWindow::sAdbkSearch = NULL;
}

// Start calendar management
void CMulberryApp::StartCalendars()
{
	// Don't allow this if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Only do once!
	if (calstore::CCalendarStoreManager::sCalendarStoreManager == NULL)
	{
		// Now init store
		new calstore::CCalendarStoreManager;
		calstore::CCalendarStoreManager::sCalendarStoreManager->SyncAccounts();
	}
	else
	{
		// Force manager to update all accounts
		calstore::CCalendarStoreManager::sCalendarStoreManager->SyncAccounts();
	}
}

// Stop calendar managment
void CMulberryApp::StopCalendars(bool quitting)
{
	// Don't allow this if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Just delete the manager & window
	delete calstore::CCalendarStoreManager::sCalendarStoreManager;
	calstore::CCalendarStoreManager::sCalendarStoreManager = NULL;
}

// Search for addresses
void CMulberryApp::DoAdbkImportExport()
{
	// Display warning
	CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::WarnImportExport");
}

// Search for addresses
void CMulberryApp::DoSearchAddress()
{
	// Check that search address books exist
	if (!CAddressBookManager::sAddressBookManager->CanSearch())
	{
		// Display warning
		CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::NoSearch");
		return;
	}

	// Create Adbk manager window or bring to front
	if (CAdbkSearchWindow::sAdbkSearch)
		FRAMEWORK_WINDOW_TO_TOP(CAdbkSearchWindow::sAdbkSearch)
	else
	{
		CAdbkSearchWindow* adbksrch = (CAdbkSearchWindow*) LWindow::CreateWindow(paneid_AdbkSearchWindow, this);
		adbksrch->ResetState();
		adbksrch->Show();
	}
}

void CMulberryApp::ClickMenuBar(const EventRecord &inMacEvent)
{
	// Always do update of dynamic menus - this will only be done if they are dirty
	CCopyToMenu::ResetMenuList();
	CApplyRulesMenu::ResetMenuList();

	// Do inherited
	CBalloonApp::ClickMenuBar(inMacEvent);
}

//	Respond to commands
Boolean CMulberryApp::ObeyCommand(CommandT inCommand,void *ioParam)
{
	ResIDT	menuID;
	SInt16	menuItem;

	bool	cmdHandled = true;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Show then select the requested window
		switch (menuID)
		{
		case MENU_ImportAddresses:
		case MENU_ExportAddresses:
			DoAdbkImportExport();
			cmdHandled = true;
			break;
		case MENU_Windows:
			LWindow* aWindow = CWindowsMenu::GetWindow(menuID, menuItem);
			if (aWindow)
				FRAMEWORK_WINDOW_TO_TOP(aWindow)
			cmdHandled = true;
			break;

		default:
			cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
		}
	}
	else
	{
		switch (inCommand)
		{
		case cmd_AboutPlugins:
#ifdef MEMDEBUG
			{
				EventRecord outMacEvent;
				LEventDispatcher::GetCurrentEvent(outMacEvent);

				if (outMacEvent.modifiers & shiftKey)
					start_leak_check();
				else if (outMacEvent.modifiers & optionKey)
					report_leaks("manual report");
				else if (outMacEvent.modifiers & controlKey)
					valid_blocks();
				else
					ShowAboutPluginsBox();
			}
#else
			ShowAboutPluginsBox();
#endif
			break;

		case cmd_EditMacros:
			OnEditMacros();
			break;

		case cmd_HelpDetailedTooltips:
			OnHelpDetailedTooltips();
			break;

		case cmd_HelpMulberryOnline:
			OnHelpMulberryOnline();
			break;

		case cmd_HelpOnlineFAQ:
			OnHelpOnlineFAQ();
			break;

		case cmd_HelpCheckUpdates:
			OnHelpCheckUpdates();
			break;

		case cmd_HelpMulberrySupport:
			OnHelpMulberrySupport();
			break;

		case cmd_HelpBuyMulberry:
			OnHelpBuyMulberry();
			break;

		case cmd_NewLetter:
		case cmd_ToolbarNewLetter:
		case cmd_ToolbarNewLetterOption:
			MakeNewLetterDoc();
			break;

		case cmd_Open:
			// Pass on to server manager window (must exist due to command status)
			CMailAccountManager::sMailAccountManager->GetMainView()->GetTable()->ObeyCommand(inCommand, ioParam);
			break;

		case cmd_OpenLetter:
			ChooseLetterDoc();
			break;

		case cmd_Preferences:
			{
				// Check for option key
				bool option_key = ::GetCurrentKeyModifiers() & optionKey;

				// If option key and logging allowed then do log prefs
				if (option_key && CAdminLock::sAdminLock.mAllowLogging)
				{
					CLog::DoLoggingOptions();
				}
				else
					DoMyPreferences();
			}
			break;

		case cmd_Disconnected:
		case cmd_ToolbarServerConnectBtn:
			DoDisconnected();
			break;

		case cmd_OpenSMTPQueues:
			CSMTPWindow::OpenSMTPWindow();
			break;

		case cmd_ChangePassword:
			DoChangePassword();
			break;

		case cmd_CreateMailbox:
		case cmd_ToolbarCreateMailboxBtn:
			// Pass on to server manager window (must exist due to command status)
			CMailAccountManager::sMailAccountManager->GetMainView()->GetTable()->DoCreateMailbox(false);
			break;

		case cmd_AutoCheckMail:
			DoMailCheck();
			break;

		case cmd_SearchMailbox:
			DoSearch();
			break;

		case cmd_RulesMailbox:
			DoRules();
			break;

		case cmd_AddressBookManager:
			DoAdbkManager();
			break;

		case cmd_NewAddressBook:
			if (CPreferences::sPrefs->mUse3Pane.GetValue())
			{
				// Show three pane window and force to contacts tab
				if (C3PaneWindow::s3PaneWindow)
				{
					FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
					C3PaneWindow::s3PaneWindow->SetViewType(N3Pane::eView_Contacts);
				}
			}
			else
			{
				// Show address book manager
				CAdbkManagerWindow::CreateAdbkManagerWindow();
			}
			break;

		case cmd_OpenAddressBook:
			if (CPreferences::sPrefs->mUse3Pane.GetValue())
			{
				// Show three pane window and force to contacts tab
				if (C3PaneWindow::s3PaneWindow)
				{
					FRAMEWORK_WINDOW_TO_TOP(C3PaneWindow::s3PaneWindow)
					C3PaneWindow::s3PaneWindow->SetViewType(N3Pane::eView_Contacts);
				}
			}
			else
			{
				// Show address book manager
				CAdbkManagerWindow::CreateAdbkManagerWindow();
			}
			break;

		case cmd_SearchAddressBook:
			DoSearchAddress();
			break;

		case cmd_CalendarManager:
			DoCalendarManager();
			break;

		case cmd_SubscribedCalendars:
			DoSubscribedCalendars();
			break;

		case cmd_3Pane:
			DoSet3Pane(true);
			break;

		case cmd_1Pane:
			DoSet3Pane(false);
			break;

		case cmd_ToolbarButtons:
			OnAppToolbarsShow();
			break;

		case cmd_ToolbarSmallIcons:
			OnAppToolbarsSmall();
			break;

		case cmd_ToolbarShowIcons:
			OnAppToolbarsIcon();
			break;

		case cmd_ToolbarShowCaptions:
			OnAppToolbarsCaption();
			break;

		case cmd_ToolbarShowBoth:
			OnAppToolbarsBoth();
			break;

		case cmd_WindowOptions:
			OnWindowOptions();
			break;

		case cmd_Status:
			if (CStatusWindow::sStatusWindow->IsVisible())
				CStatusWindow::sStatusWindow->Hide();
			else
				FRAMEWORK_WINDOW_TO_TOP(CStatusWindow::sStatusWindow)
			cmdHandled = true;
			break;

		default:
			cmdHandled = LDocApplication::ObeyCommand(inCommand, ioParam);
			break;
		}
	}

	return cmdHandled;
}


//	Pass back status of a (menu) command
void CMulberryApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	ResIDT	menuID;
	SInt16	menuItem;

	outUsesMark = false;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Always enable windows menu
		switch (menuID)
		{
		case MENU_ImportAddresses:
		case MENU_ExportAddresses:
			outEnabled = true;
			break;

		case MENU_Windows:
			CWindowsMenu::UpdateMenuList();
			outEnabled = true;
			break;

		default:
			LDocApplication::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		}
	}
	else
	{
		switch (inCommand)
		{
		// This must be handled by others and must be set to false if it gets this far
		// as LDocApplication will enable it
		case cmd_Open:
			// Check for server manager window
			outEnabled = CMailAccountManager::sMailAccountManager &&
							CMailAccountManager::sMailAccountManager->GetMainView();
			break;

		// Always set
		case cmd_AboutPlugins:
			outEnabled = (CPluginManager::sPluginManager.GetPlugins().size() > 0);
			break;

		case cmd_HelpDetailedTooltips:
			outEnabled = true;
			outUsesMark = true;
			outMark = CPreferences::sPrefs->mDetailedTooltips.GetValue() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_EditMacros:
		case cmd_HelpMulberryOnline:
		case cmd_HelpOnlineFAQ:
		case cmd_HelpCheckUpdates:
		case cmd_HelpMulberrySupport:
			outEnabled = true;
			break;

		case cmd_HelpBuyMulberry:
			outEnabled = CMulberryApp::sApp->IsDemo();
			break;

		case cmd_OpenSMTPQueues:
			// Check for queuable SMTP senders
			outEnabled = !CAdminLock::sAdminLock.mNoSMTPQueues &&
							CSMTPAccountManager::sSMTPAccountManager &&
							CSMTPAccountManager::sSMTPAccountManager->CanSendDisconnected();
			break;

		case cmd_NewLetter:
		case cmd_ToolbarNewLetter:
		case cmd_ToolbarNewLetterOption:
		case cmd_Preferences:
			outEnabled = true;
			break;

		case cmd_Disconnected:
		case cmd_ToolbarServerConnectBtn:
			outEnabled = !CAdminLock::sAdminLock.mNoDisconnect;
			outUsesMark = true;
			outMark = noMark;
			if (CConnectionManager::sConnectionManager.IsConnected())
			{
				LStr255 txt(STRx_Standards, str_Disconnect);
				::PLstrcpy(outName, txt);
			}
			else
			{
				LStr255 txt(STRx_Standards, str_Connect);
				::PLstrcpy(outName, txt);
			}
			break;

		case cmd_ChangePassword:
			outEnabled = (CPluginManager::sPluginManager.CountPswdChange() > 0);
			break;

		case cmd_OpenLetter:
			outEnabled = !CAdminLock::sAdminLock.mNoLocalDrafts;
			break;

		case cmd_CreateMailbox:
		case cmd_ToolbarCreateMailboxBtn:
			// Check for server manager window
			outEnabled = CMailAccountManager::sMailAccountManager &&
							CMailAccountManager::sMailAccountManager->GetMainView();
			break;

		case cmd_AutoCheckMail:
		case cmd_SearchMailbox:
			outEnabled = CMailAccountManager::sMailAccountManager &&
							CMailAccountManager::sMailAccountManager->HasOpenProtocols();
			break;

		// Always allow rules edit
		case cmd_RulesMailbox:
			outEnabled = !CAdminLock::sAdminLock.mNoRules;
			break;

		// Always do address book commands
		case cmd_AddressBookManager:
		case cmd_NewAddressBook:
		case cmd_OpenAddressBook:
		case cmd_ImportAddressBook:
		case cmd_ExportAddressBook:
		case cmd_SearchAddressBook:
			outEnabled = true;
			break;

		// Always do calendar commands
		case cmd_CalendarManager:
		case cmd_SubscribedCalendars:
			outEnabled = true;
			break;

		case cmd_Geometry:
		case cmd_WindowOptions:
			// Always enabled
			outEnabled = true;
			break;

		case cmd_3Pane:
			outEnabled = CAdminLock::sAdminLock.mAllow3PaneChoice;
			outUsesMark = true;
			outMark = CPreferences::sPrefs->mUse3Pane.GetValue() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_1Pane:
			outEnabled = CAdminLock::sAdminLock.mAllow3PaneChoice;
			outUsesMark = true;
			outMark = CPreferences::sPrefs->mUse3Pane.GetValue() ? (UInt16)noMark : (UInt16)checkMark;
			break;

		case cmd_Toolbar:
			// Always enabled
			outEnabled = true;
			break;

		case cmd_ToolbarSmallIcons:
			outEnabled = true;
			outUsesMark = true;
			outMark = CPreferences::sPrefs->mToolbarSmallIcons.GetValue() ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_ToolbarShowIcons:
			outEnabled = true;
			outUsesMark = true;
			outMark = (CPreferences::sPrefs->mToolbarShowIcons.GetValue() &&
						!CPreferences::sPrefs->mToolbarShowCaptions.GetValue()) ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_ToolbarShowCaptions:
			outEnabled = true;
			outUsesMark = true;
			outMark = (CPreferences::sPrefs->mToolbarShowCaptions.GetValue() &&
						!CPreferences::sPrefs->mToolbarShowIcons.GetValue()) ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_ToolbarShowBoth:
			outEnabled = true;
			outUsesMark = true;
			outMark = (CPreferences::sPrefs->mToolbarShowCaptions.GetValue() &&
						CPreferences::sPrefs->mToolbarShowIcons.GetValue()) ? (UInt16)checkMark : (UInt16)noMark;
			break;

		case cmd_ToolbarButtons:
			// Always enabled
			{
				outEnabled = true;
				LStr255 txt(STRx_Standards, CPreferences::sPrefs->mToolbarShow.GetValue() ? str_HideToolbar : str_ShowToolbar);
				::PLstrcpy(outName, txt);
			}
			break;

		case cmd_Status:
			outEnabled = (CStatusWindow::sStatusWindow != NULL);
			outUsesMark = true;
			outMark = (CStatusWindow::sStatusWindow && CStatusWindow::sStatusWindow->IsVisible()) ? (UInt16)checkMark : (UInt16)noMark;
			break;

		default:
			LDocApplication::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		}
	}
}

// Special behaviour for help menu
void CMulberryApp::UpdateMenus()
{
	// Do default
	CBalloonApp::UpdateMenus();
	
	// Need to check off the Detailed Tooltips item here
	if (mHelpMenu)
		mHelpMenu->CommandStatus();
}

#pragma mark ____________________________About Items

// Show fancy about box
void CMulberryApp::ShowAboutBox()
{
	// Create the dialog
	CBalloonDialog	theHandler(paneid_AboutDialog, this);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
			break;
	}

}

// Show about plugins box
void CMulberryApp::ShowAboutPluginsBox()
{
	// Create the dialog
	CBalloonDialog	theHandler(paneid_AboutPluginsDialog, this);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
			break;
	}
}

#pragma mark ____________________________Help Menu Items

const char* cUpdatesMajor = "?vmajor=";
const char* cUpdatesMinor = "&vminor=";
const char* cUpdatesBugRel = "&vbugrel=";
const char* cUpdatesStage = "&vstage=";
const char* cUpdatesNonRel = "&vnonrel=";
const char* cMulberrySupportURLInfo = "?subject=Mulberry%20Support%20Request&body=";
const char* cBuyMulberryURL = "http://www.mulberrymail.com";

// Remove unwanted registration items
void CMulberryApp::AdjustHelpMenu()
{
	// Get the menu handle
	MenuHandle menuH = CHelpMenuAttachment::GetHelpMenuHandle();
	if (!menuH)
		return;
	
	// Delete the last two items which will always be the buy item and separator
	short items = ::CountMenuItems(menuH);
	::DeleteMenuItem(menuH, items);
	::DeleteMenuItem(menuH, items - 1);
}

// Go to Mulberry web page
void CMulberryApp::OnHelpDetailedTooltips()
{
	// Toggle tooltip option
	CPreferences::sPrefs->mDetailedTooltips.SetValue(!CPreferences::sPrefs->mDetailedTooltips.GetValue());
	if (!CPreferences::sPrefs->mDetailedTooltips.GetValue())
		CTooltip::HideTooltip();
}

// Go to Mulberry web page
void CMulberryApp::OnHelpMulberryOnline()
{
	// Launch the Mulberry webpage URL
	cdstring url = CAdminLock::sAdminLock.mMulberryURL;
	OSStatus err = CICSupport::ICLaunchURL(url);
}

// Go to Mulberry faq web page
void CMulberryApp::OnHelpOnlineFAQ()
{
	// Launch the Mulberry FAQ webpage URL
	cdstring url = CAdminLock::sAdminLock.mMulberryFAQ;
	OSStatus err = CICSupport::ICLaunchURL(url);
}

// Go to check updates page
void CMulberryApp::OnHelpCheckUpdates()
{
	// Add version info to base URL
	cdstring url = CAdminLock::sAdminLock.mUpdateURL;
	url += cUpdatesMajor;
	url += cdstring(static_cast<unsigned long>(GetVersionNumber().majorRev));
	url += cUpdatesMinor;
	url += cdstring(static_cast<unsigned long>((GetVersionNumber().minorAndBugRev >> 4) & 0x0F));
	url += cUpdatesBugRel;
	url += cdstring(static_cast<unsigned long>(GetVersionNumber().minorAndBugRev & 0x0F));
	url += cUpdatesStage;
	switch (GetVersionNumber().stage)
	{
	case developStage:
		url += "d";
		break;
	case alphaStage:
		url += "a";
		break;
	case betaStage:
		url += "b";
		break;
	case finalStage:
		url += "r";
		break;
	default:
		url += "u";
		break;
	}
	url += cUpdatesNonRel;
	url += cdstring(static_cast<unsigned long>(GetVersionNumber().nonRelRev));

	// Launch the URL
	OSStatus err = CICSupport::ICLaunchURL(url);
}

// Create Mulberry support message
void CMulberryApp::OnHelpMulberrySupport()
{
	// Add product info to base URL
	cdstring url = CAdminLock::sAdminLock.mMulberrySupportAddress;
	url += cMulberrySupportURLInfo;
	
	// Get details about this app
	cdstring details = CPreferences::sPrefs->GetSupportDetails();
	details.EncodeURL();
	url += details;

	ProcessMailto(url);
}

// Go to order form page
void CMulberryApp::OnHelpBuyMulberry()
{
	// Launch the Mulberry order form webpage URL
	cdstring url = cBuyMulberryURL;
	OSStatus err = CICSupport::ICLaunchURL(url);
}

#pragma mark ____________________________Command handling

// Check for new mail
void CMulberryApp::DoMailCheck()
{
	// Check server
	if (CMailAccountManager::sMailAccountManager)
		CMailAccountManager::sMailAccountManager->ForceMailCheck();
}

void CMulberryApp::DoSearch()
{
	// Just create the search window
	CSearchWindow::CreateSearchWindow();
}

void CMulberryApp::DoRules()
{
	// Only if not locked
	if (CAdminLock::sAdminLock.mNoRules)
		return;

	// Just create the rules window
	CRulesWindow::CreateRulesWindow();
}

void CMulberryApp::DoAdbkManager()
{
	// Just create the window
	CAdbkManagerWindow::CreateAdbkManagerWindow();
}

void CMulberryApp::DoCalendarManager()
{
	// Just create the window
	CCalendarStoreWindow::CreateCalendarStoreWindow();
}

void CMulberryApp::DoSubscribedCalendars()
{
	// Just create the window
	CCalendarWindow::CreateSubscribedWindow();
}

// Begin IMAP - do password if required
bool CMulberryApp::BeginINET(CINETProtocol* proto)
{
	bool rmail = (typeid(*proto) == typeid(CMboxProtocol));

	bool canceled = false;

	bool do_open = false;

	if (proto->IsNotOpen())
		do_open = true;

	// Check that servers are the same
	else if (proto->GetDescriptor() != proto->GetAccount()->GetServerIP())
	{
		// Close and mark to re-open
		proto->Close();
		do_open = true;
	}

	// Must open/re-open
	if (do_open)
	{
		bool failed = false;

		try
		{
			proto->SetDescriptor(proto->GetAccount()->GetServerIP());
			proto->Open();

			if (rmail)
			{
				// Server name lookup may have taken place
				// Use canonical name in server window
				CServerWindow* wnd = CMailAccountManager::sMailAccountManager->GetProtocolWindow((CMboxProtocol*) proto);
				if (wnd)
				{
					MyCFString temp(proto->GetDescriptor(), kCFStringEncodingUTF8);
					wnd->SetCFDescriptor(temp);
				}
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Must reload default mailboxes as server's mailbox list now deleted
			if (rmail)
				((CMboxProtocol*) proto)->InitProtocol();

			failed = true;
		}

		// Proto may fail to open if locked by Admin
		if (failed || proto->IsNotOpen())
			return false;
	}

	// Make sure logged on to IMAP server
	cdstring actual_uid;
	if (!proto->IsLoggedOn())
	{

		try
		{
			// Try to logon
			proto->Logon();

			// Recover actual uid
			actual_uid = proto->GetAccount()->GetAuthenticator().GetAuthenticator()->GetActualUID();

			if (rmail)
			{
				// See whether current proto is default or server d already set
				if (CAdminLock::sAdminLock.GetServerID().empty() ||
					(proto->GetAccount() == CPreferences::sPrefs->mMailAccounts.GetValue().front()))
				{
					// Lock return address if required
					if (CAdminLock::sAdminLock.mLockReturnAddress)
					{
						CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
						cdstring real_name = addr.GetName();
						CPreferences::sPrefs->LockReturnAddress(actual_uid, real_name);
					}

					// Set Originator-Info
					CAdminLock::sAdminLock.SetLoginID(actual_uid);
					CAdminLock::sAdminLock.SetServerID(proto->GetAccount()->GetServerIP());
				}
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			if (rmail)
			{
				// See whether current proto is default
				if (proto->GetAccount() == CPreferences::sPrefs->mMailAccounts.GetValue().front())
				{
					// Null Originator-Info
					CAdminLock::sAdminLock.SetLoginID(cdstring::null_str);
					CAdminLock::sAdminLock.SetServerID(cdstring::null_str);
				}
			}
			
			canceled = true;
		}
	}

	// Recover correct MU settings if MU in use and no manual prefs setting
	if (mMUPrefsUsed && !canceled)
	{
		bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
						CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);
		// See whether current proto is default
		if (remote ||
			(proto->GetAccount() == CPreferences::sPrefs->mMailAccounts.GetValue().front()))
		{
			CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
			cdstring real_name = addr.GetName();
			CPreferences::sPrefs->MultiuserConfigure(actual_uid, real_name);

			mMUPrefsUsed = false;
		}
	}

	// Return indicating non-cancel state
	return !canceled;
}

// Begin remote - do password if required
bool CMulberryApp::BeginRemote(CPreferences* prefs)
{
	// Create client & prefs set
	if (!sOptionsProtocol)
		sOptionsProtocol = new COptionsProtocol(prefs->mRemoteAccounts.GetValue().front());

	// Make sure accounts match
	else if (sOptionsProtocol->GetAccountUniqueness() != prefs->mRemoteAccounts.GetValue().front()->GetUniqueness())
	{
		delete sOptionsProtocol;
		sOptionsProtocol = NULL;
		sOptionsProtocol = new COptionsProtocol(prefs->mRemoteAccounts.GetValue().front());
	}

	// Give existing protocol new account
	else
	{
		sOptionsProtocol->SetAccount(prefs->mRemoteAccounts.GetValue().front());
	}
	if (!sRemotePrefs)
		sRemotePrefs = new CRemotePrefsSet;

	return BeginINET(sOptionsProtocol);
}

// Switch diconnected modes
void CMulberryApp::DoDisconnected()
{
	// Check for held messages in SMTP queue when disconnecting
	if (CConnectionManager::sConnectionManager.IsConnected() &&
		CSMTPAccountManager::sSMTPAccountManager &&
		CSMTPAccountManager::sSMTPAccountManager->PendingItems())
	{
		// Ask user
		if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::General::ReallyDisconnectSMTPHeldItems") == CErrorHandler::Cancel)
			return;
	}

	CDisconnectDialog::PoseDialog();
}

// Do preferences dialog
bool CMulberryApp::DoMyPreferences()
{
	bool canceled = false;
	bool new_prefs = false;
	bool rename = false;
	cdstrpairvect renames;

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_PreferencesDialog, this);
		CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) theHandler.GetDialog();
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT	hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Check for valid exit
				if (!prefs_dlog->DoOK())
					continue;

				// Hide it before doing changes
				prefs_dlog->Hide();

				// Tear down active accounts here BEFORE changing prefs
				// Look for complete change in prefs
				bool stopped_mail = false;
				bool stopped_adbk = false;
				bool stopped_cal = false;
				if (prefs_dlog->GetLoadedNewPrefs())
				{
					// Restart mail accounts
					if (CMailAccountManager::sMailAccountManager)
					{
						StopNetworkMail(false);
						stopped_mail = true;
					}

					// Restart addressbook accounts
					if (CAddressBookManager::sAddressBookManager)
					{
						StopAddressBooks(false);
						stopped_adbk = true;
					}

					// Restart calendar accounts
					if (iCal::CICalendarManager::sICalendarManager)
					{
						StopCalendars(false);
						stopped_cal = true;
					}
				}

				// Now force update of prefs
				prefs_dlog->UpdatePrefs();

				// Look for complete change in prefs
				if (prefs_dlog->GetLoadedNewPrefs())
				{
					// Restart mail accounts
					if (stopped_mail)
						StartNetworkMail();

					// Restart addressbook accounts
					if (stopped_adbk)
						StartAddressBooks();

					// Restart calendar accounts
					if (stopped_cal)
						StartCalendars();
				}

				// Sync mail accounts
				if (!stopped_mail && CMailAccountManager::sMailAccountManager)
				{
					// Look for renames
					if (prefs_dlog->GetAccountRename())
					{
						CMailAccountManager::sMailAccountManager->RenameFavourites(prefs_dlog->GetRenames());
						CMailAccountManager::sMailAccountManager->RenameMRUs(prefs_dlog->GetRenames());

						// Not very efficient because this might get done in SyncAccounts
						CCopyToMenu::ResetMenuList();
					}
					CMailAccountManager::sMailAccountManager->SyncAccounts(CPreferences::sPrefs->mMailAccounts.GetValue());
				}

				// Sync SMTPs
				if (!stopped_mail && CSMTPAccountManager::sSMTPAccountManager)
				{
					// Must close SMTP queue window
					CSMTPWindow::CloseSMTPWindow();

					CSMTPAccountManager::sSMTPAccountManager->SyncSMTPSenders(CPreferences::sPrefs->mSMTPAccounts.GetValue());
				}

				// Sync address books
				if (!stopped_adbk && CAddressBookManager::sAddressBookManager)
					CAddressBookManager::sAddressBookManager->SyncAccounts();

				// Sync calendars
				if (!stopped_cal && calstore::CCalendarStoreManager::sCalendarStoreManager)
					calstore::CCalendarStoreManager::sCalendarStoreManager->SyncAccounts();

				// Stop resetting of multi-user prefs when logging in
				mMUPrefsUsed = false;

				canceled = false;

				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				prefs_dlog->DoCancel();

				canceled = true;
				break;
			}
		}
	}

	// Make sure accounts match - might have been changed to do set management etc
	if (sOptionsProtocol)
	{
		// Check for removal of remote prefs account
		if (CPreferences::sPrefs->mRemoteAccounts.GetValue().empty())
		{
			delete sOptionsProtocol;
			sOptionsProtocol = NULL;
		}

		// Make sure accounts match
		else if (sOptionsProtocol->GetAccountUniqueness() != CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetUniqueness())
		{
			delete sOptionsProtocol;
			sOptionsProtocol = NULL;
			sOptionsProtocol = new COptionsProtocol(CPreferences::sPrefs->mRemoteAccounts.GetValue().front());
		}

		// Give existing protocol new account
		else
			sOptionsProtocol->SetAccount(CPreferences::sPrefs->mRemoteAccounts.GetValue().front());
	}

	return canceled;

}


// Do preferences dialog
void CMulberryApp::DoChangePassword()
{
	CChangePswdAcctDialog::AcctPasswordChange();
}

// Edit macros
void CMulberryApp::OnEditMacros()
{
	CMacroEditDialog::PoseDialog();
}

#pragma mark ____________________________3 Pane stuff

void CMulberryApp::OnWindowOptions()
{
	CWindowOptionsDialog::PoseDialog();
}

void CMulberryApp::DoSet3Pane(bool use3pane)
{
	// Check admin lock
	if (!CAdminLock::sAdminLock.mAllow3PaneChoice)
		return;

	// Check for change and ignore if no required
	if (CPreferences::sPrefs->mUse3Pane.GetValue() == use3pane)
		return;
	
	// Try to close - may fail if user cancels
	bool cancel = false;
	try
	{
		CMailAccountManager::sMailAccountManager->GetMainView()->DoClose();
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Do not change the state
		cancel = true;
	}

	// Cancel change if close did not work
	if (cancel)
		return;

	// Do the change
	CPreferences::sPrefs->mUse3Pane.SetValue(use3pane);
	
	if (use3pane)
	{
		// Close down server manager
		CMailAccountManager::sMailAccountManager->SetMainView(NULL);

		// Open server manager
		CMailAccountManager::sMailAccountManager->CreateMainView();

		// Make sure 3-pane is told to re-instate previously open mailboxes/adbks
		if (C3PaneWindow::s3PaneWindow)
		{
			C3PaneWindow::s3PaneWindow->DoneInitMailAccounts();
			C3PaneWindow::s3PaneWindow->DoneInitAdbkAccounts();
			C3PaneWindow::s3PaneWindow->DoneInitCalendarAccounts();
		}
	}
	else
	{
		// Close 3-pane
		C3PaneWindow::Destroy3PaneWindow();
		CMailAccountManager::sMailAccountManager->SetMainView(NULL);

		// Open server manager
		CMailAccountManager::sMailAccountManager->CreateMainView();

		// Create status window
		{
			// Determine OS
			long gestalt_result;
			::Gestalt(gestaltSystemVersion, &gestalt_result);
			bool OS8 = (gestalt_result > 0x07FF);
			CStatusWindow::CreateWindow(OS8 ? paneid_StatusWindow8 : paneid_StatusWindow, this);

			// Set state
			if (CStatusWindow::sStatusWindow)
				CStatusWindow::sStatusWindow->ResetState();
		}
	}
}

void CMulberryApp::OnAppToolbarsShow()
{
	CPreferences::sPrefs->mToolbarShow.SetValue(!CPreferences::sPrefs->mToolbarShow.GetValue());
	CToolbarView::UpdateAllToolbars();
}

void CMulberryApp::OnAppToolbarsSmall()
{
	CPreferences::sPrefs->mToolbarSmallIcons.SetValue(!CPreferences::sPrefs->mToolbarSmallIcons.GetValue());
	CToolbarView::UpdateAllToolbars();
}

void CMulberryApp::OnAppToolbarsIcon()
{
	// Only if different
	if (!CPreferences::sPrefs->mToolbarShowIcons.GetValue() ||
			CPreferences::sPrefs->mToolbarShowCaptions.GetValue())
	{
		CPreferences::sPrefs->mToolbarShowIcons.SetValue(true);
		CPreferences::sPrefs->mToolbarShowCaptions.SetValue(false);
		CToolbarView::UpdateAllToolbars();
	}
}

void CMulberryApp::OnAppToolbarsCaption()
{
	// Only if different
	if (CPreferences::sPrefs->mToolbarShowIcons.GetValue() ||
			!CPreferences::sPrefs->mToolbarShowCaptions.GetValue())
	{
		CPreferences::sPrefs->mToolbarShowIcons.SetValue(false);
		CPreferences::sPrefs->mToolbarShowCaptions.SetValue(true);
		CToolbarView::UpdateAllToolbars();
	}
}

void CMulberryApp::OnAppToolbarsBoth()
{
	// Only if different
	if (!CPreferences::sPrefs->mToolbarShowIcons.GetValue() ||
			!CPreferences::sPrefs->mToolbarShowCaptions.GetValue())
	{
		CPreferences::sPrefs->mToolbarShowIcons.SetValue(true);
		CPreferences::sPrefs->mToolbarShowCaptions.SetValue(true);
		CToolbarView::UpdateAllToolbars();
	}
}

#pragma mark ____________________________General app behaviour

// Open files
void CMulberryApp::OpenDocument(PPx::FSObject* inMacFSSpec)
{
	// Do protection first
	if (!Protect())
	{
		DoQuit();
		return;
	}

	// Get Finder Info
	FinderInfo finfo;
	inMacFSSpec->GetFinderInfo(&finfo, NULL, NULL);

	// Check for prefs file type and only do first time on launch
	switch (finfo.file.fileType)
	{
	case kPrefFileType:
		// Check for first time ( = no current prefs file)
		if (!mPrefsLoaded)
		{
			// Set start flag
			mStartInit = true;

			// Must attempt to load Internet Config first
			if ((CICSupport::ICStart(kApplID) == noErr) && CICSupport::ICFindConfigFile(sPSpec))
				CICSupport::ICStop();

			// Try multi-user first
			COptionsMap* muser_prefs = NULL;
			try
			{
				// Look for multi-user prefs in profile
				muser_prefs = GetMultiuser();

				// Check for multi-user use
				if (muser_prefs)
				{
					// First read in MU prefs and cache 'special' values
					// Must do this before getting admin, as admin may lock out changes to prefs
					ReadMultiuser(muser_prefs);

					// Get admin details from MU prefs
					CAdminLock::sAdminLock.ReadFromMap(muser_prefs);

					delete muser_prefs;
					muser_prefs = NULL;
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Clean up and throw up
				delete muser_prefs;

				CLOG_LOGRETHROW;
				throw;
			}

			// Must reject if local prefs denied
			if (CAdminLock::sAdminLock.mNoLocalPrefs)
				return;

			// Create the prefs file
			sCurrentPrefsFile = new CPreferencesFile(false, false);

			// Set the FSSpec of the prefs file
			sCurrentPrefsFile->SetSpecifier(*inMacFSSpec);
			sCurrentPrefsFile->SetSpecified(true);

			// Read in new prefs
			bool read_ok = sCurrentPrefsFile->VerifyRead(true);

			// Must remove file if not OK
			if (!read_ok)
			{
				delete sCurrentPrefsFile->GetPrefs();
				delete sCurrentPrefsFile;
				sCurrentPrefsFile = NULL;
			}
			else
				// Make it global prefs
				sCurrentPrefsFile->GetPrefs()->SetAsDefault();

			// Verify the prefs read in
			if (!read_ok || !sCurrentPrefsFile->GetPrefs()->Valid(false))

				// Force setting of prefs
				if (!ForcePrefsSetting())
					return;

			// Do remaining init
			DoPostPrefsStartup();
		}
		else
		{
			// Must reject if local prefs denied
			if (CAdminLock::sAdminLock.mNoLocalPrefs)
				return;

			// Ask whether replace is really wanted
			if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Preferences::ReallyReplacePrefs") == CErrorHandler::Cancel)
				return;

			// Create new prefs file
			CPreferencesFile* newPrefsFile = new CPreferencesFile(false, false);

			// Set the FSSpec of the prefs file
			newPrefsFile->SetSpecifier(*inMacFSSpec);
			newPrefsFile->SetSpecified(true);

			// Read in new prefs
			bool read_ok = newPrefsFile->VerifyRead(true);

			// Verify the prefs read in
			if (!read_ok || !newPrefsFile->GetPrefs()->Valid(false))
			{
				// Inform of error and exit without file and new prefs
				if (read_ok)
					CErrorHandler::PutStopAlertRsrc("Alerts::General::InvalidPrefsInFile");
				delete newPrefsFile->GetPrefs();
				delete newPrefsFile;
				return;
			}

			// Make new prefs the default
			newPrefsFile->GetPrefs()->SetAsDefault();

			// Delete existing file object
			delete sCurrentPrefsFile;

			// Replace existing file
			sCurrentPrefsFile = newPrefsFile;

			// Stop any
			DoPostPrefsStartup();
		}
		break;

	case kLetterDocType:
		// Make sure prefs exists - force if not
		if (!mPrefsLoaded) StartUp();

		if (CAdminLock::sAdminLock.mNoLocalDrafts)
		{
			::SysBeep(1);
			break;
		}

		// Only open if not quitting
		if (mState != programState_Quitting)
		{
			// See if its already open
			LDocument*	theDoc = LDocument::FindByFileSpec(*inMacFSSpec);
			if (theDoc)
				theDoc->MakeCurrent();
			else
			{
				CLetterDoc* theLetterDoc = NULL;
				try
				{
					theLetterDoc = new CLetterDoc(this, inMacFSSpec);
					((CLetterWindow*) theLetterDoc->GetWindow())->SetDirty(false);
					theLetterDoc->GetWindow()->Show();
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					delete theLetterDoc;
				}
			}
		}
		break;

	}

} // CMulberryApp::OpenDocument

// Pause periodic actions while error alert pending
void CMulberryApp::ErrorPause(bool pause)
{
	//if (sMboxProtocol)
	//	sMboxProtocol->Pause(pause);

	mPaused = pause;
}

// Create letter doc
void CMulberryApp::MakeNewLetterDoc(CAddressList* to, CAddressList* cc, CAddressList* bcc)
{
	CActionManager::NewDraft(to, cc, bcc);
}

// Choose a letter document
void CMulberryApp::ChooseLetterDoc()
{
	// Send AE for recording
	PPx::FSObject	fspec;
	if (PP_StandardDialogs::AskOpenOneFile(kLetterDocType, fspec, kNavDefaultNavDlogOptions | kNavAllowPreviews | kNavAllFilesInPopup))
		OpenDocument(&fspec);

} // CMulberryApp::ChooseLetterDoc

//	Respond to an AppleEvent
void CMulberryApp::HandleAppleEvent(
	const AppleEvent	&inAppleEvent,
	AppleEvent			&outAEReply,
	AEDesc				&outResult,
	long				inAENumber)
{
	switch (inAENumber)
	{
		case cAEShowPreferences:
			ProcessCommand(cmd_Preferences);
			break;
		case cAEGetURL:
			HandleGetURLEvent(inAppleEvent, outAEReply, inAENumber);
			break;

		case cAECheckMail:
			HandleCheckMailEvent(inAppleEvent, outAEReply, inAENumber);
			break;

		default:
			LDocApplication::HandleAppleEvent(inAppleEvent, outAEReply,
								outResult, inAENumber);
			break;
	}
}
// Handle GetURL AppleEvent
void CMulberryApp::HandleGetURLEvent(
	const AppleEvent	&inAppleEvent,
	AppleEvent&			outAEReply,
	SInt32				inAENumber)
{
	AEDesc		text_desc;
	OSErr		err = ::AEGetParamDesc(&inAppleEvent, keyDirectObject, typeChar, &text_desc);
	ThrowIfOSErr_(err);

#if ACCESSOR_CALLS_ARE_FUNCTIONS
	// Make local copy of text
	AEDesc	coerceDesc = {typeNull, NULL};
	char* txt = NULL;
	if (::AECoerceDesc(&text_desc, typeChar, &coerceDesc) == noErr) {

		long len = ::AEGetDescDataSize(&coerceDesc);
		txt = new char[len + 1];
		err = ::AEGetDescData(&coerceDesc, txt, len);
		txt[len] = 0;

		::AEDisposeDesc(&coerceDesc);

		ThrowIfOSErr_(err);

	} else {						// Coercion failed
		ThrowOSErr_(errAETypeError);
	}
#else
	// Make local copy of text
	long len = ::GetHandleSize(text_desc.dataHandle);
	char* txt;
	{
		StHandleLocker lock(text_desc.dataHandle);
		txt = ::strndup(*text_desc.dataHandle, len);
	}
#endif

	// Look for mailto token (lowercase)
	char* token = ::strtok(txt, " :");
	if (token)
	{
		// Bring to front
		ProcessSerialNumber process;
		::GetCurrentProcess(&process);
		::SetFrontProcess(&process);

		if (::strcmpnocase(token, cMailToURL) == 0)
		{
			// Get remainder of text
			char* addr = ::strtok(NULL, cdstring::null_str);
			char *text = (addr ? ::strdup(addr) : NULL);

			// Only handle if SMTP client exists (ie app is already running - not waiting for prefs dialog)
			if (!CMailAccountManager::sMailAccountManager)
			{
				// Postpone to idle time later on - idler will delete list
				CPostponeGURL* postpone = new CPostponeGURL(text, CPostponeGURL::eMailto);
				postpone->StartIdling();

				// Force startup if not already as GURL event can launch app without normal events
				if (!mStartInit)
					StartUp();
			}
			else
				// Handle GURL now
				ProcessMailto(text);
		}
		else if (::strcmpnocase(token, cWebcalURL) == 0)
		{
			// Get remainder of text
			char* addr = ::strtok(NULL, cdstring::null_str);
			char *text = (addr ? ::strdup(addr) : NULL);

			// Only handle if calendars available
			if (calstore::CCalendarStoreManager::sCalendarStoreManager == NULL)
			{
				// Postpone to idle time later on - idler will delete list
				CPostponeGURL* postpone = new CPostponeGURL(text, CPostponeGURL::eWebcal);
				postpone->StartIdling();

				// Force startup if not already as GURL event can launch app without normal events
				if (!mStartInit)
					StartUp();
			}
			else
				// Handle GURL now
				ProcessWebcal(text);
		}
	}
	delete txt;
	::AEDisposeDesc(&text_desc);
}

// Handle GetURL AppleEvent
void CMulberryApp::HandleCheckMailEvent(
	const AppleEvent	&inAppleEvent,
	AppleEvent&			outAEReply,
	SInt32				inAENumber)
{
	long num_found = 0;

	DoMailCheck();

	::AEPutParamPtr(&outAEReply, keyDirectObject, typeLongInteger, &num_found, sizeof(long));
}

void CMulberryApp::ProcessMailto(const char* mailto)
{
	CLetterWindow* newWindow = NULL;
	cdstring to;
	cdstring cc;
	cdstring bcc;
	cdstring body;
	cdstring subject;
	cdstrvect files;

	CURLClickElement::ParseMailto(mailto, to, cc, bcc, subject, body, files);

	CAddressList toField(to.c_str(), to.length());
	CAddressList ccField(cc.c_str(), cc.length());
	CAddressList bccField(bcc.c_str(), bcc.length());

	CActionManager::NewDraft(&toField, &ccField, &bccField, subject, body, files);
	
	// Do warning of files
	if ((files.size() != 0) && CPreferences::sPrefs->mWarnMailtoFiles.GetValue())
	{
		bool dont_show = false;
		CErrorDialog::NoteAlertRsrc("ErrorDialog::Text::WarnMailtoFiles", NULL, &dont_show);
		if (dont_show)
			CPreferences::sPrefs->mWarnMailtoFiles.SetValue(false);
	}
}

void CMulberryApp::ProcessWebcal(const char* webcal)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		try
		{
			// Prepend http:
			cdstring temp("http:");
			temp += webcal;

			// Check for duplicate
			const calstore::CCalendarStoreNode* cal = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNodeByRemoteURL(temp);

			if (cal != NULL)
				// Error
				CErrorHandler::PutStopAlertRsrcStr("CCalendarStoreTable::DuplicateWebCalendar", temp);
			else
				calstore::CCalendarStoreManager::sCalendarStoreManager->NewWebCalendar(temp);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}
}
