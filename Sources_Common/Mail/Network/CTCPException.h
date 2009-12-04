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

#ifndef __CTCPEXCEPTION__MULBERRY__
#define __CTCPEXCEPTION__MULBERRY__

#include "CNetworkException.h"

class CTCPException : public CNetworkException
{
public:
	enum { class_ID = 'tcp ' };

	enum ETCPException
	{
		err_TCPFailed = 'TCPF',
		err_TCPAbort = 'TCPA',
		err_TCPNoSSLPlugin = 'TCPP',
		err_TCPSSLError = 'TCPS',
		err_TCPSSLCertError = 'TCPC',
		err_TCPSSLCertNoAccept = 'TCPN',
		err_TCPSSLClientCertLoad = 'TCPD'
	};

	// Only allow construction from the error codes we know about
	CTCPException(int err_code) : CNetworkException(err_code)
		{ _class = class_ID; }
};

class CTCPTimeoutException : public CTCPException
{
public:
	CTCPTimeoutException() : CTCPException(CTCPException::err_TCPAbort) {}
};
	
#endif
