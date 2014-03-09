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

#ifndef __CTCPSOCKET__MULBERRY__
#define __CTCPSOCKET__MULBERRY__

#include "cdmutex.h"
#include "cdstring.h"
#include <vector>
#include <time.h>

#include "CMailControl.h"

#if __dest_os == __mac_os
#define _winsock
#define _mac_winsock
#elif __dest_os == __win32_os
#define _winsock
#elif __dest_os == __linux_os || __dest_os == __mac_os_x
#define _bsdsock
#else
#error __dest_os
#endif

#ifdef _winsock
#include <winsock.h>
#endif

#ifdef _bsdsock
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

const long cTCPBufferSize = 8192L;

typedef sockaddr_in AddrInfo;
typedef unsigned short tcp_port;
typedef unsigned long ip_addr;

class CTCPSocket;
typedef std::vector<CTCPSocket*> CTCPSocketList;
typedef std::vector<ip_addr> CTCPAddrList;

class CTCPSocket
{
public:
	enum TCPState
	{
		TCPNotOpen,
		TCPOpenNoSocket,
		TCPOpenSocket,
		TCPWaitingConn,
		TCPStartingConn,
		TCPClosingConn,
		TCPConnected
	};

	// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

			CTCPSocket();
			CTCPSocket(const CTCPSocket& copy);
	virtual ~CTCPSocket();

	// S T A T I C S
	static bool	HasInet();
	static bool WillDial();
	static bool HasModem();
	static bool AutoDial();
	static bool AutoHangup();
	static unsigned long CountConnected();
	static bool CheckConnectionState();
	static bool CheckConnections(bool check_only);
	static bool GetInterfaceIPs(CTCPAddrList& addrs, bool include_localhost = true);

	static void SetTimeouts(unsigned long connectRetryTimeout, unsigned long connectRetryMaxCount)
		{ sConnectRetryTimeout = connectRetryTimeout; sConnectRetryMaxCount = connectRetryMaxCount; }

	// O T H E R  M E T H O D S

	virtual void TCPOpen();
	virtual void TCPClose();

	virtual void TCPGetLocalIPAddr();
	static const cdstring& TCPGetLocalHostName();
	const cdstring& TCPGetSocketName();
	ip_addr TCPGetLocalConnectedIP() const;
	virtual void TCPSetLocalPort(tcp_port local_port)
				{ mLocalPort = local_port; }

	virtual void SetDescriptor(const cdstring& desc)
		{ mDescriptor = desc; }
	virtual const cdstring& GetDescriptor() const
		{ return mDescriptor; }

	virtual void SetBusyDescriptor(const cdstring& desc)
		{ mBusyDescriptor = desc; }
	virtual const cdstring& GetBusyDescriptor() const
		{ return mBusyDescriptor; }

	// Utility
	virtual void TimerReset() { mLastClock = ::time(NULL); }		// Reset timer
	virtual time_t GetTimer() const { return mLastClock; }		// Get timer

	virtual void SetAbort()
		{ mAbort = true; }
	virtual void TCPHandleError(int err);

	virtual void TCPNameToIP(const cdstring& name, ip_addr* ip, const cdstring& cname);
	virtual void TCPIPToName(ip_addr ip, cdstring& name);

	virtual TCPState TCPGetState() const
		{ return mTCPState; }

	static bool TCPIsHostName(const cdstring& test);			// Look for valid host name, not IP number

	// Remote address related
	virtual void TCPSpecifyRemoteIP(ip_addr remote_ip, tcp_port remote_port, bool get_cname);
	virtual void TCPSpecifyRemoteName(const cdstring& remote_name, tcp_port remote_port, bool get_cname);
	virtual const cdstring& GetRemoteName() const
		{ return mRemoteName; }
	virtual const cdstring& GetRemoteCName() const
		{ return mRemoteCName; }

