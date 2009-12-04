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

/* 
	CLocalCalendarClient.cpp

	Author:			
	Description:	<describe the CLocalCalendarClient class here>
*/

#include "CLocalCalendarClient.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarRecord.h"
#include "CCalendarStoreNode.h"
#include "CCalendarStoreWebcal.h"
#include "CDisplayItem.h"
#include "CGeneralException.h"
#include "CLocalCommon.h"

#include "CICalendar.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include "diriterator.h"
#include "cdfstream.h"

#include __stat_header

using namespace calstore; 

CLocalCalendarClient::CLocalCalendarClient(CCalendarProtocol* owner) :
	CCalendarClient(owner)
{
	// Init instance variables
	InitLocalClient();
}

CLocalCalendarClient::CLocalCalendarClient(const CLocalCalendarClient& copy, CCalendarProtocol* owner) :
	CCalendarClient(copy, owner)
{
	// Init instance variables
	InitLocalClient();

	mCWD = copy.mCWD;

	mRecorder = copy.mRecorder;
}

CLocalCalendarClient::~CLocalCalendarClient()
{
	mRecorder = NULL;
}

void CLocalCalendarClient::InitLocalClient()
{
	// Protocol that can disconnect will always be cached
	mCaching = GetCalendarProtocol()->CanDisconnect();

	mRecorder = NULL;
	mRecordID = 0;
}

// Create duplicate, empty connection
CINETClient* CLocalCalendarClient::CloneConnection()
{
	// Copy construct this
	return new CLocalCalendarClient(*this, GetCalendarProtocol());

}

bool CLocalCalendarClient::IsCaching() const
{
	return mCaching;
}

#pragma mark ____________________________Start/Stop

// Start TCP
void CLocalCalendarClient::Open()
{
	// Do account reset
	Reset();
}

// Reset acount info
void CLocalCalendarClient::Reset()
{
	// get CWD from owner
	mCWD = GetCalendarProtocol()->GetOfflineCWD();

	// Must append dir delim if not present
	if (mCWD.length() && (mCWD[mCWD.length() - 1] != os_dir_delim))
		mCWD += os_dir_delim;

	// May need to check for INBOX on POP3
	CheckCWD();
}

// Check CWD
void CLocalCalendarClient::CheckCWD()
{
	// Local does nothing

}

// Release TCP
void CLocalCalendarClient::Close()
{
	// Local does nothing

}

// Program initiated abort
void CLocalCalendarClient::Abort()
{
}

// Forced close
void CLocalCalendarClient::Forceoff()
{
}

#pragma mark ____________________________Login & Logout

void CLocalCalendarClient::Logon()
{
	// Local does nothing
	
	// Must fail if empty CWD
	if (mCWD.empty())
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}
}

void CLocalCalendarClient::Logoff()
{
	// Nothing to do for local
}

#pragma mark ____________________________Handle Errors

// Descriptor for object error context
const char*	CLocalCalendarClient::INETGetErrorDescriptor() const
{
	return "Calendar: ";
}

#pragma mark ____________________________Protocol

// Tickle to keep connection alive
void CLocalCalendarClient::_Tickle(bool force_tickle)
{
	// Local does nothing

}

void CLocalCalendarClient::_ListCalendars(CCalendarStoreNode* root)
{
	// Node must be protocol or directory
	if (!root->IsProtocol() && !root->IsDirectory())
		return;

	// Get name for new file
	cdstring fpath = MapName(*root);

	// Directory scan starting at the root
	ListCalendars(root, fpath);
	
	// Always sort children after adding all of them
	root->SortChildren();
}

void CLocalCalendarClient::ListCalendars(CCalendarStoreNode* root, const cdstring& path)
{
	// Directory scan
	diriterator _dir(path, true, ".ics");
	const char* fname = NULL;
	while(_dir.next(&fname))
	{
		// Get the full path of the found item
		cdstring fpath(path);
		::addtopath(fpath, fname);

		// Get the relative path which will be the name of the node.
		// This is relative to the root path for this client.
		cdstring rpath(fpath, mCWD.length());
		
		// Strip off trailing .ics
		if (rpath.compare_end(".ics"))
			rpath.erase(rpath.length() - 4);

		// Create the new node and add to parent
		CCalendarStoreNode* node = new CCalendarStoreNode(GetCalendarProtocol(), root, _dir.is_dir(), false, false, rpath);
		root->AddChild(node);

		// Scan into directories
		if (_dir.is_dir())
		{
			// Use new node as the root
			ListCalendars(node, fpath);
			
			// Always mark node as ahving been expanded
			node->SetHasExpanded(true);
			
			// Always sort the children after adding all of them
			node->SortChildren();
		}
	}
}

