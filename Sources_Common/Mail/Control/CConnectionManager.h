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


// CConnectionManager.h

#ifndef __CCONNECTIONMANAGER__MULBERRY__
#define __CCONNECTIONMANAGER__MULBERRY__

#include "cdstring.h"

#include "CFutureItems.h"

// Classes
class CProgress;

class CConnectionManager
{
public:
	enum EMboxSync
	{
		eAllMessages = 0,
		eNewMessages,
		eNoMessages
	};
	
	enum EMessageSync
	{
		eEntireMessage = 0,
		eMessageBelow,
		eFirstDisplayable
	};
	
	class CConnectOptions
	{
	public:
		bool 			mStayConnected;
		bool 			mMboxPlayback;
		bool 			mUpdatePOP3;
		bool 			mSMTPSend;
		bool 			mAdbkPlayback;
		CFutureItems	mFuture;
		
		CConnectOptions();
		CConnectOptions(const CConnectOptions& copy)
			{ _copy(copy); }
		CConnectOptions& operator=(const CConnectOptions& copy)				// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }
		int operator==(const CConnectOptions& test) const;					// Compare with same type


		cdstring GetInfo() const;
		bool SetInfo(char_stream& info, NumVersion vers_prefs);
	
	private:
		void _copy(const CConnectOptions& copy);
	};

	class CDisconnectOptions
	{
	public:
		EMboxSync		mMboxSync;
		EMessageSync	mMsgSync;
		unsigned long	mMsgSyncSize;
		bool			mListSync;
		bool 			mSMTPWait;
		bool 			mAdbkSync;
		CFutureItems	mFuture;
		
		CDisconnectOptions();
		CDisconnectOptions(const CDisconnectOptions& copy)
			{ _copy(copy); }
		CDisconnectOptions& operator=(const CDisconnectOptions& copy)				// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }
		int operator==(const CDisconnectOptions& test) const;					// Compare with same type

		cdstring GetInfo() const;
		bool SetInfo(char_stream& info, NumVersion vers_prefs);
	
	private:
		void _copy(const CDisconnectOptions& copy);
	};

	static CConnectionManager sConnectionManager;

	CConnectionManager();
	~CConnectionManager();
	
	void SetConnected(bool connected)
		{ mConnected = connected; }
	bool IsConnected() const
		{ return mConnected; }
	bool CanDisconnect() const;

	bool DoConnection(bool connect);

	void Connect(CProgress* progress1, CProgress* progress2);
	void Disconnect(bool fast, bool force, CProgress* progress);

	void Suspend();
	void Resume();

	const cdstring& GetCWD() const;
	const cdstring& GetApplicationCWD() const
		{ return mApplicationCWD; }
	void SetApplicationCWD(const cdstring& cwd)
		{ mApplicationCWD = cwd; }

	const cdstring& GetUserCWD() const
		{ return mUserCWD; }

	// Special directories
	cdstring GetTempDirectory() const;
	cdstring GetSafetySaveDraftDirectory() const;
	cdstring GetExternalEditDraftDirectory() const;
	cdstring GetViewAttachmentDirectory() const;
	cdstring GetSecurityTempDirectory() const;
	cdstring GetSIEVEDirectory() const;
	cdstring GetAuthoritiesCertsDirectory() const;
	cdstring GetServerCertsDirectory() const;
	cdstring GetUserCertsDirectory() const;
	cdstring GetPersonalCertsDirectory() const;
	cdstring GetTimezonesDirectory() const;

private:
	bool mConnected;
	cdstring mApplicationCWD;
	cdstring mUserCWD;
	
	void CleanTempDirectories() const;
};

#endif
