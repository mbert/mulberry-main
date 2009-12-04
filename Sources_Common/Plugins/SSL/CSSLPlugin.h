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

// CSSLPlugin.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 28-May-2000
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based security plug-ins in Mulberry.
//
// History:
// CD:	 28-May-2000:	Created initial header and implementation.
//

#ifndef __CSSLPLUGIN__MULBERRY__
#define __CSSLPLUGIN__MULBERRY__

#include "CPlugin.h"

#include "openssl_.h"

// Classes

class CSSLPlugin : public CPlugin
{
public:

	// Actual plug-in class

	CSSLPlugin(fspec files);
	virtual ~CSSLPlugin();

	// Initialise/terminate
	virtual bool VerifyVersion() const;					// Make sure version matches

		void InitSSL();
		void SetVerifyCallback(SSL_CTX* ctx);

protected:
	bool mInit;

	static int VerifyCallback(int ok, X509_STORE_CTX *ctx);
	int Verify(int ok, X509_STORE_CTX *ctx);
};

#endif