void CLocalCalendarClient::_CreateCalendar(const CCalendarStoreNode& node)
{
	// Get name for new file
	cdstring fpath = MapName(node);
	
	// Make sure it does not already exist
	if (::fileexists(fpath) || ::direxists(fpath))
	{
		//throw CGeneralException(-1, "Calendar file/directory exists");
		throw CGeneralException(-1);
	}
	
	// Must ensure entire path exists
	{
		cdstring convert = LocalFileName(node.GetName(), GetCalendarProtocol()->GetDirDelim(), GetCalendarProtocol()->IsDisconnectedCache());
		if (!node.IsDirectory())
			convert += ".ics";
		cdstring mbox_name = mCWD;
		cdstring dir_delim = os_dir_delim;
		char* dir = ::strtok(convert.c_str_mod(), dir_delim);
		char* next_dir = ::strtok(NULL, dir_delim);
		while(dir && next_dir)
		{
			mbox_name += dir;
			::chkdir(mbox_name);
			mbox_name += os_dir_delim;
			dir = next_dir;
			next_dir = ::strtok(NULL, dir_delim);
		}
	}

	if (node.IsDirectory())
	{
		if (__mkdir(fpath, S_IRWXU))
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not create directory");
			throw CGeneralException(_errno);
		}
	}
	else
	{
		FILE* file;
		if ((file = ::fopen_utf8(fpath, "ab")) != NULL)
			::fclose(file);
		else
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not create calendar file");
			throw CGeneralException(_errno);
		}
		
		// Always clear any existing cache file as a precaution
		if (IsCaching())
		{
			cdstring cpath = MapCacheName(node);
			if (::fileexists(cpath))
				::remove_utf8(cpath);
		}
	}

	// Record action
	if (mRecorder)
	{
		CActionRecorder::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Create(node);
	}
}

void CLocalCalendarClient::_DeleteCalendar(const CCalendarStoreNode& node)
{
	// Get name for new file
	cdstring fpath = MapName(node);
	
	if (node.IsDirectory())
	{
		// Make sure it already exists
		if (!::direxists(fpath))
		{
			//throw CGeneralException(-1, "Directory being deleted does not exist");
			throw CGeneralException(-1);
		}
		
		// Make sure it is empty
		if (::count_dir_contents(fpath) != 0)
		{
			//throw CGeneralException(-1, "Directory being deleted is not empty");
			throw CGeneralException(-1);
		}

		if (::delete_dir(fpath) != 0)
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not delete directory");
			throw CGeneralException(_errno);
		}
	}
	else
	{
		// Make sure it already exists
		if (!::fileexists(fpath))
		{
			//throw CGeneralException(-1, "Calendar file does not exist");
			throw CGeneralException(-1);
		}
		
		if (::remove_utf8(fpath) != 0)
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not delete calendar file");
			throw CGeneralException(_errno);
		}
		
		// Always clear any existing cache file
		if (IsCaching())
		{
			cdstring cpath = MapCacheName(node);
			if (::fileexists(cpath))
				::remove_utf8(cpath);
		}
	}

	// Record action
	if (mRecorder)
	{
		CActionRecorder::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Delete(node);
	}
}

void CLocalCalendarClient::_RenameCalendar(const CCalendarStoreNode& node, const cdstring& node_new)
{
	// Get name for new file
	cdstring fpath_old = MapName(node);
	cdstring convert = LocalFileName(node_new, GetCalendarProtocol()->GetDirDelim(), GetCalendarProtocol()->IsDisconnectedCache());
	cdstring fpath_new = MapName(convert, node.IsDirectory());
	
	if (node.IsDirectory())
	{
		// Make sure old already exists
		if (!::direxists(fpath_old))
		{
			//throw CGeneralException(-1, "Directory being renamed does not exist");
			throw CGeneralException(-1);
		}
		
		// Make sure new does not already exist
		if (::direxists(fpath_new))
		{
			//throw CGeneralException(-1, "Directory being renamed to exists");
			throw CGeneralException(-1);
		}

		if (::rename_utf8(fpath_old, fpath_new) != 0)
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not rename directory");
			throw CGeneralException(_errno);
		}
	}
	else
	{
		// Make sure old already exists
		if (!::fileexists(fpath_old))
		{
			//throw CGeneralException(-1, "Calendar file being renamed does not exist");
			throw CGeneralException(-1);
		}
		
		// Make sure new does not already exist
		if (::fileexists(fpath_new))
		{
			//throw CGeneralException(-1, "Calendar file being renamed to exists");
			throw CGeneralException(-1);
		}

		if (::rename_utf8(fpath_old, fpath_new) != 0)
		{
			int _errno = os_errno;
			//throw CGeneralException(_errno, "Could not rename calendar file");
			throw CGeneralException(_errno);
		}
		
		// Always rename any existing cache file
		if (IsCaching())
		{
			cdstring cpath_old = MapCacheName(node);
			cdstring cpath_new = MapCacheName(convert, node.IsDirectory());
			if (::fileexists(cpath_old))
			{
				// Delete existing cache file as a precaution
				if (::fileexists(cpath_new))
					::remove_utf8(cpath_new);
				
				// Rename cache file
				::rename_utf8(cpath_old, cpath_new);
			}
		}
	}

	// Record action
	if (mRecorder)
	{
		CActionRecorder::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Rename(node, node_new);
	}
}

