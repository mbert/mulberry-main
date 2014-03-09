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


// Code for MacTCP class

#include "CTCPSocket.h"

#include "CLog.h"
#include "CMailControl.h"
#include "CTCPException.h"

#if defined(_mac_winsock)
#include "COpenTransport.h"
#elif defined(_winsock)
#include "CWininet.h"
#elif defined(_bsdsock)
#if __dest_os == __linux_os
#include "CUnixINET.h"
#define CINETUtils	CUnixINET
#elif __dest_os == __mac_os_x
#include "CINETUtils.h"
#endif
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h> //for struct linger for set_sock_option with SO_LINGER
typedef hostent HOSTENT;
#include <sys/time.h> //for timeval
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#endif

#define RESULT_NO_ERR 0

#ifdef _winsock
#define tcp_errno ::WSAGetLastError()
#define dns_errno ::WSAGetLastError()
#define ASYNC_DB
#else
#define tcp_errno errno
#define dns_errno h_errno
#endif

#include <algorithm>

#ifdef ASYNC_DB
#ifdef _mac_winsock
#define	WM_TCP_ASYNCDNR	(u_int) this
#define	WM_TCP_ASYNCSELECT	(u_int) this
#else
#define	WM_TCP_ASYNCDNR	(WM_USER + 2)
#define	WM_TCP_ASYNCSELECT	(WM_USER + 3)
#endif
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

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

unsigned long CTCPSocket::sConnectRetryTimeout = 15;	// Application may change this
unsigned long CTCPSocket::sConnectRetryMaxCount = 3;	// Application may change this

long CTCPSocket::sDrvrCtr = 0;
cdstring CTCPSocket::sLocalName;
CTCPSocketList CTCPSocket::sSockets;
cdmutex CTCPSocket::sLock;				// mutex for access to static members
cdmutex CTCPSocket::sLockLocalName;		// mutex for access to sLocalName
#ifdef _winsock
#ifdef _mac_winsock
NMMessageHandler CTCPSocket::sAsyncHandler = NM_INVALID_HANDLER;		// Async database op handler
#else
HWND CTCPSocket::sAsyncHandler = nil;		// Async database op handler
#endif
typedef int socklen_t;
#endif

const short TCP_WINSOCK_VERSION		= 0x0101;
const long cTCPDefaultTimeout 		= 30L * 60L;

// Default constructor
CTCPSocket::CTCPSocket()
{
	mDrvrOpen = false;
	mLocalPort = 0;
	::memset((char *) &mRemote, 0, sizeof(mRemote));

	mTCPState = TCPNotOpen;
	mCancel = false;
	mAbort = false;

	mLastClock = 0;

#ifdef _winsock
	mAsyncHandle = NULL;
	mAsyncSelect = false;
	mAsyncError = 0;
#ifdef _mac_winsock
	mAsyncHandler = NM_INVALID_HANDLER;
#else
	mAsyncHandler = sAsyncHandler;
#endif
#endif

	// Add to global list of sockets
	{
		cdmutex::lock_cdmutex _lock(sLock);
		sSockets.push_back(this);
	}
}

// Copy constructor
CTCPSocket::CTCPSocket(const CTCPSocket& copy)
{
	mDrvrOpen = false;
	mDescriptor = copy.mDescriptor;
	mLocalPort = 0;
	mRemote = copy.mRemote;
	mOriginalName = copy.mOriginalName;
	mRemoteName = copy.mRemoteName;
	mRemoteCName = copy.mRemoteCName;

	mTCPState = TCPNotOpen;
	mCancel = false;
	mAbort = false;

	mLastClock = 0;

#ifdef _winsock
	mAsyncHandle = NULL;
	mAsyncError = 0;
#ifdef _mac_winsock
	mAsyncHandler = NM_INVALID_HANDLER;
#else
	mAsyncHandler = sAsyncHandler;
#endif
#endif

	// Add to global list of sockets
	{
		cdmutex::lock_cdmutex _lock(sLock);
		sSockets.push_back(this);
	}
}

CTCPSocket::~CTCPSocket()
{
	// Must be closed
	TCPClose();

	// Remove from global list of sockets
	{
		cdmutex::lock_cdmutex _lock(sLock);
		CTCPSocketList::iterator found = std::find(sSockets.begin(), sSockets.end(), this);
		if (found != sSockets.end())
			sSockets.erase(found);
	}
}

bool CTCPSocket::HasInet()
{
#if defined(_mac_winsock)
	return !COpenTransport::HasOpenTransport() || COpenTransport::HasInet();
#elif defined(_winsock)
	return !CWininet::HasWININET() || CWininet::HasInet();
#elif defined(_bsdsock)
	return CINETUtils::HasInet();
#endif
}

bool CTCPSocket::WillDial()
{
#if defined(_mac_winsock)
	// Special hack to prevent 68K crash
#if TARGET_RT_MAC_CFM
	return COpenTransport::HasOpenTransport() && COpenTransport::WillDial();
#else
	return false;
#endif
#elif defined(_winsock)
	return CWininet::HasWININET() && CWininet::WillDial();
#elif defined(_bsdsock)
	return CINETUtils::WillDial();
#endif
}

bool CTCPSocket::HasModem()
{
#if defined(_mac_winsock)
	return COpenTransport::HasOpenTransport() && COpenTransport::HasModem();
#elif defined(_winsock)
	return CWininet::HasWININET() && CWininet::HasModem();
#elif defined(_bsdsock)
	return CINETUtils::HasModem();
#endif
}

bool CTCPSocket::AutoDial()
{
	// Remove any local name as it may be reassigned on next dialup
	sLocalName = cdstring::null_str;

#if defined(_mac_winsock)
	 if (COpenTransport::HasOpenTransport())
	 {
	 	if (COpenTransport::AutoDial())
	 	{
			do
			{
				CMailControl::ProcessBusy(NULL, false);
			} while(!COpenTransport::ActionComplete());
			
			return COpenTransport::DialConnected();
		}
		else
			return false;
	 }
	 
	 return true;
#elif defined(_winsock)
	return CWininet::HasWININET() && CWininet::AutoDial();
#elif defined(_bsdsock)
	return CINETUtils::AutoDial();
#endif
}

