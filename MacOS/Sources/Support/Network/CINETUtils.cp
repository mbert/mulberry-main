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


// Source for CINETUtils class

#include "CINETUtils.h"

#include "CConnectionManager.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_media.h>
#include <netinet/in.h>
#include <unistd.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

bool CINETUtils::HasInet()
{
	return InterfaceTest(eTestINETUp);
}

bool CINETUtils::HasModem()
{
	return InterfaceTest(eTestModemUp);
}

bool CINETUtils::WillDial()
{
	// No dial if connection already present
	return !HasInet();
}

bool CINETUtils::AutoDial()
{
	return HasInet();
}

bool CINETUtils::AutoHangup()
{
	return true;
}

bool CINETUtils::GetInterfaceIPs(unsigned long*& ips)
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
    for (int n = 0; n < ifc.ifc_len; n += _SIZEOF_ADDR_IFREQ(*ifr))
    {
    	ifr = (struct ifreq*) ((char*) ifc.ifc_req + n);

    	// Get flags for interface
	    struct ifreq ifrf;
		::memset(&ifrf, 0, sizeof(ifrf));
	    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
	    if (::ioctl(sock, SIOCGIFFLAGS, &ifrf) < 0)
			continue;

		// Check that it is up
		if (!(ifrf.ifr_flags & IFF_UP))
			continue;
		
		// Get media state for interface and make sure it is active
	    struct ifmediareq ifrm;
		::memset(&ifrm, 0, sizeof(ifrm));
	    ::strcpy(ifrm.ifm_name, ifr->ifr_name);
	    if ((::ioctl(sock, SIOCGIFMEDIA, &ifrm) != -1) && !(ifrm.ifm_status & IFM_ACTIVE))
	    {
			continue;
	    }

		ctr++;
    }

	// Create space for ips
	ips = (unsigned long*) malloc(sizeof(unsigned long) * (ctr + 1));
	
	// Copy valid ips into ip space
	unsigned long* p = ips;
	unsigned long idx = 0;		// Used to prevent buffer overflow
    ifr = ifc.ifc_req;
    for (int n = 0; (idx < ctr) && (n < ifc.ifc_len); n += _SIZEOF_ADDR_IFREQ(*ifr))
    {
     	ifr = (struct ifreq*) ((char*) ifc.ifc_req + n);

    	// Get flags for interface
	    struct ifreq ifrf;
		::memset(&ifrf, 0, sizeof(ifrf));
	    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
	    if (::ioctl(sock, SIOCGIFFLAGS, &ifrf) < 0)
			continue;

		// Check that it is up
		if (!(ifrf.ifr_flags & IFF_UP))
			continue;
		
		// Get media state for interface and check that it is active
	    struct ifmediareq ifrm;
		::memset(&ifrm, 0, sizeof(ifrm));
	    ::strcpy(ifrm.ifm_name, ifr->ifr_name);
	    if ((::ioctl(sock, SIOCGIFMEDIA, &ifrm) != -1) && !(ifrm.ifm_status & IFM_ACTIVE))
	    {
			continue;
	    }

		// Check that it is not loopback
	    ::strcpy(ifrf.ifr_name, ifr->ifr_name);
	    if (::ioctl(sock, SIOCGIFADDR, &ifrf) < 0)
	    {
	    	// Must fill in something - use 127.0.0.1
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
	
	// Terminate list with NULL;
	*p = 0;

	// Clean-up
    ::free(ifc.ifc_buf);
    ::close(sock);
    
    return true;
}

bool CINETUtils::InterfaceTest(EInterfaceTest test)
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
    for (int n = 0; n < ifc.ifc_len; n += _SIZEOF_ADDR_IFREQ(*ifr))
    {
    	ifr = (struct ifreq*) ((char*) ifc.ifc_req + n);
    
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
