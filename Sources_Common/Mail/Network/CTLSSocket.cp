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


// Code for MacTCP class

#include "CTLSSocket.h"

#include "CCertificateManager.h"
#include "CLog.h"
#include "CMailControl.h"
#include "CPluginManager.h"
#include "CSSLPlugin.h"
#include "CStringUtils.h"
#include "CTCPException.h"

#include "openssl_.h"

#include <algorithm>

#if defined(_bsdsock)
#include <errno.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#ifdef _winsock
#define tcp_errno ::WSAGetLastError()
#define ASYNC_DB
#else
#define tcp_errno errno
#endif

#ifdef _winsock
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
//#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
//#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#endif

int	CTLSSocket::sDataIndex = -1;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTLSSocket::CTLSSocket()
{
	mTLSOn = false;
	mTLSType = 0;
	m_ctx = NULL;
	m_tls = NULL;
	mClientCert = NULL;
	mClientPrivate = NULL;
}

// Copy constructor
CTLSSocket::CTLSSocket(const CTLSSocket& copy) :
	CTCPSocket(copy)
{
	mTLSOn = false;
	mTLSType = 0;
	m_ctx = NULL;
	m_tls = NULL;
	mClientCert = NULL;
	mClientPrivate = NULL;
}

CTLSSocket::~CTLSSocket()
{
	m_ctx = NULL;
	m_tls = NULL;

	if (mClientCert != NULL)
	{
		::X509_free(mClientCert);
		mClientCert = NULL;
	}
	if (mClientPrivate != NULL)
	{
		::EVP_PKEY_free(mClientPrivate);
		mClientPrivate = NULL;
	}
}

void CTLSSocket::TCPClose()
{
	// Inherited
	CTCPSocket::TCPClose();

	// Dispose of TLS specific items
	mTLSOn = false;
	if (m_tls != NULL)
	{
		::SSL_free(m_tls);
		m_tls = NULL;
	}
	if (m_ctx != NULL)
	{
		::SSL_CTX_free(m_ctx);
		m_ctx = NULL;
	}
	if (mClientCert != NULL)
	{
		::X509_free(mClientCert);
		mClientCert = NULL;
	}
	if (mClientPrivate != NULL)
	{
		::EVP_PKEY_free(mClientPrivate);
		mClientPrivate = NULL;
	}
}


// O T H E R  M E T H O D S _________________________________________________________________________

#pragma mark ________________________________start/stop

void CTLSSocket::TLSSetTLSOn(bool tls_on, int tls_type)
{
	// Only allow this to be switched on if TLS is actually present
	if (CPluginManager::sPluginManager.HasSSL())
	{
		mTLSOn = tls_on;
		mTLSType = tls_type;
		
		// Init library if turning on
		if (mTLSOn)
		{
			CPluginManager::sPluginManager.GetSSL()->InitSSL();
			CCertificateManager::sCertificateManager->LoadCertificates();
		}
	}
	else if (tls_on)
		// Throw if trying to turn TLS on and no plugin
	{
		CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPNoSSLPlugin);
		throw CTCPException(CTCPException::err_TCPNoSSLPlugin);
	}
}

// Load and verify client cert data
bool CTLSSocket::TLSSetClientCert(const cdstring& cert, const cdstring& passphrase)
{
	// Remove previous certs, if any
	if (mClientCert != NULL)
	{
		::X509_free(mClientCert);
		mClientCert = NULL;
	}
	if (mClientPrivate != NULL)
	{
		::EVP_PKEY_free(mClientPrivate);
		mClientPrivate = NULL;
	}

	// cert name is the fingerprint
	mClientCert = CCertificateManager::sCertificateManager->FindCertificate(cert,  CCertificateManager::ePersonalCertificates, CCertificateManager::eByFingerprint);
	mClientPrivate = CCertificateManager::sCertificateManager->LoadPrivateKey(cert, passphrase, CCertificateManager::eByFingerprint);
	
	if ((mClientCert == NULL) || (mClientPrivate == NULL))
	{
		// Clean up
		if (mClientCert != NULL)
		{
			::X509_free(mClientCert);
			mClientCert = NULL;
		}
		if (mClientPrivate != NULL)
		{
			::EVP_PKEY_free(mClientPrivate);
			mClientPrivate = NULL;
		}
		
		return false;
	}
	else
		return true;
}

// C O N N E C T I O N S ____________________________________________________________________________

#pragma mark ________________________________connections