bool CTCPSocket::AutoHangup()
{
#if defined(_mac_winsock)
	 if (COpenTransport::HasOpenTransport())
	 {
	 	if (COpenTransport::AutoHangup())
	 	{
			do
			{
				CMailControl::ProcessBusy(NULL, false);
			} while(!COpenTransport::ActionComplete());
			
			return !COpenTransport::DialConnected();
		}
		else
			return false;
	 }
	 
	 return true;
#elif defined(_winsock)
	return CWininet::HasWININET() && CWininet::AutoHangup();
#elif defined(_bsdsock)
	return CINETUtils::AutoHangup();
#endif
}

unsigned long CTCPSocket::CountConnected()
{
	unsigned long ctr = 0;

	// Look at each registered socket
	cdmutex::lock_cdmutex _lock(sLock);
	for(CTCPSocketList::iterator iter = sSockets.begin(); iter != sSockets.end(); iter++)
	{
		// Check whether its connected
		if ((*iter)->TCPGetState() == TCPConnected)
			ctr++;
	}
	
	return ctr;
}

// Check whether any connected sockets require a valid network interface (not localhost)
// and see whether at least one active interface exists.
bool CTCPSocket::CheckConnectionState()
{
	// Get list of active interfaces (not localhost)
	CTCPAddrList good_addrs;
	GetInterfaceIPs(good_addrs, false);
	bool has_active = (good_addrs.size() != 0);

	// Look at each registered socket to see which are connected
	cdmutex::lock_cdmutex _lock(sLock);
	for(CTCPSocketList::iterator iter = sSockets.begin(); iter != sSockets.end(); iter++)
	{
		// Check whether its supposed to be connected
		if ((*iter)->TCPGetState() == TCPConnected)
		{
			// Get local IP address
			ip_addr local_ip = (*iter)->TCPGetLocalConnectedIP();
			
			// Ignore if 127.0.0.1 (localhost)
			if (ntohl(local_ip) == 0x7F000001)
				continue;
			
			// Anything else requires an active interface
			if (!has_active)
				return false;
		}
	}
	
	return true;
}

// Make sure each connected socket has a local ip that matches one currently active on the system.
// If no match, optionally force the connection into thr abort state so that recovery can take place.
bool CTCPSocket::CheckConnections(bool check_only)
{
	bool result = true;

	// Check validity of all open connections
	
	// Get list of good or bad addresses (if no interface info available, pretend all are OK)
	CTCPAddrList good_addrs;
	if (!GetInterfaceIPs(good_addrs))
		return true;

	// Look at each registered socket
	cdmutex::lock_cdmutex _lock(sLock);
	for(CTCPSocketList::iterator iter = sSockets.begin(); iter != sSockets.end(); iter++)
	{
		// Check whether its supposed to be connected
		if ((*iter)->TCPGetState() == TCPConnected)
		{
			// Get local IP address
			ip_addr local_ip = (*iter)->TCPGetLocalConnectedIP();
			
			// Ignore if 0 or 127.0.0.1 (localhost)
			if ((local_ip == 0) || (ntohl(local_ip) == 0x7F000001))
				continue;
			
			// Check whether its in the good list
			CTCPAddrList::const_iterator found = std::find(good_addrs.begin(), good_addrs.end(), local_ip);
			if (found != good_addrs.end())
				continue;
			
			// Must force abort on this socket if required - this should cause an INETClient reconnect attempt
			if (!check_only)
				(*iter)->SetAbort();
			result = false;
		}
	}
	
	return result;
}

bool CTCPSocket::GetInterfaceIPs(CTCPAddrList& addrs, bool include_localhost)
{
	// Get ips from network sub-system
	ip_addr* ips = NULL;
#if defined(_mac_winsock)
	bool result = COpenTransport::GetInterfaceIPs(ips);
#elif defined(_winsock)
	bool result = CWininet::GetInterfaceIPs(ips);
#elif defined(_bsdsock)
	bool result = CINETUtils::GetInterfaceIPs(ips);
#endif

	// Copy ip list into vector
	if (result && ips)
	{
		ip_addr* p = ips;
		while(*p)
		{
			// Ignore localhost ips if requested
			if (!include_localhost && (ntohl(*p) == 0x7F000001))
			{
				p++;
				continue;
			}

			// Only insert if not duplicate
			CTCPAddrList::const_iterator found = std::find(addrs.begin(), addrs.end(), *p);
			if (found != addrs.end())
			{
				p++;
				continue;
			}
			addrs.push_back(*p++);
		}
		free(ips);
	}
	
	return result;
}

// O T H E R  M E T H O D S _________________________________________________________________________

#pragma mark ________________________________start/stop

// Open driver and get local ip address
void CTCPSocket::TCPOpen()
{
	// Check whether already open
	if (mTCPState == TCPNotOpen)
	{

		cdmutex::lock_cdmutex _lock(sLock);
		if (!sDrvrCtr)
		{
			// Open driver if not already done
#ifdef _winsock
			WSADATA WsaData;
			int err = ::WSAStartup(TCP_WINSOCK_VERSION, &WsaData);

			// Check that network stack really loaded
			// Required if auto dial-up was cancelled
#if defined(_mac_winsock)
			// Check OpenTransport only
			if (COpenTransport::HasOpenTransport())
			{
				if (!COpenTransport::HasInet())
				{
					err = WSASYSNOTREADY;

					// Cleanup sockets to ensure reload on next open
					::WSACleanup();
				}
			}
#endif

			if (err)
			{
				CLOG_LOGTHROW(CTCPException, err);
				throw CTCPException(err);
			}

#ifdef _mac_winsock
			// Set up async DB handler
			if (!sAsyncHandler)
				sAsyncHandler = ::NMCreateMessageHandler(NewNMMessageCallbackProc(AsyncDnrCallBackProc), 0, 0);
#endif
#endif
		}

		// Increment driver usage counter
		sDrvrCtr++;
		mDrvrOpen = true;

		// No longer do this - the local name will be looked-up only when needed

		// Make sure local name is cached
		//if (sLocalName.empty())
		//	TCPGetLocalIPAddr();

		// Update status of TCP controller
		mTCPState = TCPOpenNoSocket;
	}

	TimerReset();
}

