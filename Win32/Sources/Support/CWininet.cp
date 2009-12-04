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


// CWininet.cp

// Basic WININET support

#include "CWininet.h"

bool CWininet::sWININETResult = false;

CWininet::CWininet()
{
}

CWininet::~CWininet()
{
}

#pragma mark ____________________________Statics

bool CWininet::HasInet()
{
	DWORD flags = 0;
	return InternetGetConnectedState(&flags, 0);
}

bool CWininet::WillDial()
{
	DWORD flags = 0;
	BOOL result = InternetGetConnectedState(&flags, 0);
	return !result && (flags & INTERNET_CONNECTION_MODEM);
}

bool CWininet::HasModem()
{
	DWORD flags = 0;
	InternetGetConnectedState(&flags, 0);
	return flags & INTERNET_CONNECTION_MODEM;
}

bool CWininet::AutoDial()
{
	return InternetAutodial(INTERNET_AUTODIAL_FORCE_ONLINE, 0);
}

bool CWininet::AutoHangup()
{
	return InternetAutodialHangup(0);
}

bool CWininet::GetInterfaceIPs(unsigned long*& ips)
{
	ips = NULL;

	// Get address table size first
	PMIB_IPADDRTABLE ip_table = NULL;
	ULONG ip_table_size = 0;
	if (GetIpAddrTable(ip_table, &ip_table_size, true) != ERROR_INSUFFICIENT_BUFFER)
		return false;

	// Allocate table size
	ip_table = (PMIB_IPADDRTABLE) ::malloc(ip_table_size);
	if (!ip_table)
		return false;
	::memset(ip_table, 0, ip_table_size);

	// Get actual table
	if (GetIpAddrTable(ip_table, &ip_table_size, true) != NO_ERROR)
	{
		::free(ip_table);
		return false;
	}
	
	// Now look at each address and dtermine whether its valid
	unsigned long ctr = 0;
	for(DWORD i = 0; i < ip_table->dwNumEntries; i++)
	{
		// Must have non-zero address
		if (ip_table->table[i].dwAddr && (ip_table->table[i].dwAddr != 0x7F000001))
		{
			// Look at indexed interface
			MIB_IFROW if_row;
			if_row.dwIndex = ip_table->table[i].dwIndex;
			if (GetIfEntry(&if_row) != NO_ERROR)
			{
				::free(ip_table);
				return false;
			}
			
			// Check whether its active
			if ((if_row.dwAdminStatus == MIB_IF_ADMIN_STATUS_UP) &&
				((if_row.dwOperStatus == MIB_IF_OPER_STATUS_UNREACHABLE) ||
				 (if_row.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL) ||
				 (if_row.dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED)))
				ctr++;
		}
	}

	// Now assign results array
	ips = (unsigned long*) ::malloc(sizeof(unsigned long) * (ctr + 1));
	if (!ips)
	{
		::free(ip_table);
		return false;
	}

	unsigned long* p = ips;
	unsigned long idx = 0;	// Used to prevent buffer overflow
	for(DWORD i = 0; (idx < ctr) && (i < ip_table->dwNumEntries); i++)
	{
		// Must have non-zero address
		if (ip_table->table[i].dwAddr && (ip_table->table[i].dwAddr != 0x7F000001))
		{
			// Look at indexed interface
			MIB_IFROW if_row;
			if_row.dwIndex = ip_table->table[i].dwIndex;
			if (GetIfEntry(&if_row) != NO_ERROR)
			{
				::free(ips);
				::free(ip_table);
				return false;
			}
			
			// Check whether its active
			if ((if_row.dwAdminStatus == MIB_IF_ADMIN_STATUS_UP) &&
				((if_row.dwOperStatus == MIB_IF_OPER_STATUS_UNREACHABLE) ||
				 (if_row.dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL) ||
				 (if_row.dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED)))
			{
				*p++ = ip_table->table[i].dwAddr;
				idx++;
			}
		}
	}

	// NULL terminate
	*p++ = NULL;
	
	// Clean-up
	::free(ip_table);

	return true;
}