	// Async DB operations
#ifdef _winsock
	static void	TCPProcessAsyncDNRCompletion(HANDLE hdl, long err);		// Async DNR operation completed
	static void	TCPProcessAsyncSelectCompletion(int socket, long err);	// Async select operation completed
#ifndef _mac_winsock
	static void TCPSetAsyncHandler(HWND hwnd)							// Set window to handle all async operations
		{ sAsyncHandler = hwnd; }
#endif
	virtual HANDLE TCPGetAsyncHandle() const							// Return HANDLE associated with current async operation
		{ return mAsyncHandle; }
	virtual void TCPSetAsyncHandle(HANDLE hdl)							// Set HANDLE associated with async operation
		{ mAsyncHandle = hdl; }
	virtual void TCPAsyncDone(long err);								// Async operation complete
	virtual void TCPAsyncBlock(unsigned long secs = -1);		// Block until async op completes or user cancels or timeout
#endif

	// Connections
	virtual void TCPWaitConnection();						// Passive connect (listen)

	virtual void TCPStartConnection();						// Active connect

	virtual void TCPCloseConnection();						// Close connect

	virtual void TCPAbort(bool silent = false,				// Abort connection
							bool full_close = true);

	// Receiving data
	virtual void TCPReceiveData(char* buf, long* len);			// Receive some data

	// Sending data
	virtual void TCPSendData(char* buf, long len);				// Send data

protected:
	static unsigned long		sConectTimeout;		// Timeout for 

	static unsigned long		sConnectRetryTimeout;				// Retry connect after this time
	static unsigned long		sConnectRetryMaxCount;				// Max. no. of connect retries before failure

	static long					sDrvrCtr;			// WinSock driver open counter
	bool						mDrvrOpen;			// This object has aquired use of driver
	static cdstring				sLocalName;			// Name of local host
	cdstring					mLocalName;			// Name of local host derived from connected socket
	cdstring					mDescriptor;		// Used to identify who socket is connecting to
	tcp_port					mLocalPort;			// Local port assigned
	AddrInfo					mRemote;			// Remote IP details
	cdstring					mOriginalName;		// Host name entered by user
	cdstring					mRemoteName;		// Remote host name (maybe alias)
	cdstring					mRemoteCName;		// Remote host canonical name

	int							mSocket;			// Socket

	TCPState					mTCPState;			// Status
	bool						mCancel;			// User cancel flag
	bool						mAbort;				// Program abort flag
	time_t						mLastClock;			// Last value of clock after call

	CBusyContext				mBusy;				// Busy details
	cdstring					mBusyDescriptor;	// Busy descriptor

	// Async database ops
#ifdef _winsock
	HANDLE						mAsyncHandle;		// Task in progress
	bool						mAsyncSelect;		// Select task in progress
	long						mAsyncError;		// Error result from async call
#ifdef _mac_winsock
	static NMMessageHandler		sAsyncHandler;		// Async database op handler
	static pascal u_long		AsyncDnrCallBackProc(NMMessageHandler h, u_int wMsg, u_long wParam, u_long lParam);
	NMMessageHandler			mAsyncHandler;
	static pascal u_long		AsyncSelectCallBackProc(NMMessageHandler h, u_int wMsg, u_long wParam, u_long lParam);
#else
	static HWND					sAsyncHandler;		// Async database op handler
	HWND						mAsyncHandler;		// Async database op handler
#endif
#endif
	static CTCPSocketList		sSockets;			// List of all available sockets
	static cdmutex				sLock;				// mutex for access to static members
	static cdmutex				sLockLocalName;		// mutex for access to sLocalName

			void TCPCreateSocket();

    virtual bool _ReceiveData(char* buf, long* len);			// Receive some data directly
	virtual long _SendData(char* buf, long len);				// Send data

			// Yielding
			void TCPYield();
			void TCPSelectYield(bool read, unsigned long secs = -1);

			// Errors
			void TCPForceAbort();
};

#endif