void CTCPSocket::TCPClose()
{
	if (mTCPState >= TCPOpenSocket)
		TCPAbort();
	else
	{

#ifdef _mac_winsock
			// Destroy async DB handler
		if (mAsyncHandler != NM_INVALID_HANDLER)
			::NMDestroyMessageHandler(mAsyncHandler, 0);
		mAsyncHandler = NM_INVALID_HANDLER;
#endif

		if (mDrvrOpen)
		{
			cdmutex::lock_cdmutex _lock(sLock);
			sDrvrCtr--;
			if (!sDrvrCtr)
			{
#ifdef _winsock
#ifdef _mac_winsock
				// Destroy async DB handler
				if (sAsyncHandler)
					::NMDestroyMessageHandler(sAsyncHandler, 0);
				sAsyncHandler = 0;
#endif
				// Remove any local name as it may be reassigned on next dialup
				cdmutex::lock_cdmutex _lock(sLockLocalName);
				sLocalName = cdstring::null_str;

				// Cleanup sockets
				::WSACleanup();
#else
				sLocalName = cdstring::null_str;
#endif
			}

			mDrvrOpen = false;
		}

		mTCPState = TCPNotOpen;
		mCancel = false;
		mAbort = false;
	}
}

void CTCPSocket::TCPHandleError(int err)
{
	switch(err)
	{
	case ENETDOWN:
	case ENETUNREACH:
	case ENETRESET:
	case ECONNABORTED:
	case ECONNRESET:
	case EHOSTDOWN:
	case EHOSTUNREACH:
		// Force connection checks
		CheckConnections(false);
		break;
	default:;
	}
}

// Get local ip address
void CTCPSocket::TCPGetLocalIPAddr()
{
	cdmutex::lock_cdmutex _lock(sLockLocalName);
	
	// Zero out at start
	sLocalName = cdstring::null_str;

	// Policy:

	// Try to get local host name from WinSock
	// This will attempt to do a DNS lookup for the local machine

#if defined(_mac_winsock)
	// Special behaviour for OpenTransport
	// This is designed to avoid the gethostname call on Mac OS which blocks,
	// and to use the async reverse lookup call instead
	if (COpenTransport::HasOpenTransport())
	{
		// Just get the ip number of this machine
		ip_addr ip = 0;
		int result = COpenTransport::gethostip(ip);
		if (result == RESULT_NO_ERR)
		{
			// Now do async lookup of self!
			cdstring str;
			TCPIPToName(ip, sLocalName);
			
			// If lookup failed, use IP address
			if (sLocalName.empty())
			{
				struct in_addr addr;
				addr.s_addr = ip;
				sLocalName = ::inet_ntoa(addr);
			}
			
			// Just return from this special case
			return;
		}
	}
#endif

	// Always use gethostname by default
	char name[256];
	int result = ::gethostname(name, 256);

	// If DNS lookup works then use result
	if (result != SOCKET_ERROR)
	{
#if !defined(_mac_winsock) && defined(_winsock)
		// Win32 may return with the dumb Windows hostname not the TCP/IP hostname
		
		// Need to lookup the name to get IP address and then lookup IP address to get hostname
#ifdef ASYNC_DB
		cdstring async_buffer;
		async_buffer.reserve(MAXGETHOSTSTRUCT);
		HOSTENT* host_ent = (HOSTENT*) async_buffer.c_str();
		if (sAsyncHandler)
		{
			bool started_async = false;
			{
				cdmutex::lock_cdmutex _lock(sLock);
				mAsyncHandle = ::WSAAsyncGetHostByName(sAsyncHandler, WM_TCP_ASYNCDNR, name, (char*) host_ent, MAXGETHOSTSTRUCT);
				started_async = (mAsyncHandle != NULL);

				// Get the error for the sync call here as exiting this block will wipe the last error
				if (!started_async)
				{
					mAsyncError = ::WSAGetLastError();
					host_ent = NULL;
				}
			}

			if (started_async)
			{
				// Must not fail
				try
				{
					// Begin a busy operation
					StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

					TCPAsyncBlock();
				}
				catch (CTCPException& ex)
				{
					CLOG_LOGCATCH(CTCPException&);

					if ((ex.error() >= WSAHOST_NOT_FOUND) && (ex.error() <= WSANO_DATA))
						host_ent = NULL;
					else
					{
						CLOG_LOGRETHROW;
						throw;
					}
				}
			}
		}
		else
			host_ent = NULL;
#else
		HOSTENT* host_ent = ::gethostbyname(name);
#endif
		if (host_ent)
		{
			struct in_addr sin_addr;
			sin_addr.s_addr = *(unsigned long*) host_ent->h_addr_list[0];
			TCPIPToName(sin_addr.s_addr, sLocalName);
		}
		else
			sLocalName = name;
#elif defined(_bsdsock)
		// Check to see if name is fully qualified
		if (*name && (::strchr(name, '.') == NULL))
		{
			HOSTENT* host_ent = ::gethostbyname(name);
			if (host_ent)
			{
				struct in_addr sin_addr;
				sin_addr.s_addr = *(unsigned long*) host_ent->h_addr_list[0];
				TCPIPToName(sin_addr.s_addr, sLocalName);
			}
			else
				sLocalName = name;
		}
		else
			sLocalName = name;

#else
		// Local hostname is whatever gethostname returned
		sLocalName = name;
#endif
	}

	// If DNS fails, Windows will return TCP/IP local name -> carry on as multihomed
	// If DNS fails, Mac OS will not return local name => attempt socket bind and extract IP address then do inverse lookup

	else
	{
		ip_addr ip = 0;
#if defined(_mac_winsock)
		// Special behaviour for OpenTransport
		if (COpenTransport::HasOpenTransport())
		{
			// Just get the ip number of this machine
			result = COpenTransport::gethostip(ip);
			if (result != RESULT_NO_ERR)
				ip = 0;
		}
		
		if (!ip)
#endif
		{
			// Create a temporary socket
			int sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (sock != INVALID_SOCKET)
			{
				struct sockaddr_in sa;
				sa.sin_family = AF_INET;
				sa.sin_port = htons(0);
				sa.sin_addr.s_addr = htonl(INADDR_ANY);

				// Bind to local port to get address
				int result = ::bind(sock, (struct sockaddr *)&sa, sizeof(sa));
				if (result != SOCKET_ERROR)
				{
					// Get its name
					socklen_t len = sizeof(sa);
					result = ::getsockname(sock, (sockaddr *)&sa, &len);
					if (result != SOCKET_ERROR)
						ip = sa.sin_addr.s_addr;
				}
				
				// Now close socket
#ifdef _winsock
			::closesocket(sock);
#else
			::close(sock);
#endif
			}
		}

		// Just convert IP direct to name
		if (ip)
		{
			struct in_addr addr;
			addr.s_addr = ip;
			sLocalName = ::inet_ntoa(addr);
		}
	}
}