bool CLocalCalendarClient::_TestCalendar(const CCalendarStoreNode& node)
{
	// Get name for new file
	cdstring fpath = MapName(node);
	
	// Make sure it already exists
	if (node.IsDirectory())
	{
		return ::direxists(fpath);
	}
	else
	{
		return ::fileexists(fpath);
	}
}

bool CLocalCalendarClient::_TouchCalendar(const CCalendarStoreNode& node)
{
	if (!_TestCalendar(node))
	{
		StValueChanger<CCalendarRecord*> value(mRecorder, NULL);
		_CreateCalendar(node);
		return true;
	}
	else
		return false;
}

void CLocalCalendarClient::_LockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Nothing to do
}

void CLocalCalendarClient::_UnlockCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Nothing to do
}

bool CLocalCalendarClient::_CheckCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Nothing to do for local
	return false;
}

bool CLocalCalendarClient::_CalendarChanged(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Nothing to do for local as this is only used when sync'ing with server
	return false;
}

void CLocalCalendarClient::_UpdateSyncToken(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Nothing to do for local as this is only used when sync'ing with server
}

void CLocalCalendarClient::_SizeCalendar(CCalendarStoreNode& node)
{
	// Does it exist on disk?
	if (_TestCalendar(node))
	{
		cdstring fpath = MapName(node);

		// Get sizes
		unsigned long size = 0;
		struct stat finfo;
		if (::stat_utf8(fpath, &finfo))
		{
			int err_no = os_errno;
			CLOG_LOGTHROW(CGeneralException, err_no);
			throw CGeneralException(err_no);
		}
		else
			size += finfo.st_size;

		// Add cache file if present
		if (IsCaching())
		{
			cdstring cpath = MapCacheName(node);
			if (!::stat_utf8(cpath, &finfo))
				size += finfo.st_size;

		}

		node.SetSize(size);
	}
	else
		// Not cached => size = 0
		node.SetSize(0);
}

void CLocalCalendarClient::_ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed)
{
	// Get name for new file
	cdstring fpath = MapName(node);
	
	// Make sure it already exists
	if (!::fileexists(fpath))
	{
		//throw CGeneralException(-1, "Calendar file does not exist");
		throw CGeneralException(-1);
	}
	
	// Read calendar from file
	cdifstream is(fpath);
	cal.Parse(is);
	
	// Get read-only state
	cal.SetReadOnly(!::filereadwriteable(fpath));
	if (node.GetWebcal() && node.GetWebcal()->GetReadOnly())
		cal.SetReadOnly(true);
	
	// Load cache file
	if (IsCaching())
	{
		cdstring cpath = MapCacheName(node);
		cdifstream cis(cpath);
		cal.ParseCache(cis);
	}
}

