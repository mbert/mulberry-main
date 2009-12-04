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


// Header for CMulberryApp class

#ifndef __CMULBERRYAPP__MULBERRY__
#define __CMULBERRYAPP__MULBERRY__

#include "CBalloonApp.h"

#include <PPxFSObject.h>

#include <Power.h>

const	OSType		kCRCType = 'colm';

// STR 's
enum {
	str_licensee = 128,
	str_serial_number = 129,
	str_organisation = 130,
	str_appregkey = 131,
	str_cryptoregkey = 132,
	colm_base = 128
};

const ResIDT	cDefaultWindows = 1;

// Globals

// Classes
class	CAddressList;
class	cdstring;
class	CHelpMenuAttachment;
class	CINETProtocol;
class	COptionsMap;
class	COptionsProtocol;
class	CPeriodicCheck;
class	CPeriodicReset;
class	CPreferences;
class	CPreferencesFile;
class	CRemotePrefsSet;

class	CMulberryApp : public CBalloonApp
{
	friend class CRegistrationDialog;
	friend class CPeriodicCheck;
	friend pascal long MySleepProc(long message, SleepQRecPtr qRecPtr);

// Instance variables
public:
	static CMulberryApp*		sApp;				// This application
	static bool					sSuspended;			// Indicates if App in background
	static COptionsProtocol*	sOptionsProtocol;	// The Options client
	static CRemotePrefsSet*		sRemotePrefs;		// Set of remote prefs
	static CPreferencesFile*	sCurrentPrefsFile;	// The current prefs file
	static CPeriodicCheck*		sMailCheck;			// Mail check
	static CPeriodicReset*		sMailCheckReset;	// Reset key check
	
	static ProcessSerialNumber	sPSN;				// Serial number of this process
	static ProcessInfoRec		sPInfo;				// Process info
	static FSSpec				sPSpec;				// Process file spec

private:
	static VersRecHndl			sVersion1;			// Apps version rsrc 1
	static VersRecHndl			sVersion2;			// Apps version rsrc 2
	
	unsigned long				mCRC[5];			// CRC codes
	bool						mDoneProtect;		// Protection test done
	bool						mFailedProtect;		// Protection test
	bool						mStartInit;			// App started
	bool						mPrefsLoaded;		// Has at least one set of prefs been loaded
	bool						mMUPrefsUsed;		// Has an MU prefs been used rather than file
	bool						mRunDemo;			// Running as demo
	bool						mPaused;			// Periodics paused
	bool						mSleeping;			// Sleep request handled
	bool						mTrackMouse;		// Mouse is being tracked by a pane
	
	CHelpMenuAttachment*		mHelpMenu;			// Help menu attachment used for help menu status updating

// Methods
public:
						CMulberryApp();
	virtual 			~CMulberryApp();

	static VersRecHndl	GetVersion(short versID);				// Get version record handle
	static NumVersion	GetVersionNumber(short versID = 1);		// Get version number

protected:
	virtual void		Initialize();
	virtual void		MakeMenuBar();
	virtual void		ClickMenuBar(const EventRecord &inMacEvent);


public:
	virtual void		DoQuit(SInt32 inSaveOption = kAEAsk);
			void		InitConnection(CPreferences& prefs);

			bool		DoRegistration(bool initial);		// Do registration dialog

	virtual void		UpdateMenus();

			void		SetTrackMouse(bool track)
			{
				mTrackMouse = track;
			}
protected:
	virtual void		EventResume(const EventRecord &inMacEvent);
	virtual void		EventSuspend(const EventRecord &inMacEvent);
	virtual void		AdjustCursor(const EventRecord& inMacEvent);

private:
	void		ReadDefaults();						// Read in default resources

	bool		Protect();							// Do copy protection checking

	void		PluginInit();						// Initialise plugins
	
	virtual void	StartUp();							// Startup without prefs
	bool			DoDefaultPrefsFile();				// Load prefs file from System Folder
	bool			DoMultiuser(COptionsMap* muser_prefs);	// Do multiuser dialog
	COptionsMap* 	GetMultiuser();						// Get multiuser prefs with update
	void			ReadMultiuser(COptionsMap* muser_prefs);// Read in certain fields from multi-user prefs
	bool			ForcePrefsSetting();				// Force setting of prefs, quit if cancel
	void			DoPostPrefsStartup();				// Continue startup after prefs

