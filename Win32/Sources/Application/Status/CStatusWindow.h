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

// Classes
class CUTF8StatusBar : public CStatusBar
{
public:
	CUTF8StatusBar() {}
	
	BOOL SetPaneTextUTF8(int nIndex, const char* utf8, BOOL bUpdate = TRUE);

	BOOL RedrawWindow(
			  LPCRECT lpRectUpdate = NULL,
			  CRgn* prgnUpdate = NULL,
			  UINT flags = RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE 
			 );
};

class CStatusWindow
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

	static	void	InitStatusBar(CUTF8StatusBar* statusBar);

	static	void	SetSMTPStatusStr(const cdstring& str);		// Set SMTP status string
	static	void	SetSMTPStatus(const char* rsrcid);		// Set SMTP status string
	static	void	SetSMTPStatus2(const char* rsrcid,
									long num1,
									long num2);				// Set SMTP status string with numbers

	static	void	SetIMAPStatusStr(const cdstring& str);		// Set IMAP status string
	static	void	SetIMAPStatus(const char* rsrcid);		// Set IMAP status string
	static	void	SetIMAPStatus1(const char* rsrcid,
									long num1);				// Set IMAP status string with a number
	static	void	SetIMAPStatus2(const char* rsrcid,
									long num1,
									long num2);				// Set IMAP status string with numbers

	static	void	SetSMTPProgressStr(const cdstring& str);	// Set SMTP status string
	static	void	SetSMTPProgress(long progress);			// Set SMTP status string

	static	void	SetIMAPProgressStr(const cdstring& str);	// Set IMAP status string
	static	void	SetIMAPProgress(long progress);			// Set IMAP status string

protected:
	static	void	SetDefaultStatus(void);			// Set default status text
	static	void	SetDefaultProgress(void);		// Set default progress text

private:
	static	CUTF8StatusBar*	sStatusBar;
	static	cdstring	sIdleStatus;
	static	cdstring	sIMAPStatus;
	static	cdstring	sSMTPStatus;
	static bool			sStatusLock;
	static bool			sChangeOnce;

	// Never create - always static
					CStatusWindow() {};

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

#endif