void CLocalCalendarClient::_WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Get name for new file
	cdstring fpath = MapName(node);
	cdstring cpath = MapCacheName(node);
	
	// Make sure it already exists
	if (!::fileexists(fpath))
	{
		//throw CGeneralException(-1, "Calendar file does not exist");
		throw CGeneralException(-1);
	}
	
	// Make sure its writeable
	if (!::filereadwriteable(fpath))
	{
		//throw CGeneralException(-1, "Calendar file not writeable");
		throw CGeneralException(-1);
	}
	
	// Transactional write
	cdstring fpath_tmp(fpath);
	fpath_tmp += ".tmp";
	cdstring cpath_tmp(cpath);
	cpath_tmp += ".tmp";
	
	// Create the file and write calendar to it
	{
		cdofstream os(fpath_tmp);
		cal.Generate(os, IsCaching());
	}
	
	// Write cache file
	if (IsCaching())
	{
		cdofstream cos(cpath_tmp);
		cal.GenerateCache(cos);
	}

	// Remove old and rename the new
	::remove_utf8(fpath);
	::rename_utf8(fpath_tmp, fpath);
	::remove_utf8(cpath);
	::rename_utf8(cpath_tmp, cpath);

	// Record action
	if (mRecorder)
	{
		CActionRecorder::StActionRecorder record(mRecorder, mRecordID);
		mRecorder->Change(node);
	}
}

bool CLocalCalendarClient::_CanUseComponents() const
{
	// Only handles entire calendar files
	return false;
}

void CLocalCalendarClient::_GetComponentInfo(const CCalendarStoreNode& node, iCal::CICalendar& cal, cdstrmap& comps)
{
	// Does nothing in this implementation
}

void CLocalCalendarClient::_AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Does nothing in this implementation
}

void CLocalCalendarClient::_ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Does nothing in this implementation
}

void CLocalCalendarClient::_RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Does nothing in this implementation
}

void CLocalCalendarClient::_RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl)
{
	// Does nothing in this implementation
}

void CLocalCalendarClient::_ReadComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstrvect& rurls)
{
	// Does nothing in this implementation
}

iCal::CICalendarComponent* CLocalCalendarClient::_ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl)
{
	// Does nothing in this implementation
	return NULL;
}

#pragma mark ____________________________ACLs

// Set acl on server
void CLocalCalendarClient::_SetACL(CCalendarStoreNode& node, CACL* acl)
{
}

// Delete acl on server
void CLocalCalendarClient::_DeleteACL(CCalendarStoreNode& node, CACL* acl)
{
}

// Get all acls for calendar from server
void CLocalCalendarClient::_GetACL(CCalendarStoreNode& node)
{
}

// Get allowed rights for user
void CLocalCalendarClient::_ListRights(CCalendarStoreNode& node, CACL* acl)
{
}

// Get current user's rights to calendar
void CLocalCalendarClient::_MyRights(CCalendarStoreNode& node)
{
}

#pragma mark ____________________________Schedule

// Get Scheduling Inbox/Outbox URIs
void CLocalCalendarClient::_GetScheduleInboxOutbox(const CCalendarStoreNode& node, cdstring& inboxURI, cdstring& outboxURI)
{
}

// Run scheduling request
void CLocalCalendarClient::_Schedule(const cdstring& outboxURI,
									 const cdstring& originator,
									 const cdstrvect& recipients,
									 const iCal::CICalendar& cal,
									 iCal::CITIPScheduleResultsList& results)
{
	
}

void CLocalCalendarClient::_GetFreeBusyCalendars(cdstrvect& calendars)
{
	
}

void CLocalCalendarClient::_SetFreeBusyCalendars(const cdstrvect& calendars)
{
	
}

#pragma mark ____________________________Utils

cdstring CLocalCalendarClient::MapName(const CCalendarStoreNode& node) const
{
	if (node.IsProtocol())
		return mCWD;
	else
	{
		cdstring convert = LocalFileName(node.GetName(), GetCalendarProtocol()->GetDirDelim(), GetCalendarProtocol()->IsDisconnectedCache());
		return MapName(convert, node.IsDirectory());
	}
}

cdstring CLocalCalendarClient::MapName(const cdstring& node_name, bool is_dir) const
{
	// Create path from calendar name
	cdstring result(mCWD);
	::addtopath(result, node_name);
	
	// Always add ".ics" for actual calendars
	if (!is_dir)
		result += ".ics";

	return result;
}

cdstring CLocalCalendarClient::MapCacheName(const CCalendarStoreNode& node) const
{
	if (node.IsProtocol())
		return mCWD;
	else
	{
		cdstring convert = LocalFileName(node.GetName(), GetCalendarProtocol()->GetDirDelim(), GetCalendarProtocol()->IsDisconnectedCache());
		return MapCacheName(convert, node.IsDirectory());
	}
}

cdstring CLocalCalendarClient::MapCacheName(const cdstring& node_name, bool is_dir) const
{
	// Create path from calendar name
	cdstring result(mCWD);
	::addtopath(result, node_name);
	
	// Always add ".xml" for actual calendar cache files
	if (!is_dir)
		result += ".xml";

	return result;
}