// Get the bets guess as to the local host name
const cdstring& CTCPSocket::TCPGetLocalHostName()
{
	// Only do if local name is empty and stack is loaded
	cdmutex::lock_cdmutex _lock(sLockLocalName);

	if (sLocalName.empty() && sDrvrCtr)
	{
		// Create a dummy socket and get the local name
		CTCPSocket temp;
		temp.TCPOpen();
		temp.TCPGetLocalIPAddr();
	}

	return sLocalName;
}

// Get the name of an open socket
const cdstring& CTCPSocket::TCPGetSocketName()
{
	// See if already specified
	if (!mLocalName.empty())
		return mLocalName;
	
	// See if connected
	if (TCPGetState() == TCPConnected)
	{
		// Get its name
		struct sockaddr_in sa;
		socklen_t len = sizeof(sa);
		int result = ::getsockname(mSocket, (sockaddr *)&sa, &len);
		if (result != SOCKET_ERROR)
			// Just convert IP direct to name
			TCPIPToName(sa.sin_addr.s_addr, mLocalName);
	}
	
	// Check that we found something
	if (mLocalName.empty())
		// Fall back to static method
		return TCPGetLocalHostName();
	else
	{
		// If the global local name is empty, then reset it to this one as we know it is valid
		{
			cdmutex::lock_cdmutex _lock(sLockLocalName);

			if (sLocalName.empty())
				sLocalName = mLocalName;
		}
		
		// Return what we found
		return mLocalName;
	}
}

ip_addr CTCPSocket::TCPGetLocalConnectedIP() const
{
	ip_addr ip_result = 0;

	// Must be connected
	if (TCPGetState() == TCPConnected)
	{
		// Get its name
		struct sockaddr_in sa;
		socklen_t len = sizeof(sa);
		int result = ::getsockname(mSocket, (sockaddr *)&sa, &len);
		if (result != SOCKET_ERROR)
			ip_result = sa.sin_addr.s_addr;
	}
	
	return ip_result;
}

// S T R E A M  R E L A T E D _________________________________________________________________________

#pragma mark ________________________________streams

// Create a stream ready for comms

