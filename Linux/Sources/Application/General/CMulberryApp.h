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


// CMulberryApp.h
#ifndef _CMULBERRY_APP__
#define _CMULBERRY_APP__

#include <JXApplication.h>
#include "CCommander.h"

#include <JString.h>
#include <JFontStyle.h>
#include "CMainMenu.h"

#include "cdmutex.h"

// Classes
class CINETProtocol;
class COptionsMap;
class COptionsProtocol;
class CPreferences;
class CPreferencesFile;
class CPeriodicCheck;
class CRemotePrefsSet;

class JXDialogDirector;
class cdstring;
class CMulberryMDIServer;

struct HFont
{
	std::string name_;
	JSize  size_;
	JFontStyle style_;

	HFont(const std::string& name, JSize size, const JFontStyle& style)
		: name_(name), size_(size), style_(style) {}
	HFont(const HFont& other) : name_(other.name_), size_(other.size_),
			style_(other.style_) {}
	HFont() : name_("Helvetica"), size_(12), style_(JFontStyle()) {}
};

class CMulberryApp : public CCommander,		// Commander must be first so it gets destroyed last
						public JXApplication
{
public:
	static CMulberryApp*		sApp;			// This application
	static CMulberryMDIServer*	sMDIServer;		// MDI server
	static bool					mQuitting;

	CMulberryApp(int *argc, char* argv[]);
	virtual ~CMulberryApp();

			void CommandLine(const JPtrArray<JString>& argList);
	virtual void Quit();

protected:
	virtual void	ReceiveWithFeedback(JBroadcaster* sender, Message* message);
	virtual void	CleanUpBeforeSuddenDeath(const JXDocumentManager::SafetySaveReason reason);

public:
	static HFont		sAppFont;			// Font for controls
	static HFont		sAppFontBold;		// Font for controls
	static HFont		sAppSmallFont;		// Font for button titles
	static bool			sLargeFont;			// Large font display

	//When a window donesn't want to handle a main menu entry, it should
	//call this.  Returns true if the given selection is handled by CMulberryApp.
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	static COptionsProtocol*	sOptionsProtocol;		// The Options client
	static CRemotePrefsSet*		sRemotePrefs;			// Set of remote prefs
	static CPreferencesFile*	sCurrentPrefsFile;		// The current prefs file
	static CPeriodicCheck*		sMailCheck;       		// Mail check

	// Clipboard formats
	static Atom		sFlavorServerList;	// List of servers
	static Atom		sFlavorWDList;		// List of display hierarchies
	static Atom		sFlavorMboxList;	// List of mailboxes
	static Atom		sFlavorMboxRefList;	// List of mailbox references

	static Atom		sFlavorMsgList;		// List of messages
	static Atom		sFlavorAddrList;	// List of addresses
	static Atom		sFlavorAdbkAddrList;// List of addresses
	static Atom		sFlavorGrpList;		// List of groups
	static Atom		sFlavorAtchList;	// List of outgoing attachments
	static Atom		sFlavorMsgAtchList;	// List of incoming attachments
	static Atom 	sFlavorRuleType;	// List of rules
	static Atom 	sFlavorRulePosType;	// List of rules

	static Atom		sFlavorCalServer;				// Calendar accounts
	static Atom		sFlavorCalendar;				// Calendars
	static Atom		sFlavorCalendarItem;			// Calendars items (e.g. events)

	virtual void RegisterClipboard();      // Register all clipboard formats

	virtual bool OpenPrefs(const cdstring& filename, bool default_prefs = false);

	virtual JBoolean Close();         			// Terminate
	virtual bool AttemptClose();       			// Terminate
	virtual void OpenUp(bool first_time);		// Start
	virtual void CloseDown();					// Terminate
	
			bool CloseAllDocuments();

	// Command related
	void		OnUpdateLocalAddr(CCmdUI* pCmdUI);
	void		OnUpdateLoggedIn(CCmdUI* pCmdUI);
	void		OnUpdateServerManager(CCmdUI* pCmdUI);
	void		OnUpdateOpenDraft(CCmdUI* pCmdUI);
	void 		OnUpdateChangePassword(CCmdUI* pCmdUI);
	void 		OnUpdateDisconnected(CCmdUI* pCmdUI);
	void 		OnUpdateOpenSMTPQueue(CCmdUI* pCmdUI);
	void		OnUpdateAppRules(CCmdUI* pCmdUI);
	void		OnUpdateAppUse3Pane(CCmdUI* pCmdUI);
	void		OnUpdateAppUseSeparatePanes(CCmdUI* pCmdUI);
	void 		OnUpdateAppToolbarsShow(CCmdUI* pCmdUI);
	void 		OnUpdateAppToolbarsSmall(CCmdUI* pCmdUI);
	void 		OnUpdateAppToolbarsIcon(CCmdUI* pCmdUI);
	void 		OnUpdateAppToolbarsCaption(CCmdUI* pCmdUI);
	void 		OnUpdateAppToolbarsBoth(CCmdUI* pCmdUI);
	void 		OnUpdateHelpDetailedTooltips(CCmdUI* pCmdUI);

	void 		OnAppHelp();						// Help
	void 		OnAppAbout();						// About Box
	void 		OnAppAboutPlugins();				// About Plugins Box
	void 		OnAppNewDraft();
	void 		OnAppOpenMailbox();
	void 		OnAppOpenDraft();
	void 		OnAppPreferences();					// Preferences Dialog Box
	void 		OnAppDisconnected();
	void 		OnAppOpenSMTPQueue();
	void 		OnFilePageSetup();
	void 		OnAppTextMacros();
	void 		OnAppChangePassword();
	void 		OnAppCreateMailbox();
	void 		OnAppMailCheck();
	void		OnAppRules();								// Rules
	void 		OnAppSearch();
	void 		OnAppOpenTarget();
	void 		OnAppAddressBookManager();
	void 		OnAppNewAddressBook();
	void 		OnAppOpenAddressBook();
	void		OnAppAddrImportExport();			// Warn adbk import/export
	void 		OnAppSearchAddress();
	void 		OnAppSelect3PaneWindow();
	void 		OnAppSelectWindow(JIndex choice);
	void 		OnAppCalendarManager();
	void 		OnAppCalendarSubscribed();
	void		OnWindowOptions();
	void 		OnAppUse3Pane();
	void 		OnAppUseSeparatePanes();
	void 		OnAppToolbarsShow();
	void 		OnAppToolbarsSmall();
	void 		OnAppToolbarsIcon();
	void 		OnAppToolbarsCaption();
	void 		OnAppToolbarsBoth();
	void		OnHelpDetailedTooltips();
	void		OnHelpMulberryOnline();
	void		OnHelpOnlineFAQ();
	void		OnHelpCheckUpdates();
	void		OnHelpMulberrySupport();
	void		OnHelpBuyMulberry();

	void		DoSet3Pane(bool use3pane);

	void		ReadDefaults();						// Read in default resources
	static NumVersion	GetVersionNumber();					// Get version number
	static const JCharacter* GetAppSignature();

	time_t		GetDemoTimeout()					// Get demo timeout
		{ return mDemoTimeout; }

	bool		Protect();							// Do copy protection checking
	bool		DoRegistration(bool initial);		// Do registration dialog

	void		PluginInit();						// Initialise plugins
	void		StartUp();							// Startup without prefs

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

	bool	DoMultiuser(COptionsMap* muser_prefs); // Do multiuser dialog
	COptionsMap* GetMultiuser();					// Get multiuser prefs with update
	void	ReadMultiuser(COptionsMap* muser_prefs);// Read in certain fields from multi-user prefs

	bool	ForcePrefsSetting();					// Force setting of prefs, quit if cancel
	void	DoPostPrefsStartup();					// Init after prefs set

	void  InitConnection(CPreferences& prefs);

	void	StartNetworkMail();						// Start network mail controllers
	void	StopNetworkMail(bool quitting);			// Stop network mail controllers
	void	StartAddressBooks();					// Start address book management
	void	StopAddressBooks(bool quitting);		// Stop address book managment
	void	StartCalendars();						// Start calendar management
	void	StopCalendars(bool quitting);			// Stop calendar book managment

	// Opening/creating mailboxes
	bool	BeginINET(CINETProtocol* proto);	// Begin protocol
	bool	BeginRemote(CPreferences* prefs);	// Begin protocol
	JBoolean HandleCustomEvent();

	void	DoMailCheck();							// Check for new mail

	// Other bits
	bool	DoPreferences();						// Do preferences dialog

	void	ErrorPause(bool pause);					// Error displayed
	bool	IsPaused()								// Are periodics paused
		{ return mPaused; }

	void	UpdateSelectionColour();				// Change the color used for text/table selections

	void	CheckDefaultMailClient();				// Check whether its the default client for mailto's
	void	CheckDefaultWebcalClient();				// Check whether its the default client for webcal's
	
	bool	LaunchURL(const cdstring& url);					// Launch this URL

#if NOTYET 
	// Notification
	void  ShowNotification();
	void  HideNotification();
#endif

	// Sleep
	void		OnPowerSignal(int param);
	bool		GoToSleep(bool force);							// Computer is being put to sleep
	void		WakeFromSleep(bool silent = false);				// Computer woken from sleep

private:
	unsigned long		mCRC[5];			// CRC codes
	bool				mDoneProtect;		// Protection test done
	bool				mFailedProtect;		// Protection test
	bool				mStartInit;			// App started
	bool				mPrefsLoaded;		// Has at least one set of prefs been loaded
	bool				mMUPrefsUsed;		// Has an MU prefs been used rather than file
	bool				mRunDemo;			// Running as demo
	bool				mPaused;			// Periodics paused
	bool				mSleeping;			// Computer asleep
	time_t				mDemoTimeout;		// Demo timeout value
	bool				mNotificationOn;	// Notification displayed
	bool				startedYet;
	JRGB				mSelectionColour;	// Colour for text/table selections
	cdstring			mExtrasPath;		// Alternate path for extra files
};



#endif
