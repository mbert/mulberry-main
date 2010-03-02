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

// CSSLPlugin.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 04-May-1998
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a wrapper for DLL based security plug-ins in Mulberry.
//
// History:
// CD:	 04-May-1998:	Created initial header and implementation.
//

#include "CSSLPlugin.h"

#include "CCertificateManager.h"
#include "CDLLFunctions.h"
#include "CErrorHandler.h"
#include "CPluginManager.h"
#include "CPreferenceVersions.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CMulberryApp.h"
#include "CStringResources.h"
#endif
#include "CTLSSocket.h"
#include "CUtils.h"

CDLLLoader* sSSLLoader = NULL;

extern "C" int RAND_load_rsrc();

CSSLPlugin::CSSLPlugin(fspec files) :
		CPlugin(files)
{
	mInit = false;
}

CSSLPlugin::~CSSLPlugin()
{
	if (mInit)
	{
		delete CCertificateManager::sCertificateManager;
		
		// Always loaded in this state
		UnloadPlugin();
	}
}

// Make sure version matches
bool CSSLPlugin::VerifyVersion() const
{
	// New API >= 3.1b3
	if (VersionTest(GetVersion(), VERS_3_1_0_B_3) >= 0)
		return true;
	else
	{
		CErrorHandler::PutStopAlertRsrcStr("Alerts::General::IllegalPluginCryptoVersion", GetName().c_str());
		return false;
	}
}

void CSSLPlugin::InitSSL()
{
	if (!mInit)
	{
		// Most load the plugin and keep it loaded
		LoadPlugin();

#if __dest_os == __win32_os
		sSSLLoader = new CDLLLoader(mConn);
#endif

		// Init TLS library
#if __dest_os == __linux_os
		//RAND_load_rsrc();
#endif

		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();

		new CCertificateManager();

		mInit = true;
	}
}

void CSSLPlugin::SetVerifyCallback(SSL_CTX* ctx)
{
	// Set cert verify callback
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, VerifyCallback);
}

int CSSLPlugin::VerifyCallback(int ok, X509_STORE_CTX *ctx)
{
	return CPluginManager::sPluginManager.GetSSL()->Verify(ok, ctx);
}

int CSSLPlugin::Verify(int ok, X509_STORE_CTX *ctx)
{
	// Add error to list of errors for this certificate
	if (ctx->error != X509_V_OK)
	{
		SSL* s = (SSL*) X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
		CTLSSocket* tls = (CTLSSocket*) SSL_get_ex_data(s, CTLSSocket::sDataIndex);
		tls->TLSAddCertError(ctx->error);
	}

	// Always accept the certificate here - we will check the list
	// of errors later and kill the connection if its not acceptable
	return 1;
}

