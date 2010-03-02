/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CMulberryApp.h

// Classes
class CINETProtocol;
class COptionsMap;
class COptionsProtocol;
class CPreferences;
class CPreferencesFile;
class CPeriodicCheck;
class CRemotePrefsSet;
class cdstring;

class CMulberryApp : public CWinApp
{
public:
	static CMulberryApp*		sApp;								// This application
	static CFont*				sAppFont;							// Font for controls
	static CFont*				sAppFontBold;						// Font for controls
	static CFont*				sAppSmallFont;						// Font for button titles
	static BOOL					sLargeFont;							// Large font display
	static COptionsProtocol*	sOptionsProtocol;					// The Options client
	static CRemotePrefsSet*		sRemotePrefs;						// Set of remote prefs
	static CPreferencesFile*	sCurrentPrefsFile;					// The current prefs file
	static CPeriodicCheck*		sMailCheck;							// Mail check
	static bool					sMultiTaskbar;						// Use multiple taskbar items
	static bool					sCommonControlsEx;					// Has InitCommonControlsEx
	static cdstring				sPostponeDDE;						// Postponed DDE command appearing at startup

	// Clipboard formats
	static CLIPFORMAT			sFlavorServerList;					// Mail accounts
	static CLIPFORMAT			sFlavorWDList;						// Display hierarchies
	static CLIPFORMAT			sFlavorMboxList;					// Mailbox hierarchies
	static CLIPFORMAT			sFlavorMboxRefList;					// Cabinets

	static CLIPFORMAT			sFlavorMsgList;						// List of messages
	static CLIPFORMAT 			sFlavorAddrList;					// List of addresses
	static CLIPFORMAT 			sFlavorAdbkAddrList;				// List of adbk addresses
	static CLIPFORMAT 			sFlavorGrpList;						// List of groups
	static CLIPFORMAT 			sFlavorAtchList;					// List of outgoing attachments
	static CLIPFORMAT 			sFlavorMsgAtchList;					// List of incoming attachments
	static CLIPFORMAT 			sFlavorRuleType;					// List of rules

	static CLIPFORMAT 			sFlavorCalServer;					// Calendar accounts
	static CLIPFORMAT 			sFlavorCalendar;					// Calendars
	static CLIPFORMAT 			sFlavorCalendarItem;				// Calendars items (e.g. events)

	CMulberryApp();

