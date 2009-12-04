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

// CPluginManager.h
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 16-Nov-1997
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32
//
// Description:
// This class implements a manager for DLL based plug-ins in Mulberry.
//
// History:
// CD	16-Nov-1997: Created initial header and implementation.
// CD	23-Dec-1997: Adding preference support.
//

#ifndef __CPLUGINMANAGER__MULBERRY__
#define __CPLUGINMANAGER__MULBERRY__

#include "CPlugin.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const ResIDT	MENU_ImportAddresses = 166;
const ResIDT	MENU_ExportAddresses = 167;
#endif

class CAdbkIOPlugin;
class CAuthPlugin;
class CConfigPlugin;
class COptionsMap;
class CPswdChangePlugin;
class CSecurityPlugin;
class CSpellPlugin;
class CSSLPlugin;

#if __dest_os == __linux_os
class JXTextMenu;
#endif

class CPluginManager
{
public:
	static CPluginManager sPluginManager;

	CPluginManager();
	~CPluginManager();
	
	// Getting plug-in information
	void LookforPlugins();						// Search Plug-ins directory for suitable files
	void LookforPlugins(fspec plugindir, cdstrset& previous);		// Search specific directory for suitable files
	CPluginList& GetPlugins()					// Return list of plugins
		{ return mList; }
	void CleanPlugins();

	CPlugin* CreateActualPluginType(CPlugin* original) const;	// Create actual plugin type

	// Get specific plug-ins
	bool HasConfig() const
		{ return mConfigPlugin != NULL; }
	CConfigPlugin* GetConfig()
		{ return mConfigPlugin; }

	bool HasSpelling() const
		{ return (mSpellingPlugin != NULL); }
	CSpellPlugin* GetSpelling()
		{ return mSpellingPlugin; }

	bool HasSecurity() const
		{ return mHasSecurityPlugin; }

	bool HasMailMerge() const
		{ return (mMailMergePlugin != NULL); }
	CPlugin* GetMailMerge()
		{ return mMailMergePlugin; }

	bool HasSSL() const
		{ return (mSSLPlugin != NULL); }
	CSSLPlugin* GetSSL()
		{ return mSSLPlugin; }

	void GetAuthPlugins(cdstrvect& names) const;				// Get names of all auth plugins
	CAuthPlugin* FindAuthPlugin(const char* auth_id) const;		// Find matching plugin
	CAuthPlugin* GetAuthPlugin(const char* prefs_id) const;		// Find matching plugin

	CAdbkIOPlugin* FindAdbkIOPlugin(const char* name) const;	// Find matching plugin

	void UpdatePswdChange();
	unsigned long CountPswdChange() const
		{ return mCountPswdChange; }
	CPswdChangePlugin* FindPswdChangePlugin(const char* name) const;

#if __dest_os == __win32_os
	void SyncAdbkIOPluginMenu(CMenu* import_menu, CMenu* export_menu) const;				// Sync menu with AdbkIOs
#elif __dest_os == __linux_os
	void SyncAdbkIOPluginMenu(JXTextMenu* import_menu, JXTextMenu* export_menu) const;		// Sync menu with AdbkIOs
#endif

	const cdstrvect& GetPluginDirs() const;
	void SetPluginDirs(const cdstrvect& dirs)
		{ mPluginDirs = dirs; }

	// Preferences
	void 	WriteToMap(COptionsMap* theMap, bool dirty_only);	// Write data to a stream
	bool	ReadFromMap(COptionsMap* theMap,
							NumVersion vers_app,
							NumVersion& vers_prefs);			// Read data from a stream

private:
	CPluginList		mList;
	CConfigPlugin*	mConfigPlugin;
	CSpellPlugin*	mSpellingPlugin;
	bool			mHasSecurityPlugin;
	CPlugin*		mMailMergePlugin;
	CSSLPlugin*		mSSLPlugin;
	unsigned long	mCountPswdChange;
	
	mutable cdstrvect	mPluginDirs;
	
	void	InitPluginDirs() const;

	void	AddAdbkIOPluginUI(CAdbkIOPlugin* plugin) const;
};

#endif
