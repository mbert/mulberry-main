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


// Header for MacTCP class

#ifndef __CTLSSOCKET__MULBERRY__
#define __CTLSSOCKET__MULBERRY__

#include "CTCPSocket.h"

#if __dest_os == __mac_os_x
#define _OS_X_SECURITY
#endif

#ifdef _OS_X_SECURITY
#include <Security/Security.h>
#else
//#include "openssl_.h"
struct ssl_ctx_st;
struct ssl_st;
struct x509_st;
struct evp_pkey_st;
#endif

class CTLSSocket : public CTCPSocket
{
public:
	static int	sDataIndex;

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

			CTLSSocket();
			CTLSSocket(const CTLSSocket& copy);
	virtual ~CTLSSocket();

	// O T H E R  M E T H O D S

	virtual void TCPClose();

			void TLSSetTLSOn(bool tls_on, int tls_type=0);
			bool TLSIsTLSOn() const
		{ return mTLSOn; }
			bool TLSSetClientCert(const cdstring& cert, const cdstring& passphrase);

	// Connections
	virtual void TCPWaitConnection();						// Passive connect (listen)

	virtual void TCPStartConnection();						// Active connect
			void TLSStartConnection();						// Start TLS session

    virtual void TCPCloseConnection();						// Close connect
            void TLSCloseConnection();						// Close TLS session

	// Receiving data
	virtual void TCPReceiveData(char* buf, long* len);			// Receive some data

	// Sending data
	virtual void TCPSendData(char* buf, long len);				// Send data

	// Certificate management
			void TLSAddCertError(int err);						// Add error to list of current certificate errors

	const cdstring&	GetCertText() const
	{
		return mCertText;
	}

protected:
	bool			mTLSOn;
	int				mTLSType;
#ifdef _OS_X_SECURITY
    SSLContextRef   m_ctx;
#else
	ssl_ctx_st*		m_ctx;
	ssl_st*			m_tls;
	x509_st*		mClientCert;
	evp_pkey_st*	mClientPrivate;
#endif
	std::vector<int>	mCertErrors;
	cdstring            mCertText;
	cdstring            mCertSubject;
	cdstring            mCertIssuer;
	cdstring            mCipher;

	virtual void TLSReceiveData(char* buf, long* len);			// Receive some data
	virtual void TLSSendData(char* buf, long len);				// Send data
    
private:
#ifdef _OS_X_SECURITY
	static OSStatus TLSReadFunc(SSLConnectionRef ref, void* data, size_t* len);
	static OSStatus TLSWriteFunc(SSLConnectionRef ref, const void* data, size_t* len);
#endif
};

#endif