	virtual BOOL InitInstance();							// Initialisation
	virtual BOOL AnotherInstance();							// Check for another instance
	virtual void RegisterClipboard();						// Register all clipboard formats
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);		// Open special file
	virtual BOOL OnClose();									// Terminate
	virtual int ExitInstance();								// Finalisation
	virtual int DoMessageBox( LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt );

	virtual BOOL AttemptClose();							// Terminate
	virtual void OpenUp(bool first_time);					// Terminate
	virtual void CloseDown();									// Terminate

	// Command related
	afx_msg void		OnUpdateLocalAddr(CCmdUI* pCmdUI);		// Update command
	afx_msg void		OnUpdateLoggedIn(CCmdUI* pCmdUI);		// Update command
	afx_msg void		OnUpdateServerManager(CCmdUI* pCmdUI);	// Update command
	afx_msg void		OnUpdateOpenDraft(CCmdUI* pCmdUI);		// Update command
	afx_msg void 		OnUpdateChangePassword(CCmdUI* pCmdUI);	// Update command
	afx_msg void 		OnUpdateDisconnected(CCmdUI* pCmdUI);	// Update command
	afx_msg void 		OnUpdateOpenSMTPQueue(CCmdUI* pCmdUI);	// Update command
	afx_msg void		OnUpdateAppRules(CCmdUI* pCmdUI);		// Update command
	afx_msg void		OnUpdateAppUse3Pane(CCmdUI* pCmdUI);			// Update command
	afx_msg void		OnUpdateAppUseSeparatePanes(CCmdUI* pCmdUI);	// Update command
	afx_msg void 		OnUpdateAppToolbarsShow(CCmdUI* pCmdUI);
	afx_msg void 		OnUpdateAppToolbarsSmall(CCmdUI* pCmdUI);
	afx_msg void 		OnUpdateAppToolbarsIcon(CCmdUI* pCmdUI);
	afx_msg void 		OnUpdateAppToolbarsCaption(CCmdUI* pCmdUI);
	afx_msg void 		OnUpdateAppToolbarsBoth(CCmdUI* pCmdUI);
	afx_msg void 		OnUpdateHelpDetailedTooltips(CCmdUI* pCmdUI);

	afx_msg void 		OnAppAbout();						// About Box
	afx_msg void 		OnAppAboutPlugins();				// About Plugins Box
	afx_msg void 		OnAppNewDraft();
	afx_msg void 		OnAppOpenMailbox();
	afx_msg void 		OnAppOpenDraft();
	afx_msg void 		OnAppPreferences();					// Preferences Dialog Box
	afx_msg void 		OnAppDisconnected();
	afx_msg void 		OnAppOpenSMTPQueue();
	afx_msg void 		OnFilePageSetup();
	afx_msg void 		OnAppTextMacros();
	afx_msg void 		OnAppChangePassword();
	afx_msg void 		OnAppSpellOptions();
	afx_msg void 		OnAppCreateMailbox();
	afx_msg void 		OnAppMailCheck();
	afx_msg void 		OnAppSearch();
	afx_msg void 		OnAppOpenTarget();
	afx_msg void 		OnAppRules();
	afx_msg void 		OnAppAddressBookManager();
	afx_msg void 		OnAppNewAddressBook();
	afx_msg void 		OnAppOpenAddressBook();
	afx_msg void 		OnAppAddrImportExport(UINT nID);
	afx_msg void 		OnAppSearchAddress();
	afx_msg void 		OnAppSelect3PaneWindow();
	afx_msg void 		OnAppSelectWindow(UINT nID);
	afx_msg void 		OnAppCalendarManager();
	afx_msg void 		OnAppCalendarSubscribed();
	afx_msg void		OnWindowOptions(void);
	afx_msg void 		OnAppUse3Pane();
	afx_msg void 		OnAppUseSeparatePanes();
	afx_msg void 		OnAppToolbarsShow();
	afx_msg void 		OnAppToolbarsSmall();
	afx_msg void 		OnAppToolbarsIcon();
	afx_msg void 		OnAppToolbarsCaption();
	afx_msg void 		OnAppToolbarsBoth();
	afx_msg void		OnHelpDetailedTooltips();
	afx_msg void		OnHelpMulberryOnline();
	afx_msg void		OnHelpOnlineFAQ();
	afx_msg void		OnHelpCheckUpdates();
	afx_msg void		OnHelpMulberrySupport();
	afx_msg void		OnHelpBuyMulberry();

	void		DoSet3Pane(bool use3pane);

	void		ReadDefaults();						// Read in default resources
	static NumVersion	GetVersionNumber();					// Get version number
	time_t		GetDemoTimeout()					// Get demo timeout
							{ return mDemoTimeout; }

	bool		Protect();							// Do copy protection checking
	bool		DoRegistration(bool initial);		// Do registration dialog

	void		PluginInit();						// Initialise plugins
	void		StartUp();							// Startup without prefs
	virtual BOOL		OnDDECommand(LPTSTR lpszCommand);		// Handle DDE command
	void		ProcessURL(const cdstring& url);			// Parse mailto URL
	static void			ProcessMailto(const cdstring& mailto);		// Process mailto URL
	static void			ProcessWebcal(const cdstring& webcal);		// Process webcal URL

	bool		IsDemo()
		{ return mRunDemo; }
	void		RunDemo(bool demo)							// Set run demo flag
		{ mRunDemo = demo; }
	bool		LoadedPrefs() const
		{ return mPrefsLoaded; }
	bool		MUPrefsUsed()							// Set run demo flag
		{ return mMUPrefsUsed; }
	bool		DoDefaultPrefsFile();				// Load prefs file from System Folder
	bool		DoMultiuser(COptionsMap* muser_prefs);	// Do multiuser dialog
	COptionsMap*	GetMultiuser();					// Get multiuser prefs with update
	void		ReadMultiuser(COptionsMap* muser_prefs);// Read in certain fields from multi-user prefs
	bool		ForcePrefsSetting();				// Force setting of prefs, quit if cancel
	void		DoPostPrefsStartup();				// Init after prefs set
	void		InitWindowState();						// Init window state (SDI/MDI)

	void		InitConnection(CPreferences& prefs);

	void		StartNetworkMail();						// Start network mail controllers
	void		StopNetworkMail(bool quitting);			// Stop network mail controllers

	void		StartAddressBooks();					// Start address book management
	void		StopAddressBooks(bool quitting);		// Stop address book managment

	void		StartCalendars();						// Start calendar management
	void		StopCalendars(bool quitting);			// Stop calendar book managment

	// Opening/creating mailboxes
	bool		BeginINET(CINETProtocol* proto);		// Begin protocol
	bool		BeginRemote(CPreferences* prefs);		// Begin protocol

	void		DoMailCheck();						// Check for new mail

	// Other bits
	bool		DoPreferences();					// Do preferences dialog
	void		ErrorPause(bool pause);					// Error displayed
	bool		IsPaused()							// Are periodics paused
							{ return mPaused; }
	bool		IsQuitting()							// Is quitting
							{ return mQuitting; }

	void		CheckDefaultMailClient();			// Check whether its the default client for mailto's
	void		CheckDefaultWebcalClient();			// Check whether its the default client for webcal's
	
	bool		IsMAPIInstalled() const;
	void		InstallMAPI(bool install) const;

	bool		LaunchURL(const cdstring& url);					// Launch this URL

	bool		GetPageMargins(CRect& rect) const
		{ if (mPageSetupDone) { rect = mPageMargins; } return mPageSetupDone; }

	// Notification
	void		ShowNotification();
	void		HideNotification();

	// Sleep
	afx_msg LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
	bool		GoToSleep(bool force);							// Computer is being put to sleep
	void		WakeFromSleep(bool silent = false);				// Computer woken from sleep

private:
	unsigned long		mCRC[5];								// CRC codes
	bool				mDoneProtect;							// Protection test done
	bool				mFailedProtect;							// Protection test
	bool				mStartInit;								// App started
	bool				mPrefsLoaded;							// Has at least one set of prefs been loaded
	bool				mMUPrefsUsed;							// Has an MU prefs been used rather than file
	bool				mRunDemo;								// Running as demo
	bool				mPaused;								// Periodics paused
	bool				mSleeping;								// Computer asleep
	bool				mQuitting;								// App is quitting
	time_t				mDemoTimeout;							// Demo timeout value
	bool				mNotificationOn;						// Notification displayed
	bool				mPageSetupDone;							// Page setup done once => margins are valid
	CRect				mPageMargins;							// Margins from last page setup

	// Message map
	DECLARE_MESSAGE_MAP()

	void		MyRegisterShellFileType(CDocTemplate* templ,	// Special version of RegisterShellFileType
										int nTemplateIndex,
										bool shell_new);
};
