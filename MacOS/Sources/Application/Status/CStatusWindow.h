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


// Header for CStatusWindow class

#ifndef __CSTATUSWINDOW__MULBERRY__
#define __CSTATUSWINDOW__MULBERRY__


#include "CProgress.h"

// Consts

// Panes
const	ClassIDT	class_StatusWindow ='SWin';
const	PaneIDT		paneid_StatusWindow = 8000;
const	PaneIDT		paneid_StatusWindow8 = 8002;
const	PaneIDT		paneid_SMTPStatus = 'SMTP';
const	PaneIDT		paneid_IMAPStatus = 'IMAP';
const	PaneIDT		paneid_SMTPProgress = 'SDON';
const	PaneIDT		paneid_IMAPProgress = 'IDON';

// Messages

// Resources

// Classes

class CStaticText;

class CStatusWindow : public LWindow
{

public:
	class StStatusLock
	{
	public:
		StStatusLock()
			{ if (!sStatusLock) sStatusLock = true; sChangeOnce = false; mLocked = sStatusLock; }
		~StStatusLock()
			{ if (mLocked) sStatusLock = false; }
		void AllowChange() const
			{ if (mLocked) sChangeOnce = true; }
	private:
		bool mLocked;
	};

	friend class StStatusLock;

	enum { class_ID = 'SWin' };

	static CStatusWindow* sStatusWindow;

					CStatusWindow();
					CStatusWindow(LStream *inStream);
	virtual 		~CStatusWindow();
	
protected:
	virtual void	FinishCreateSelf(void);
	virtual void	SetDefaultStatus(void);					// Set default status text
	virtual void	SetDefaultProgress(void);				// Set default progress text

public:
	virtual void	ResetState(void);						// Reset window state from prefs
	virtual void	SaveState(void);						// Save state in prefs

	static	void	SetSMTPStatus(const cdstring& str);		// Set SMTP status string
	static	void	SetSMTPStatus(const char* rsrc);		// Set SMTP status string
	static	void	SetSMTPStatus2(const char* rsrc,
									long num1,
									long num2);				// Set SMTP status string with numbers

	static	void	SetIMAPStatus(const cdstring& str);		// Set IMAP status string
	static	void	SetIMAPStatus(const char* rsrcid);		// Set IMAP status string
	static	void	SetIMAPStatus1(const char* rsrcid,
									long num1);				// Set IMAP status string with a number
	static	void	SetIMAPStatus2(const char* rsrcid,
									long num1,
									long num2);				// Set IMAP status string with numbers

	static	void	SetSMTPProgress(const cdstring& str);	// Set SMTP status string
	static	void	SetSMTPProgress(long progress);			// Set SMTP status string

	static	void	SetIMAPProgress(const cdstring& str);	// Set IMAP status string
	static	void	SetIMAPProgress(long progress);			// Set IMAP status string

	virtual void	AttemptClose();							// Hide instead of close

	static	void	SetSMTPStatusStr(const cdstring& status);
	static	void	SetSMTPProgressStr(const cdstring& status);
	static	void	SetIMAPStatusStr(const cdstring& status);
	static	void	SetIMAPProgressStr(const cdstring& status);

private:
	CStaticText*		mSMTPStatus;
	CStaticText*		mIMAPStatus;
	CStaticText*		mSMTPProgress;
	CStaticText*		mIMAPProgress;
	static bool			sStatusLock;
	static bool			sChangeOnce;
	
			void	SetSMTPStatusPrivate(const cdstring& status);
			void	SetSMTPProgressPrivate(const cdstring& status);
			void	SetIMAPStatusPrivate(const cdstring& status);
			void	SetIMAPProgressPrivate(const cdstring& status);
};

class CSMTPAttachProgress : public CProgress
{
public:
					CSMTPAttachProgress();
	virtual			~CSMTPAttachProgress();
	
	virtual void	SetPercentage(unsigned long percentage);
	virtual void	SetCount(unsigned long count);

};

class CNetworkAttachProgress : public CProgress
{
public:
					CNetworkAttachProgress();
	virtual			~CNetworkAttachProgress();
	
	virtual void	SetPercentage(unsigned long percentage);
	virtual void	SetCount(unsigned long count);

};

class StStatusWindowHide
{
public:
					StStatusWindowHide();
					~StStatusWindowHide();

private:
	bool			mWasVisible;
};

#endif