	void			StartNetworkMail();					// Start network mail controllers
	void			StopNetworkMail(bool quitting);		// Stop network mail controllers

	void			StartAddressBooks();				// Start address book management
	void			StopAddressBooks(bool quitting);	// Stop address book managment

	void			StartCalendars();					// Start calendar management
	void			StopCalendars(bool quitting);		// Stop calendar book managment

	void			DoAdbkImportExport();				// Warn adbk import/export
	void			DoSearchAddress();					// Search for a string

	void			OpenUp(bool first_time);				// Starting app
	void			CloseDown();							// Close down app without quit

	bool			GoToSleep(bool force);					// Computer is being put to sleep
	void			WakeFromSleep();						// Computer woken from sleep
	void			WakeFromSleepAction();					// Do actions when computer woken from sleep

public:	

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);


	virtual void		ShowAboutBox();						// Show fancy about box
	void		ShowAboutPluginsBox();				// Show about plugins box

	// Help menu bits
	void		AdjustHelpMenu();
	void		OnHelpDetailedTooltips();
	void		OnHelpMulberryOnline();
	void		OnHelpOnlineFAQ();
	void		OnHelpCheckUpdates();
	void		OnHelpMulberrySupport();
	void		OnHelpBuyMulberry();

	// Mailbox handling
	void		DoMailCheck();						// Check for new mail
	void		DoSearch();							// Search
	void		DoRules();							// Rules
	void		DoAdbkManager();					// Address Book manager
	void		DoCalendarManager();
	void		DoSubscribedCalendars();

	// Opening/creating mailboxes
	bool		BeginINET(CINETProtocol* proto);		// Begin protocol
	bool		BeginRemote(CPreferences* prefs);		// Begin protocol

	// Other bits
	void		DoDisconnected();					// Switch diconnected modes
	bool		DoMyPreferences();					// Do preferences dialog
	void		DoChangePassword();					// Do password changing

	void		DoSet3Pane(bool use3pane);

	void		OnEditMacros();

	void		OnWindowOptions();
	void		OnAppToolbarsShow();
	void		OnAppToolbarsSmall();
	void		OnAppToolbarsIcon();
	void		OnAppToolbarsCaption();
	void		OnAppToolbarsBoth();

	virtual void		OpenDocument(PPx::FSObject* inMacFSSpec);		// Open prefs file only

	bool		IsDemo() const						// Set run demo flag
		{ return mRunDemo; }
	void		RunDemo(bool demo)							// Set run demo flag
		{ mRunDemo = demo; }
	bool		LoadedPrefs() const
		{ return mPrefsLoaded; }
	bool		MUPrefsUsed() const					// Set run demo flag
		{ return mMUPrefsUsed; }

	void		CheckDefaultMailClient();			// Check whether its the default client for mailto's
	void		CheckDefaultWebcalClient();			// Check whether its the default client for webcal's

	void		ErrorPause(bool pause);				// Error displayed
	bool		IsPaused() const					// Are periodics paused
		{ return mPaused; }

	// Letter Doc related
	void		MakeNewLetterDoc(CAddressList* to = nil,
												CAddressList* cc = nil,
												CAddressList* bcc = nil);	// Create letter doc with initial addresses

	void		ChooseLetterDoc();					// Choose a letter document


	// Address Book Doc related
	void		MakeNewAddressBookDoc();		// Create address book doc

	void		ChooseAddressBookDoc();				// Choose an address book document

	virtual void		HandleAppleEvent(
									const AppleEvent	&inAppleEvent,
									AppleEvent			&outAEReply,
									AEDesc				&outResult,
									SInt32				inAENumber);	// Handle app specific events

	void		HandleGetURLEvent(
								const AppleEvent	&inAppleEvent,
								AppleEvent&			outAEReply,
								SInt32				inAENumber);
	void		HandleCheckMailEvent(
								const AppleEvent	&inAppleEvent,
								AppleEvent&			outAEReply,
								SInt32				inAENumber);
	static void		ProcessMailto(const char* mailto);
	static void		ProcessWebcal(const char* webcal);
};

#endif