void CTCPSocket::TCPCreateSocket()
{
	// Is it already created?
	if (mTCPState == TCPOpenSocket) return;

	try
	{
		struct sockaddr_in sa;
		unsigned long arg;
		int optVal;

		// Create socket
#ifdef _mac_winsock
		mSocket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
		mSocket = ::socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
#endif
		if (mSocket == INVALID_SOCKET)
		{
			CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPFailed);
			throw CTCPException(CTCPException::err_TCPFailed);
		}

		// Set socket options
		int result;

		// can talk to same address/same port
		optVal = true;
		result = ::setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, (char*) &optVal, sizeof(optVal));
		if (result == SOCKET_ERROR)
		{
			// Check for error
			int err = tcp_errno;

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

		// do graceful close
		
#if defined(_bsdsock)
		struct linger lingerOptVal = {0, 0};
		result = ::setsockopt(mSocket, SOL_SOCKET, SO_LINGER, &lingerOptVal, sizeof(lingerOptVal));
#else
		optVal = true;
		result = ::setsockopt(mSocket, SOL_SOCKET, SO_DONTLINGER, (char*) &optVal, sizeof(optVal));
#endif
		if (result == SOCKET_ERROR)
		{
			// Check for error
			int err = tcp_errno;

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

		// send using "push" flag
//		optVal = true;
//		result = ::setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (char*) &optVal, sizeof(optVal));
//		if (result == SOCKET_ERROR)
//		{
//			// Check for error
//			int err = tcp_errno;
//
//			// Got an error => throw
//			CLOG_LOGTHROW(CTCPException, err);
//			throw CTCPException(err);
//		}

		// don't receive urgent data separately
		optVal = true;
		result = ::setsockopt(mSocket, SOL_SOCKET, SO_OOBINLINE, (char*) &optVal, sizeof(optVal));
		if (result == SOCKET_ERROR)
		{
			// Check for error
			int err = tcp_errno;

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

#ifdef _winsock
#ifdef _mac_winsock
		if (mAsyncHandler == NM_INVALID_HANDLER)
			mAsyncHandler = ::NMCreateMessageHandler(NewNMMessageCallbackProc(AsyncSelectCallBackProc), this, 0);
		if (mAsyncHandler != NM_INVALID_HANDLER)
#else
		if (mAsyncHandler)
#endif
		{
			result = ::WSAAsyncSelect(mSocket, mAsyncHandler, WM_TCP_ASYNCSELECT, FD_CONNECT);
			if (result != RESULT_NO_ERR)
			{
				// Check for error
				int err = tcp_errno;

				// Got an error => throw
				CLOG_LOGTHROW(CTCPException, err);
				throw CTCPException(err);
			}
		}
		else
		{
			int err = ETOOMANYREFS;

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}
#endif

		// make sure the socket is in async mode (non-blocking)
		arg = true;
#ifdef _winsock
		result = ::ioctlsocket(mSocket, FIONBIO, &arg);
#else
		//unix sockets are nonblocking by default
		result = ::ioctl(mSocket, FIONBIO, (long*) &arg);
#endif
		if (result == SOCKET_ERROR)
		{
			// Check for error
			int err = tcp_errno;

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

		// Bind socket
		sa.sin_family = AF_INET;
		sa.sin_port = htons(mLocalPort);
		sa.sin_addr.s_addr = htonl(INADDR_ANY);

		result = ::bind(mSocket, (struct sockaddr *) &sa, sizeof(sa));
		if (result == SOCKET_ERROR)
		{
			// Check for error
			int err = tcp_errno;

			// Got an error => throw
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

		// Update status if no errors
		mTCPState = TCPOpenSocket;
	}
	catch (CTCPException&)
	{
		CLOG_LOGCATCH(CTCPException&);

		if (mSocket != INVALID_SOCKET)
			TCPAbort();

		// Update status if no errors
		mTCPState = TCPOpenNoSocket;

		CLOG_LOGRETHROW;
		throw;
	}
}

// U T I L I T Y _____________________________________________________________________________________

#pragma mark ________________________________background tasks

// Allow other tasks to function
void CTCPSocket::TCPForceAbort()
{
	TCPAbort(true);

	CLOG_LOGTHROW(CTCPException, CTCPException::err_TCPAbort);
	throw CTCPException(CTCPException::err_TCPAbort);
}

// Allow other tasks to function
void CTCPSocket::TCPYield()
{
#ifdef _mac_winsock
	// Give time to message handler
	::NMIdle();
#endif

	// Allow app time
	mCancel = CMailControl::ProcessBusy(&mBusy);

	// Check for user/program requested abort
	if (mCancel || mAbort)
	{
		mCancel = false;
		TCPForceAbort();
	}
}

// Allow other tasks to function while selecting a socket
void CTCPSocket::TCPSelectYield(bool read, unsigned long secs)
{
	// Flag to indicate check of connection status required if delay occurs
	bool do_delay_check = true;

	// Start timer
	time_t tstart = ::time(NULL);

	// Wait for read/write
	while(true)
	{
		fd_set set_rw;
		FD_ZERO(&set_rw);
		FD_SET(mSocket, &set_rw);
		fd_set set_error;
		FD_ZERO(&set_error);
		FD_SET(mSocket, &set_error);

		// Wait up to 1 ms
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000;

#ifdef _winsock
		int result = ::select(FD_SETSIZE, read ? &set_rw : NULL, read ? NULL : &set_rw, &set_error, &timeout);
#else
		int result = ::select(mSocket+1, read ? &set_rw : NULL, read ? NULL : &set_rw, &set_error, &timeout);
#endif

		// See if select failed or socket signalled as failed
		if ((result == -1) || FD_ISSET(mSocket, &set_error))
		{
			// Get the socket error
			int err;

			// Check for select fail or socket fail
			if (result == -1)
				err = tcp_errno;
			else
			{
#ifdef _bsdsock
				socklen_t err_len = sizeof(int);
#else
				int err_len = sizeof(int);
#endif
				::getsockopt(mSocket, SOL_SOCKET, SO_ERROR, (char*) &err, &err_len);
			}

			// Force socket closed (silent error)
			TCPAbort(true);
			
			TCPHandleError(err);

			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

		// See if socket signalled as completed and exit loop
		if (FD_ISSET(mSocket, &set_rw))
			break;

		// Do standard UI yield
		TCPYield();
		
		// Check for timeout
		double time_diff = ::difftime(::time(NULL), tstart);
		if (time_diff > secs)
		{
			CLOG_LOGTHROW(CTCPTimeoutException, secs);
			throw CTCPTimeoutException();
		}
		else if (do_delay_check && (time_diff > 5))
		{
			CheckConnections(false);
			do_delay_check = false;
		}
	}
}

#pragma mark ________________________________name lookup

// Lookup name with DNR
void CTCPSocket::TCPNameToIP(const cdstring& name, ip_addr* ip, const cdstring& cname)
{
	// Clear ip first
	*ip = 0;

	// Look for localhost
	if (name.compare("localhost", true) == 0)
		// Just convert string for ip address
		*ip = ::inet_addr("127.0.0.1");
		
	// Check for IP or cname
	else if (TCPIsHostName(name))
	{
		// Try to look up name
#ifdef ASYNC_DB
		cdstring async_buffer;
		async_buffer.reserve(MAXGETHOSTSTRUCT);
		HOSTENT* host_ent = (HOSTENT*) async_buffer.c_str();
		if (sAsyncHandler)
		{
			bool started_async = false;
			{
				cdmutex::lock_cdmutex _lock(sLock);
				char* ipname = const_cast<cdstring&>(name).c_str_mod();
				mAsyncHandle = ::WSAAsyncGetHostByName(sAsyncHandler, WM_TCP_ASYNCDNR, ipname, (char*) host_ent, MAXGETHOSTSTRUCT);
				started_async = (mAsyncHandle != NULL);

				// Get the error for the sync call here as exiting this block wipes the last error
				if (!started_async)
				{
					mAsyncError = ::WSAGetLastError();
					host_ent = NULL;
				}
			}

			// Check to see whether async operation is in progress
			if (started_async)
			{
				// Must not fail
				try
				{
					// Begin a busy operation
					StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

					TCPAsyncBlock();
				}
				catch (CTCPException& ex)
				{
					CLOG_LOGCATCH(CTCPException&);

					if ((ex.error() >= WSAHOST_NOT_FOUND) && (ex.error() <= WSANO_DATA))
						host_ent = NULL;
					else
					{
						CLOG_LOGRETHROW;
						throw;
					}
				}
			}
		}
		else
			host_ent = nil;
#else
		HOSTENT* host_ent = ::gethostbyname(const_cast<char*>(name.c_str()));
#endif

		// Was it found
		if (!host_ent)
		{
#ifdef ASYNC_DB
			int err = mAsyncError;
#else
			int err = dns_errno;
			if (err == HOST_NOT_FOUND)
				err = 11001;
			else if (err == NO_DATA)
				err = 11002;
			else if (err == NO_RECOVERY)
				err = 11003;
			else if (err == TRY_AGAIN)
				err = 11004;
#endif
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}

		// Get IP details
		*ip = *(ip_addr*) host_ent->h_addr;

		// Get cname details
		const_cast<cdstring&>(cname) = host_ent->h_name;
	}
	else
		// Just convert string
		*ip = ::inet_addr(const_cast<cdstring&>(name).c_str_mod());

}

// Lookup name with DNR
// Assume ip is in network byte order
void CTCPSocket::TCPIPToName(ip_addr ip, cdstring& name)
{

	HOSTENT* host_ent = NULL;
#ifdef ASYNC_DB
	cdstring async_buffer;
#endif
	bool no_reverse_lookup = false;

	// Special for private address ranges (NAT)

	// 10.0.0.0 -> 10.255.255.255
	if ((ntohl(ip) & 0xFF000000) == 0x0A000000)
	{
		no_reverse_lookup = true;
	}

	// 172.16.0.0 -> 172.31.255.255
	else if (((ntohl(ip) & 0xFF000000) == 0xAC000000) &&
			 ((ntohl(ip) & 0x00FF0000) >= 0x00100000) &&
			 ((ntohl(ip) & 0x00FF0000) <= 0x001F0000))
			no_reverse_lookup = true;
	
	// 192.168.0.0 -> 192.168.255.255
	else if (((ntohl(ip) & 0xFF000000) == 0xC0000000) &&
			 ((ntohl(ip) & 0x00FF0000) == 0x00A80000))
		no_reverse_lookup = true;

	if (!no_reverse_lookup)
	{
	// Look it up
#ifdef ASYNC_DB
		async_buffer.reserve(MAXGETHOSTSTRUCT);
		host_ent = (HOSTENT*) async_buffer.c_str();
		if (sAsyncHandler)
		{
			bool started_async = false;
			{
				cdmutex::lock_cdmutex _lock(sLock);
				mAsyncHandle = ::WSAAsyncGetHostByAddr(sAsyncHandler, WM_TCP_ASYNCDNR, reinterpret_cast<char*>(&ip), sizeof(ip_addr), PF_INET, (char*) host_ent, MAXGETHOSTSTRUCT);
				started_async = (mAsyncHandle != NULL);

				// Check error here as exiting this block wipes the last error
				if (!started_async)
				{
					mAsyncError = ::WSAGetLastError();
					host_ent = NULL;
				}
			}

			if (started_async)
			{
				// Must not fail
				try
				{
					// Begin a busy operation
					StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

					TCPAsyncBlock();
				}
				catch (CTCPException& ex)
				{
					CLOG_LOGCATCH(CTCPException&);

					if ((ex.error() >= WSAHOST_NOT_FOUND) && (ex.error() <= WSANO_DATA))
						host_ent = NULL;
					else
					{
						CLOG_LOGRETHROW;
						throw;
					}
				}
			}
		}
		else
			host_ent = NULL;
#else
		host_ent = ::gethostbyaddr(reinterpret_cast<char*>(&ip), sizeof(ip_addr), PF_INET);
#endif
	}

	// Was it found
	if (!host_ent)
	{
		// Just convert IP direct to name
		struct in_addr test;
		test.s_addr = ip;
		name = ::inet_ntoa(test);
	}
	else
	{
		// Get looked up name
		name = host_ent->h_name;

		// Strip any trailing period
		if (name.length() && (name[name.length() - 1] == '.'))
			name[name.length() - 1] = '\0';
	}
}

// Look for valid host name, not IP number
bool CTCPSocket::TCPIsHostName(const cdstring& test)
{
	// Check for IP or cname
	const char* p = test;
	while(*p)
	{
		if (((*p < '0') || (*p > '9')) && (*p != '.'))
			return true;

		p++;
	}

	return false;
}

// R E M O T E  A D D R E S S  R E L A T E D _____________________________________________________________________________________

// Specify remote ip & port
void CTCPSocket::TCPSpecifyRemoteIP(ip_addr remote_ip, tcp_port remote_port, bool get_cname)
{
	// Cache IP details
	::memset((char *) &mRemote, 0, sizeof(mRemote));
	mRemote.sin_family = AF_INET;
	mRemote.sin_addr.s_addr = remote_ip;
	mRemote.sin_port = htons(remote_port);

	// Do inverse lookup to get name
	if (get_cname)
	{
		TCPIPToName(remote_ip, mRemoteName);

		// Canonical the same as name
		mRemoteCName = mRemoteName;
	}

}

// Specify remote host name (look it up) and port
void CTCPSocket::TCPSpecifyRemoteName(const cdstring& remote_name, tcp_port remote_port, bool get_cname)
{
	mOriginalName = remote_name;

	ip_addr remote_ip;

	// Look for port # appended to server
	cdstring rname = remote_name;
	cdstring cname;
	tcp_port rport = remote_port;
	if (::strchr(rname.c_str(), ':'))
	{
		rname = cdstring(remote_name, 0, ::strcspn(remote_name, ":"));
		const char* num = ::strchr(remote_name, ':');
		num++;
		rport = ::atoi(num);
	}

	// Lookup name
	TCPNameToIP(rname, &remote_ip, cname);

#if 0
	// Check for IP or cname
	if ((rname[0] >= '0') && (rname[0] <= '9'))
		// Leave get-cname (reverse lookup)
		;
	else
	{
		// Modified 31-Mar-1999: Always do explicit reverse lookup to get actual
		// machine name as CNAME may not exist in some DNS records

		// Force get_cname off and manually set
		//get_cname = false;
		mRemoteName = rname;
		//mRemoteCName = cname;
	}
#endif
	// Always set the remote name based on the returned rname
	mRemoteName = rname;

	// Set remote details if no error
	TCPSpecifyRemoteIP(remote_ip, rport, get_cname);

	// Tag port # to descriptor if different from default
	if (remote_port != rport)
		mRemoteName += cdstring(':') + cdstring((long) rport);

	// Always reset local name if remote has been set
	mLocalName = cdstring::null_str;
}

#ifdef _winsock

#ifdef _mac_winsock
pascal u_long CTCPSocket::AsyncDnrCallBackProc(NMMessageHandler h, u_int wMsg, u_long wParam, u_long lParam)
{
	// Get error code and do standard processing
	long errCode = WSAGETASYNCERROR(lParam);
	TCPProcessAsyncDNRCompletion(wParam, errCode);

	return 0;
}

pascal u_long CTCPSocket::AsyncSelectCallBackProc(NMMessageHandler h, u_int wMsg, u_long wParam, u_long lParam)
{
	// Get error code and do standard processing
	long evtCode = WSAGETSELECTEVENT(lParam);
	long errCode = WSAGETSELECTERROR(lParam);
	CTCPSocket*	socket	= (CTCPSocket*) ::NMGetMessageHandlerRefCon(h);
	socket->TCPAsyncDone(errCode);

	return 0;
}
#endif

// Async DNR operation completed
void CTCPSocket::TCPProcessAsyncDNRCompletion(HANDLE hdl, long err)
{
	// Find socket that matches this handle
	cdmutex::lock_cdmutex _lock(sLock);
	for(CTCPSocketList::iterator iter = sSockets.begin(); iter != sSockets.end(); iter++)
	{
		if ((*iter)->TCPGetAsyncHandle() == hdl)
		{
			(*iter)->TCPAsyncDone(err);
			break;
		}
	}
}

// Async Select operation completed
void CTCPSocket::TCPProcessAsyncSelectCompletion(int socket, long err)
{
	// Find socket that matches this handle
	cdmutex::lock_cdmutex _lock(sLock);
	for(CTCPSocketList::iterator iter = sSockets.begin(); iter != sSockets.end(); iter++)
	{
		if ((*iter)->mSocket == socket)
		{
			(*iter)->TCPAsyncDone(err);
			break;
		}
	}
}

// Async operation complete
void CTCPSocket::TCPAsyncDone(long err)
{
	// Just set HANDLE to nil as it serves as completion flag!
	TCPSetAsyncHandle(NULL);
	mAsyncSelect = false;

	// Cache error value
	mAsyncError = err;
}

// Block until async op completes or user cancels
void CTCPSocket::TCPAsyncBlock(unsigned long secs)
{
	// Get start time for timeout calc
	time_t tstart = ::time(NULL);

	// Yield and wait for HANDLE or flag to become nil
	do
	{
		TCPYield();
		
		// Check for timeout
		if (::time(NULL) - tstart > secs)
		{
			CLOG_LOGTHROW(CTCPTimeoutException, secs);
			throw CTCPTimeoutException();
		}

	} while(TCPGetAsyncHandle() || mAsyncSelect);

	// Throw error
	if (mAsyncError != noErr)
	{
		CLOG_LOGTHROW(CTCPException, mAsyncError);
		throw CTCPException(mAsyncError);
	}
}

#endif // #ifdef _winsock

// C O N N E C T I O N S ____________________________________________________________________________

#pragma mark ________________________________connections

// Wait for an incoming connection from remote host
void CTCPSocket::TCPWaitConnection()
{
	mTCPState = TCPWaitingConn;

	// May need to create socket
	if (mTCPState == TCPOpenNoSocket)
	{
		// Create a stream for comms
		TCPCreateSocket();

		// Update status of TCP controller
		mTCPState = TCPOpenSocket;
	}

	// listen here

	// Set to new state
	mTCPState = TCPConnected;
}

// Initiate a connection with remote host
void CTCPSocket::TCPStartConnection()
{
	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

	unsigned long ctr = sConnectRetryMaxCount;
	bool complete = false;
	while(!complete && ctr)
	{
		// May need to create socket
		if (mTCPState == TCPOpenNoSocket)
		{
			// Create a stream for comms
			TCPCreateSocket();

			// Update status of TCP controller
			mTCPState = TCPOpenSocket;
		}
		
#ifdef _winsock
		// Uses WSAAsyncSelect
		mAsyncSelect = true;
#endif

		// Do active open call
		int result = ::connect(mSocket, (struct sockaddr *) &mRemote, sizeof(mRemote));

		// Check for failure
		if (result == SOCKET_ERROR)
		{
			int err = tcp_errno;

			// Check for err - (allow block to complete in its own time)
#ifdef _winsock
			if (err != EWOULDBLOCK)
#else
			if (err != EINPROGRESS)
#endif
			{
				// Connection failure => redo DNS lookup next time through
				mDescriptor = cdstring::null_str;

				// Force socket closed (silent error)
				TCPAbort(true);
				TCPHandleError(err);
				CLOG_LOGTHROW(CTCPException, err);
				throw CTCPException(err);
			}

#ifdef _winsock
			try
			{
				// Uses WSAAsyncSelect with a timeout for retries
				TCPAsyncBlock(sConnectRetryTimeout);
				
				// If done break out of loop
				complete = true;;
			}
			catch (CTCPTimeoutException&)
			{
				CLOG_LOGCATCH(CTCPTimeoutException&);
				
				// We must abort (silently without full close) the current connect operation, and loop back to try it again.
				TCPAbort(true, false);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Connection failure => redo DNS lookup next time through
				mDescriptor = cdstring::null_str;

				// Clean up the connection
				TCPAbort(true);

				CLOG_LOGRETHROW;
				throw;
			}
#else
			// Wait for connection to complete
			try
			{
				TCPSelectYield(false, sConnectRetryTimeout);
				
				// If done break out of loop
				complete = true;;
			}
			catch (CTCPTimeoutException& ex)
			{
				CLOG_LOGCATCH(CTCPTimeoutException&);
				
				// We must abort (silently without full close) the current connect operation, and loop back to try it again.
				TCPAbort(true, false);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Connection failure => redo DNS lookup next time through
				mDescriptor = cdstring::null_str;

				CLOG_LOGRETHROW;
				throw;
			}
#endif
		}
		else
			// connect worked - exit loop
			break;
		
		// Reduce loop count
		ctr--;
	}

	// Force exception when retry attempts timeout
	if (ctr == 0)
	{
		CLOG_LOGTHROW(CTCPException, ETIMEDOUT);
		throw CTCPException(ETIMEDOUT);
	}

	// Set to new state
	mTCPState = TCPConnected;
}

// Close connection
void CTCPSocket::TCPCloseConnection()
{
	if (mTCPState == TCPConnected)
	{
		struct linger aLinger;

		aLinger.l_onoff  = false;						// dont lingerÉ
		aLinger.l_linger = cTCPDefaultTimeout;			// Éfor a graceful close
		int result = ::setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char *)&aLinger, sizeof(aLinger));

#ifdef _winsock
		result = ::closesocket(mSocket);
#else
		result = ::close(mSocket);
#endif
		// Check for failure
		if (result == SOCKET_ERROR)
		{
			int err = tcp_errno;

			// Check for err - (allow block to complete in its own time)
			if (err != EWOULDBLOCK)
			{
				CLOG_LOGTHROW(CTCPException, err);
				throw CTCPException(err);
			}
		}

	}

	// Socket now dead
	mSocket = INVALID_SOCKET;

	// Set to new state
	mTCPState = TCPOpenNoSocket;

	// Now close down fully
	TCPClose();
}

// Abort connection
void CTCPSocket::TCPAbort(bool silent, bool full_close)
{
	// Look for async operation
#ifdef _winsock
	if (TCPGetAsyncHandle())
	{
		::WSACancelAsyncRequest(TCPGetAsyncHandle());
		TCPSetAsyncHandle(NULL);
		return;
	}

	// Cancel async select operation
	::WSAAsyncSelect(mSocket, mAsyncHandler, 0, 0);
	mAsyncSelect = false;
#endif

	// Look for ordinary abort
	if ((mTCPState == TCPOpenSocket) || (mTCPState == TCPConnected))
	{
		struct linger aLinger;

		aLinger.l_onoff  = false;						// dont lingerÉ
		aLinger.l_linger = 0;							// force close immediately
		int result = ::setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char *)&aLinger, sizeof(aLinger));

#ifdef _winsock
		result = ::closesocket(mSocket);
#else
		result = ::close(mSocket);
#endif

		// Check for failure
		if (result == SOCKET_ERROR)
		{
			int err = tcp_errno;

			// Socket now dead
			mSocket = INVALID_SOCKET;

			// Set to new state
			mTCPState = TCPOpenNoSocket;

			// Now do full close down
			if (full_close)
				TCPClose();

			// Check for error
			if (!silent)
			{
				CLOG_LOGTHROW(CTCPException, err);
				throw CTCPException(err);
			}
		}
	}

	// Socket now dead
	mSocket = INVALID_SOCKET;

	// Set to new state
	mTCPState = TCPOpenNoSocket;

	// Now close down fully
	if (full_close)
		TCPClose();
}


