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

/* 
	CWebDAVPrefsClient.h

	Author:			
	Description:	<describe the CWebDAVPrefsClient class here>
*/

#ifndef CWebDAVPrefsClient_H
#define CWebDAVPrefsClient_H

#include "COptionsClient.h"
#include "CWebDAVPropFindParser.h"
#include "CWebDAVSession.h"

#include "XMLNode.h"

namespace prefsstore {

class CWebDAVPrefsClient : public COptionsClient, public http::webdav::CWebDAVSession
{
public:
	CWebDAVPrefsClient(COptionsProtocol* owner);
	CWebDAVPrefsClient(const CWebDAVPrefsClient& copy, COptionsProtocol* owner);
	virtual ~CWebDAVPrefsClient();

private:
			void	InitWebDAVClient();

public:
	virtual CINETClient*	CloneConnection();			// Create duplicate, empty connection

	virtual void	Open();									// Open account
	virtual void	Reset();								// Reset account

	virtual void	Logon();								// Logon to server
	virtual void	Logoff();								// Logoff from server
	
protected:
	cdstring		mBaseURL;
	cdstring		mBaseRURL;
	cdstrmap		mLockTokens;
	cdstring		mAuthUniqueness;

	virtual tcp_port GetDefaultPort();						// Get default port;

	// P R O T O C O L
	virtual void	_InitCapability();					// Initialise capability flags to empty set
	virtual void	_ProcessCapability();				// Handle capability response
	virtual void	_Tickle(bool force_tickle);			// Do tickle

	// O P T I O N S
	virtual void	_FindAllAttributes(const cdstring& entry);		// Find all attributes in entry
	virtual void	_SetAllAttributes(const cdstring& entry);		// Set all attributes in entry
	virtual void	_DeleteEntry(const cdstring& entry);			// Delete the entire entry
	virtual void	_GetAttribute(const cdstring& entry,			// Get attribute
									const cdstring& attribute);
	virtual void	_SetAttribute(const cdstring& entry,			// Set attribute
									const cdstring& attribute,
									const cdstring& value);
	virtual void	_DeleteAttribute(const cdstring& entry,			// Delete attribute
									const cdstring& attribute);

protected:
	// H A N D L E  E R R O R
	virtual const char*	INETGetErrorDescriptor() const;			// Descriptor for object error context

	virtual void OpenSession();
	virtual void CloseSession();
	void RunSession(CHTTPRequestResponse* request);
	void DoSession(CHTTPRequestResponse* request);

	virtual void SetServerType(unsigned long type);
	virtual void SetServerDescriptor(const cdstring& txt);
	virtual void SetServerCapability(const cdstring& txt);

	virtual CHTTPAuthorization* GetAuthorization(bool first_time, const cdstrvect& www_authenticate);
			bool CheckCurrentAuthorization() const;
	virtual void DoRequest(CHTTPRequestResponse* request);
	virtual void WriteRequestData(CHTTPRequestResponse* request);
	virtual void ReadResponseData(CHTTPRequestResponse* request);
	virtual void ReadResponseDataLength(CHTTPRequestResponse* request, unsigned long read_length);
	virtual unsigned long ReadResponseDataChunked(CHTTPRequestResponse* request);

	virtual void HandleHTTPError(CHTTPRequestResponse* request);
	virtual void DisplayHTTPError(CHTTPRequestResponse* request);

	cdstring GetETag(const cdstring& rurl, const cdstring& lock_token = cdstring::null_str);
	cdstring GetProperty(const cdstring& rurl, const cdstring& lock_token, const xmllib::XMLName& property);
	cdstring LockResource(const cdstring& rurl, unsigned long timeout, bool lock_null = false);
	void UnlockResource(const cdstring& rurl, const cdstring& lock_token);

	virtual cdstring GetRURL(const cdstring& name, bool directory, bool abs = false) const;
	cdstring GetLockToken(const cdstring& rurl) const;
	bool	 LockToken(const cdstring& rurl, unsigned long timeout, bool lock_null = false);
	void	 UnlockToken(const cdstring& rurl);
	
	void	WriteData(const cdstring& entry, const cdstring& data);
	void	ReadData(const cdstring& entry, cdstring& data);
};

}	// namespace prefsstore

#endif	// CWebDAVPrefsClient_H