// Wait for an incoming connection from remote host
void CTLSSocket::TCPWaitConnection()
{
	// Do inherited
	CTCPSocket::TCPWaitConnection();
	
	// Do TLS negotiation
	if (mTLSOn)
		TLSStartConnection();
}

// Initiate a connection with remote host
void CTLSSocket::TCPStartConnection()
{
	// Do inherited
	CTCPSocket::TCPStartConnection();
	
	// Do SSL negotiation if switched on already
	if (mTLSOn)
		TLSStartConnection();
}

// Receive data
void CTLSSocket::TCPReceiveData(char* buf, long* len)
{
	// Look for existing TLS session
	if (m_tls)
		TLSReceiveData(buf, len);
	else
		CTCPSocket::TCPReceiveData(buf, len);
}

// Send data
void CTLSSocket::TCPSendData(char* buf, long len)
{
	// Look for existing TLS session
	if (m_tls)
		TLSSendData(buf, len);
	else
		CTCPSocket::TCPSendData(buf, len);
}

#pragma mark ________________________________TLS Specific

// Initiate a connection with remote host
void CTLSSocket::TLSStartConnection()
{
	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

	X509* server_cert = NULL;

	try
	{
		// Clear out any cached data
		mCertErrors.clear();
		mCertSubject = cdstring::null_str;
		mCertIssuer = cdstring::null_str;
		mCipher = cdstring::null_str;

		// Create context
		switch(mTLSType)
		{
			case 1:
			case 4:
				m_ctx = ::SSL_CTX_new(::SSLv23_client_method());
				break;
			case 2:
				m_ctx = ::SSL_CTX_new(::SSLv3_client_method());
				break;
			case 3:
				m_ctx = ::SSL_CTX_new(::TLSv1_client_method());
				break;
		}
		if (!m_ctx)
		{
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLError);
			throw CTCPException(CTCPException::err_TCPSSLError);
		}

		// Work around all known bugs
	    ::SSL_CTX_ctrl(m_ctx, SSL_CTRL_OPTIONS, SSL_OP_ALL, NULL);
		
		// Setup certificates
		CCertificateManager::sCertificateManager->LoadSSLRootCerts(m_ctx);

		// Set cert verify callback
		CPluginManager::sPluginManager.GetSSL()->SetVerifyCallback(m_ctx);

		// Start negotiation
		m_tls = ::SSL_new(m_ctx);
		if (!m_tls)
		{
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLError);
			throw CTCPException(CTCPException::err_TCPSSLError);
		}

		// Create data index if not already
		if (sDataIndex == -1)
		{
			cdstring index("CTLSSocket");
			sDataIndex = ::SSL_get_ex_new_index(0, index.c_str_mod(), NULL, NULL, NULL);
		}

		// Add pointer to this as ex_data
		::SSL_set_ex_data(m_tls, CTLSSocket::sDataIndex, (char *)this);

		// Set socket
		::SSL_set_fd(m_tls, mSocket);

		// Load in client certificates if present
		if ((mClientCert != NULL) && (mClientPrivate != NULL))
		{
			::SSL_use_certificate(m_tls, mClientCert);
			::SSL_use_PrivateKey(m_tls, mClientPrivate);
		}

		int result;
		while((result = ::SSL_connect(m_tls)) == SOCKET_ERROR)
		{
			// Check for failure
			int ssl_err = ::SSL_get_error(m_tls, result);
			if ((ssl_err == SSL_ERROR_WANT_READ) || (ssl_err == SSL_ERROR_WANT_WRITE))

				// Yield while waiting to unblock
				TCPYield();

			else
			{
				// Convert to socket error
				if (ssl_err == SSL_ERROR_SYSCALL)
					ssl_err = tcp_errno;
				else
					ssl_err = CTCPException::err_TCPSSLError;

				TCPAbort(true);

				TCPHandleError(ssl_err);

				// Throw error
				CLOG_LOGTHROW(CTCPException, ssl_err);
				throw CTCPException(ssl_err);
			}
		}

		// Must check connection state
		const char * statstr = ::SSL_state_string_long(m_tls);

		if (::SSL_state(m_tls) != SSL_ST_OK)
		{
			TCPAbort(true);
			cdstring temp(statstr);

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLError);
			throw CTCPException(CTCPException::err_TCPSSLError);
		}
		
		// Get server certificate
		server_cert = ::SSL_get_peer_certificate(m_tls);
		if (!server_cert)
		{
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLCertError);
			throw CTCPException(CTCPException::err_TCPSSLCertError);
		}

		
		CCertificateManager::sCertificateManager->CertificateToString(server_cert, mCertText);

  		char x509_buf[BUFSIZ];
		char* str = ::X509_NAME_oneline(X509_get_subject_name(server_cert), x509_buf, BUFSIZ);
		if (!str)
		{
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLCertError);
			throw CTCPException(CTCPException::err_TCPSSLCertError);
		}
		mCertSubject = str;

		str = ::X509_NAME_oneline(X509_get_issuer_name(server_cert), x509_buf, BUFSIZ);
		if (!str)
		{
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLCertError);
			throw CTCPException(CTCPException::err_TCPSSLCertError);
		}
		mCertIssuer = str;

		// Get cipher in use
		SSL_CIPHER* cipher = ::SSL_get_current_cipher(m_tls);
		
		char cipher_desc[256];
		::SSL_CIPHER_description(cipher, cipher_desc, 256);
		mCipher = cipher_desc;

		// Must now check to see whether the certificate is acceptable based
		// on any errors that occurred while setting up the connection. Use the original remote name for the test,
		// not the possibly reverse looked-up name.
		if (!CCertificateManager::sCertificateManager->CheckServerCertificate(mOriginalName, mCertErrors, server_cert))
		{
			TCPAbort(true);

			// Throw error
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPSSLCertNoAccept);
			throw CTCPException(CTCPException::err_TCPSSLCertNoAccept);
		}

		// Done with certificate
		::X509_free(server_cert);
		server_cert = NULL;
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up
		if (server_cert)
			::X509_free(server_cert);
		if (m_tls)
		{
			::SSL_free(m_tls);
			m_tls = NULL;
		}
		if (m_ctx)
		{
			::SSL_CTX_free(m_ctx);
			m_ctx = NULL;
		}
		
		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}
}