// R E C E I V I N G  D A T A _____________________________________________________________________________________

#pragma mark ________________________________receiving data

// Receive data
void CTCPSocket::TCPReceiveData(char* buf, long* len)
{
	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

	// Yield before to get any user abort
	TCPYield();
    
    while (!_ReceiveData(buf, len)) {
        // Select yield while waiting to unblock
        TCPSelectYield(true);
    }
    
	// Reset tickle timer
	TimerReset();
}

// Receive data
bool CTCPSocket::_ReceiveData(char* buf, long* len)
{
	// Get any data present at the moment
	int result;
	while((result = ::recv(mSocket, buf, *len, 0)) == SOCKET_ERROR)
	{
		int err = tcp_errno;

		// Check for failure
		if (err == EWOULDBLOCK)
            return false;
		else
		{
			TCPAbort(true);

			TCPHandleError(err);

			// Throw error
			CLOG_LOGTHROW(CTCPException, err);
			throw CTCPException(err);
		}
	}

	// Check for broken connection
	if (!result)
	{
		TCPAbort(true);

		int err = ECONNRESET;

		// Got an error => throw
		CLOG_LOGTHROW(CTCPException, err);
		throw CTCPException(err);
	}

	*len = result;
    return true;
}

// S E N D I N G  D A T A  _____________________________________________________________________________________

#pragma mark ________________________________sending data

// Send data
void CTCPSocket::TCPSendData(char* buf, long len)
{
	// Begin a busy operation
	StMailBusy busy_lock(&mBusy, &GetBusyDescriptor());

	// Yield before to get any user abort
	TCPYield();
    
    long result = 0;
    while ((result = _SendData(buf, len)) != 0) {
        // Select yield while waiting to unblock
        TCPSelectYield(false);
        
        buf += result;
        len += result;
    }
    
	// Reset tickle timer
	TimerReset();
}

// Send data
long CTCPSocket::_SendData(char* buf, long len)
{
	// Send data in blocks of buffer size
	char* p = buf;
	while(len)
	{
		int result = ::send(mSocket, p, len, 0);

		// Check for failure
		if (result == SOCKET_ERROR)
		{
			int err = tcp_errno;

			// Check type of error
			if (err == EWOULDBLOCK)

				// Select yield while waiting to unblock
				return len;

			else
			{
				TCPAbort(true);

				TCPHandleError(err);

				// Throw error
				CLOG_LOGTHROW(CTCPException, err);
				throw CTCPException(err);
			}
		}
		else
		{
			// Adjust buffer
			len -= result;
			p += result;
		}
	}
    
    return 0;
}
