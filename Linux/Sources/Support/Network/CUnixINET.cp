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


// Source for CUnixINET class

#include "CUnixINET.h"

#include "CConnectionManager.h"

#include "cdfstream.h"

#include <JSimpleProcess.h>
#include <JString.h>
#include <JThisProcess.h>
#include <jFileUtil.h>
#include <jDirUtil.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

const char* cDialScript = ".auto_dial";

JSimpleProcess*	CUnixINET::sDialer = NULL;

bool CUnixINET::HasInet()
{
	return InterfaceTest(eTestINETUp);
}

bool CUnixINET::HasModem()
{
	return InterfaceTest(eTestModemUp);
}

bool CUnixINET::WillDial()
{
	// No dial if connection already present
	return !HasInet();
}

bool CUnixINET::AutoDial()
{
	// Look for dial script
	cdstring fname = CConnectionManager::sConnectionManager.GetApplicationCWD();
	fname += cDialScript;
	if (!JFileExists(fname))
		return false;

	// Get dial script command
	cdstring cmd;
	{
		cdifstream input(fname);
		::getline(input, cmd);
	}

	// Must have command
	if (cmd.empty())
		return false;

	// Check current dialer process
	if (sDialer)
	{
		// Must kill it if its still running
		// Technically this should not happen
		JThisProcess::Instance()->CheckForFinishedChild(kFalse);
		if (!sDialer->IsFinished())
		{
			sDialer->Kill();
			sDialer->WaitUntilFinished();
		}
		
		delete sDialer;
	}
	
	// Start dial process
	sDialer = NULL;
	const JError err = JSimpleProcess::Create(&sDialer, cmd, kFalse);
	if (!err.OK())
		return false;

	// Wait for PPP to come up or dial process to end
	JThisProcess::Instance()->CheckForFinishedChild(kFalse);
	while(!HasModem() && !sDialer->IsFinished())
	{
		// Wait 1 ms
		::usleep(1000);
		JThisProcess::Instance()->CheckForFinishedChild(kFalse);
	}

	if (sDialer->IsFinished())
	{
		delete sDialer;
		sDialer = NULL;
	}

	return HasInet();
}

bool CUnixINET::AutoHangup()
{
	// Must have dial process
	JThisProcess::Instance()->CheckForFinishedChild(kFalse);
	if (sDialer && !sDialer->IsFinished())
	{
		// kill -INT dial process
		sDialer->SendSignal(SIGINT);

		// Wait for dial process to end
		sDialer->WaitUntilFinished();
	}

	delete sDialer;
	sDialer = NULL;

	return true;
}

bool CUnixINET::GetInterfaceIPs(unsigned long*& ips)
{
	// Look for a suitable interface
    unsigned long numreqs = 30;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */ 
 	int sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
 	if (sock == INVALID_SOCKET)
 		return false;

	struct ifconf ifc;
    ifc.ifc_buf = NULL;
    for (;;)
    {
		ifc.ifc_len = sizeof(struct ifreq) * numreqs;
		ifc.ifc_buf = (char*) ::realloc(ifc.ifc_buf, ifc.ifc_len);

		if (::ioctl(sock, SIOCGIFCONF, &ifc) < 0)
		{
			::close(sock);
			return false;
		}

		if (ifc.ifc_len == sizeof(struct ifreq) * numreqs)
		{
		    /* assume it overflowed and try again */
		    numreqs += 10;
		    continue;
		}
		break;
    }

	// Count number of interface IPs to return
	unsigned long ctr = 0;
    struct ifreq* ifr = ifc.ifc_req;
    for (int n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq), ifr++)
    {
    	// Get flags for interface
	    struct ifreq ifrf;
		::memset(&ifrf, 0, sizeof(ifrf));
	    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
	    if (::ioctl(sock, SIOCGIFFLAGS, &ifrf) < 0)
			continue;

		// Check that it is up and not loopback
		if (ifrf.ifr_flags & IFF_UP)
			ctr++;
    }

	// Create space for ips
	ips = (unsigned long*) malloc(sizeof(unsigned long) * (ctr + 1));
	
	// Copy valid ips into ip space
	unsigned long* p = ips;
	unsigned long idx = 0;		// Used to prevent buffer overflow
    ifr = ifc.ifc_req;
    for (int n = 0; (idx < ctr) && (n < ifc.ifc_len); n += sizeof(struct ifreq), ifr++)
    {
    	// Get flags for interface
	    struct ifreq ifrf;
		::memset(&ifrf, 0, sizeof(ifrf));
	    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
	    if (::ioctl(sock, SIOCGIFFLAGS, &ifrf) < 0)
			continue;

		// Check that it is up and not loopback
		if (ifrf.ifr_flags & IFF_UP)
		{
		    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
		    if (::ioctl(sock, SIOCGIFADDR, &ifrf) < 0)
		    {
		    	// Must fill in something - use 27.0.0.1
		    	*p++ = 0x7F000001;
		    	idx++;
		    }
		    else
		    {
		    	// Check for IPv4 address
		    	if (ifrf.ifr_addr.sa_family == AF_INET)
		    	{
			    	// Copy address
			    	*p++ = reinterpret_cast<sockaddr_in*>(&ifrf.ifr_addr)->sin_addr.s_addr;
			    	idx++;
			    }
		    }
		}
    }
	
	// Terminate list with NULL;
	*p = 0;

	// Clean-up
    ::free(ifc.ifc_buf);
    ::close(sock);
    
    return true;
}

bool CUnixINET::InterfaceTest(EInterfaceTest test)
{
	bool result = false;

	// Look for a suitable interface
    unsigned long numreqs = 30;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */ 
 	int sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
 	if (sock == INVALID_SOCKET)
 		return false;

	struct ifconf ifc;
    ifc.ifc_buf = NULL;
    for (;;)
    {
		ifc.ifc_len = sizeof(struct ifreq) * numreqs;
		ifc.ifc_buf = (char*) ::realloc(ifc.ifc_buf, ifc.ifc_len);

		if (::ioctl(sock, SIOCGIFCONF, &ifc) < 0)
		{
			::close(sock);
			return false;
		}

		if (ifc.ifc_len == sizeof(struct ifreq) * numreqs)
		{
		    /* assume it overflowed and try again */
		    numreqs += 10;
		    continue;
		}
		break;
    }

    struct ifreq* ifr = ifc.ifc_req;
    for (int n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq), ifr++)
    {
    	// Get flags for interface
	    struct ifreq ifrf;
		::memset(&ifrf, 0, sizeof(ifrf));
	    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
	    if (::ioctl(sock, SIOCGIFFLAGS, &ifrf) < 0)
			continue;

		// Check that it is up and not loopback
		switch(test)
		{
		case eTestINETUp:
			result = (ifrf.ifr_flags & IFF_UP) &&
							!(ifrf.ifr_flags & IFF_LOOPBACK);
			break;
		case eTestModemUp:
			result = (ifrf.ifr_flags & IFF_UP) &&
							(ifrf.ifr_flags & IFF_POINTOPOINT);
			break;
		}

		// Found a working one - done
		if (result)
			break;
    }

    ::free(ifc.ifc_buf);
    ::close(sock);
    return result;
}