// Receive data
void CTLSSocket::TLSReceiveData(char* buf, long* len)
{
	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

	// Yield before to get any user abort
	TCPYield();

	// Get any data present at the moment
	int result;
	while((result = ::SSL_read(m_tls, buf, *len)) == SOCKET_ERROR)
	{
		// Check for failure
		int ssl_err = ::SSL_get_error(m_tls, result);
		if (ssl_err == SSL_ERROR_WANT_READ)

			// Yield while waiting to unblock
			TCPSelectYield(true);

		else
		{
			// Convert to socket error
			if (ssl_err == SSL_ERROR_SYSCALL)
				ssl_err = tcp_errno;
			else
				ssl_err = CTCPException::err_TCPSSLError;

			TCPAbort(true);

			TCPHandleError(ssl_err);

			// Throw error
			CLOG_LOGTHROW(CTCPException, ssl_err);
			throw CTCPException(ssl_err);
		}
	}

	// Check for broken connection
	if (!result)
	{
		TCPAbort(true);

		int err = ECONNRESET;

		TCPHandleError(err);

		// Got an error => throw
		CLOG_LOGTHROW(CTCPException, err);
		throw CTCPException(err);
	}

	*len = result;

	// Reset tickle timer
	TimerReset();
}

// Send data
void CTLSSocket::TLSSendData(char* buf, long len)
{
	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

	// Yield before to get any user abort
	TCPYield();

	// Send data in blocks of buffer size
	char* p = buf;
	while(len)
	{
		int result = ::SSL_write(m_tls, p, len);

		// Check for failure
		if (result == SOCKET_ERROR)
		{
			// Check type of error
			int ssl_err = ::SSL_get_error(m_tls, result);
			if (ssl_err == SSL_ERROR_WANT_WRITE)

				// Yield while waiting to unblock
				TCPSelectYield(false);

			else
			{
				// Convert to socket error
				if (ssl_err == SSL_ERROR_SYSCALL)
					ssl_err = tcp_errno;
				else
					ssl_err = CTCPException::err_TCPSSLError;

				TCPAbort(true);

				TCPHandleError(ssl_err);

				// Throw error
				CLOG_LOGTHROW(CTCPException, ssl_err);
				throw CTCPException(ssl_err);
			}
		}
		else
		{
			// Adjust buffer
			len -= result;
			p += result;
		}
	}

	// Reset tickle timer
	TimerReset();
}

// Add error to list of current certificate errors
void CTLSSocket::TLSAddCertError(int err)
{
	// Add to list if unique
	std::vector<int>::const_iterator found = std::find(mCertErrors.begin(), mCertErrors.end(), err);
	if (found == mCertErrors.end())
		mCertErrors.push_back(err);
}
