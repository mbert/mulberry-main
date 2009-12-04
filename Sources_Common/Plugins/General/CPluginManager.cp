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

// CPluginManager.cp
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

#include "CPluginManager.h"

#include "CAdbkIOPlugin.h"
#include "CAuthPlugin.h"
#include "CConfigPlugin.h"
#include "CLocalCommon.h"
#include "CMulberryApp.h"
#include "CPreferenceKeys.h"
#include "CPswdChangePlugin.h"
#include "CSecurityPlugin.h"
#include "CSpellPlugin.h"
#include "CSSLPlugin.h"

#include "diriterator.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "MyCFString.h"
#elif __dest_os == __win32_os
#include "CUnicodeUtils.h"
#elif __dest_os == __linux_os
#include <JXTextMenu.h>
#include <JString.h>
#include <jDirUtil.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#endif

#pragma mark ____________________________consts

#if __dest_os == __mac_os || __dest_os == __mac_os_x
const unsigned char cPluginDirName[] = "\pPlug-ins";
#elif __dest_os == __win32_os
const char cPluginDirName[] = "Plug-ins";
const char cPluginSearch[] = "\\*.dll";
#elif __dest_os == __linux_os
const char cPluginDirName[] = "mulberry/Plug-ins";
const char cUserPluginDirName[] = ".mulberry/Plug-ins";
const char cPluginSearch[] = "\\*.so";
#else
#error __dest_os
#endif

#pragma mark ____________________________statics

CPluginManager CPluginManager::sPluginManager;

#pragma mark ____________________________CPluginManager

// Constructor
CPluginManager::CPluginManager()
{
	mConfigPlugin = NULL;
	mSpellingPlugin = NULL;
	mHasSecurityPlugin = false;
	mMailMergePlugin = NULL;
	mSSLPlugin = NULL;
	mCountPswdChange = 0;
}

// Destructor
CPluginManager::~CPluginManager()
{
	// Clear all plugins
	CleanPlugins();
}

void CPluginManager::InitPluginDirs() const
{
	mPluginDirs.clear();

#if __dest_os == __mac_os || __dest_os == __mac_os_x || __dest_os == __win32_os
	// Get Plug-ins folder
	cdstring dir = CConnectionManager::sConnectionManager.GetApplicationCWD();
	dir += cPluginDirName;
	mPluginDirs.push_back(dir);
#else
	// Try users home directory first (create it if it does not exist)
	JString plugindir;
	JGetHomeDirectory(&plugindir);
	cdstring dir = plugindir.GetCString();
	::addtopath(dir, cUserPluginDirName);
	try
	{
		::chkdir(dir);
		if (::direxists(dir))
			mPluginDirs.push_back(dir);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	// Try application directory next (do not create)
	dir = cdstring::null_str;
	dir.reserve(FILENAME_MAX);
	if (::getcwd(dir, FILENAME_MAX) != NULL)
	{
		::addtopath(dir, cUserPluginDirName);
		if (::direxists(dir))
			mPluginDirs.push_back(dir);
	}

	// Then /usr/local/lib
	dir = "/usr/local/lib/";
	::addtopath(dir, cPluginDirName);
	if (::direxists(dir))
		mPluginDirs.push_back(dir);

	// Then look in /usr/lib
	dir = "/usr/lib/";
	::addtopath(dir, cPluginDirName);
	if (::direxists(dir))
		mPluginDirs.push_back(dir);
#endif
}

const cdstrvect& CPluginManager::GetPluginDirs() const
{
	if (mPluginDirs.empty())
		InitPluginDirs();
	return mPluginDirs;
}

// Search Plug-ins directory for suitable files
#if __dest_os == __mac_os || __dest_os == __mac_os_x
void CPluginManager::LookforPlugins(void)
{
	// Get Plug-ins folder
	if (GetPluginDirs().empty())
		return;
	cdstring dir = GetPluginDirs().front();

#if __dest_os == __mac_os
	diriterator iter(dir, 'Mlby', 'shlb');	// Classic: use type/creator match
#else
	diriterator iter(dir, true, ".bundle");	// OS X: match via bundle extension
#endif
	const char* fname = NULL;
	while(iter.next(&fname))
	{
#if __dest_os == __mac_os_x
		{
			// Bundle must end with proper extension
			cdstring temp(fname);
			if (!temp.compare_end(".bundle"))
				continue;
		}
#endif

		// Got a plug-in!
		CPlugin* plugin = NULL;
		try
		{
			// Get FSSpec
			cdstring plugin_name(dir);
			::addtopath(plugin_name, fname);
			MyCFString cfstr(plugin_name, kCFStringEncodingUTF8);
			PPx::FSObject plugin_spec(cfstr);

			// Create new plugin
			plugin = new CPlugin(&plugin_spec);

			// Try to initialise (do not register now)
			CPlugin* actual_plugin = NULL;
			if (plugin->InitPlugin(actual_plugin) && actual_plugin)
			{
				// Add to list
				mList.push_back(actual_plugin);

				// Check for special types
				switch(actual_plugin->GetType())
				{
				case CPlugin::ePluginAddressIO:
					AddAdbkIOPluginUI(static_cast<CAdbkIOPlugin*>(actual_plugin));
					break;
				case CPlugin::ePluginSpelling:
					// Only allow CocoaSpeller plugin with this version
					if (!HasSpelling() && actual_plugin->GetName().compare_start("CocoaSpeller"))
						mSpellingPlugin = static_cast<CSpellPlugin*>(actual_plugin);
					break;
				case CPlugin::ePluginSecurity:
					if (!HasSecurity())
						mHasSecurityPlugin = true;
					CSecurityPlugin::RegisterSecurityPlugin(static_cast<CSecurityPlugin*>(actual_plugin));
					break;
				case CPlugin::ePluginPswdChange:
					mCountPswdChange++;
					break;
				case CPlugin::ePluginMailMerge:
					if (!HasMailMerge())
						mMailMergePlugin = actual_plugin;
					break;
				case CPlugin::ePluginConfig:
					if (!HasConfig())
						mConfigPlugin = static_cast<CConfigPlugin*>(actual_plugin);
					break;
				case CPlugin::ePluginSSL:
					if (!HasSSL())
					{
						// Must test that this loads OK
						CSSLPlugin* temp = static_cast<CSSLPlugin*>(actual_plugin);
						{
							StLoadPlugin _load(temp);
							mSSLPlugin = static_cast<CSSLPlugin*>(actual_plugin);
						}
						
						// Did not load (could not resolve DLL symbols properly)
						// remove any references to it
						if (!temp)
						{
							mList.pop_back();
							delete actual_plugin;
						}
						
						// SSL plugin must now always be loaded as we need access to certificates even
						// before network connections are made
						mSSLPlugin->InitSSL();
					}
					break;
				default:
					break;
				}
			}

			// Done with original plugin
			delete plugin;
			plugin = NULL;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up
			delete plugin;
		}
	}
}
#elif __dest_os == __win32_os
void CPluginManager::LookforPlugins(void)
{
	// Get Plug-ins folder
	if (GetPluginDirs().empty())
		return;
	cdstring dir = GetPluginDirs().front();

	// Iterate over all files in directory looking for shared libraries
	WIN32_FIND_DATA fileData;
	cdstring search_dir = dir;
	::addtopath(search_dir, cPluginSearch);
	HANDLE hSearch = ::FindFirstFile(search_dir.win_str(), &fileData);
	bool loop = (hSearch != INVALID_HANDLE_VALUE);

	while(loop)
	{
		// Got a plug-in!
		CPlugin* plugin = NULL;
		try
		{
			// Create new plugin
			cdstring pname = dir;
			::addtopath(pname, fileData.cFileName);

			// Check for duplicate name due to dll renaming hack
			bool dup_rename = false;
			for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
				if ((*iter)->GetFName() == pname)
				{
					dup_rename = true;
					break;
				}

			if (!dup_rename)
			{
				// Create new plugin
				plugin = new CPlugin(pname);

				// Try to initialise (do not register now)
				CPlugin* actual_plugin = NULL;
				if (plugin->InitPlugin(actual_plugin) && actual_plugin)
				{
					// Add to list
					mList.push_back(actual_plugin);

					// Check for special types
					switch(actual_plugin->GetType())
					{
					case CPlugin::ePluginAddressIO:
						AddAdbkIOPluginUI(static_cast<CAdbkIOPlugin*>(actual_plugin));
						break;
					case CPlugin::ePluginSpelling:
						// Only allow Spellex plugin with this version
						if (!HasSpelling() && actual_plugin->GetName().compare_start("Spellex"))
							mSpellingPlugin = static_cast<CSpellPlugin*>(actual_plugin);
						break;
					case CPlugin::ePluginSecurity:
						if (!HasSecurity())
							mHasSecurityPlugin = true;
						CSecurityPlugin::RegisterSecurityPlugin(static_cast<CSecurityPlugin*>(actual_plugin));
						break;
					case CPlugin::ePluginPswdChange:
						mCountPswdChange++;
						break;
					case CPlugin::ePluginMailMerge:
						if (!HasMailMerge())
							mMailMergePlugin = actual_plugin;
						break;
					case CPlugin::ePluginConfig:
						if (!HasConfig())
							mConfigPlugin = static_cast<CConfigPlugin*>(actual_plugin);
						break;
					case CPlugin::ePluginSSL:
						if (!HasSSL())
						{
							// Must test that this loads OK
							CSSLPlugin* temp = static_cast<CSSLPlugin*>(actual_plugin);
							{
								StLoadPlugin _load(temp);
								mSSLPlugin = static_cast<CSSLPlugin*>(actual_plugin);
							}
							
							// Did not load (could not resolve DLL symbols properly)
							// remove any references to it
							if (!temp)
							{
								mList.pop_back();
								delete actual_plugin;
							}
							
							// SSL plugin must now always be loaded as we need access to certificates even
							// before network connections are made
							mSSLPlugin->InitSSL();
						}
						break;
					default:
						break;
					}
				}

				// Done with original plugin
				delete plugin;
				plugin = NULL;
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up
			delete plugin;
			plugin = NULL;
		}

		// Do next search
		if (!::FindNextFile(hSearch, &fileData))
		{
			if (::GetLastError() == ERROR_NO_MORE_FILES)
				loop = false;
			else
			{
				long err_last = ::GetLastError();
				CLOG_LOGTHROW(CGeneralException, err_last);
				throw CGeneralException(err_last);
			}
		}
	}

	if ((hSearch != INVALID_HANDLE_VALUE) && !::FindClose(hSearch))
	{
		long err_last = ::GetLastError();
		CLOG_LOGTHROW(CGeneralException, err_last);
		throw CGeneralException(err_last);
	}
}
#elif __dest_os == __linux_os

void CPluginManager::LookforPlugins(void)
{
	// Loop over all plugin dirs in turn
	cdstrset previous;
	for(cdstrvect::const_iterator iter = GetPluginDirs().begin(); iter != GetPluginDirs().end(); iter++)
	{
		LookforPlugins(*iter, previous);
	}

}

void CPluginManager::LookforPlugins(fspec plugindir, cdstrset& previous)
{
	// Must check for directory - not file
	if (!JDirectoryReadable(plugindir))
		return;

	// Iterate over all files in directory looking for shared libraries
	diriterator iter(plugindir);
	const char* fname = NULL;
	while(iter.next(&fname))
	{
		// Make sure this hasn't been seen before in another directory
		if (previous.count(fname))
			// Ignore duplicates
			continue;
		else
			previous.insert(fname);

		// Got a plug-in!
		CPlugin* plugin = NULL;
		try
		{
			// Create new plugin
			JString pname = JCombinePathAndName(plugindir, fname);

			// Create new plugin
			plugin = new CPlugin(pname);
			
			// Try to initialise (do not register now)
			CPlugin* actual_plugin = NULL;
			if (plugin->InitPlugin(actual_plugin) && actual_plugin)
			{
				// Add to list
				mList.push_back(actual_plugin);
				
				// Check for special types
				switch(actual_plugin->GetType())
				{
				case CPlugin::ePluginAddressIO:
					AddAdbkIOPluginUI(static_cast<CAdbkIOPlugin*>(actual_plugin));
					break;
				case CPlugin::ePluginSpelling:
					if (!HasSpelling())
						mSpellingPlugin = static_cast<CSpellPlugin*>(actual_plugin);
					
					// Prefer ASpell over PSpell/ISpell if both present
					else if (plugin->GetName() == "ASpell Plugin")
						mSpellingPlugin = static_cast<CSpellPlugin*>(actual_plugin);
					
					// Prefer PSpell over ISpell if both present
					else if ((plugin->GetName() == "PSpell Plugin") &&
							 (mSpellingPlugin->GetName() == "ISpell Plugin"))
						mSpellingPlugin = static_cast<CSpellPlugin*>(actual_plugin);
					break;
				case CPlugin::ePluginSecurity:
					if (!HasSecurity())
						mHasSecurityPlugin = true;
					CSecurityPlugin::RegisterSecurityPlugin(static_cast<CSecurityPlugin*>(actual_plugin));
					break;
				case CPlugin::ePluginPswdChange:
					mCountPswdChange++;
					break;
				case CPlugin::ePluginMailMerge:
					if (!HasMailMerge())
						mMailMergePlugin = actual_plugin;
					break;
				case CPlugin::ePluginConfig:
					if (!HasConfig())
						mConfigPlugin = static_cast<CConfigPlugin*>(actual_plugin);
					break;
				case CPlugin::ePluginSSL:
					if (!HasSSL())
					{
						// Must test that this loads OK
						CSSLPlugin* temp = static_cast<CSSLPlugin*>(actual_plugin);
						{
							StLoadPlugin _load(temp);
							mSSLPlugin = static_cast<CSSLPlugin*>(actual_plugin);
						}
						
						// Did not load (could not resolve DLL symbols properly)
						// remove any references to it
						if (!temp)
						{
							mList.pop_back();
							delete actual_plugin;
						}
						
						// SSL plugin must now always be loaded as we need access to certificates even
						// before network connections are made
						mSSLPlugin->InitSSL();
					}
					break;
				default:
					break;
				}
			}
			
			// Done with original plugin
			delete plugin;
			plugin = NULL;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up
			delete plugin;
		}


	}//end while
}

#else
#error __dest_os
#endif

// Clean-up
void CPluginManager::CleanPlugins()
{
	// Clear all plugins
	for(CPluginList::iterator iter = mList.begin(); iter != mList.end(); iter++)
		delete *iter;
	mList.clear();
}

// Create actual plugin type
CPlugin* CPluginManager::CreateActualPluginType(CPlugin* original) const
{
	// Determine type
	CPlugin* plugin = NULL;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	fspec plugin_spec = &original->GetSpec();
#else
	fspec plugin_spec = original->GetFName().c_str();
#endif
	switch(original->GetType())
	{
	case CPlugin::ePluginAddressIO:
		// Replace with correct type
		plugin = new CAdbkIOPlugin(plugin_spec);
		break;
	case CPlugin::ePluginNetworkAuthentication:
		// Replace with correct type
		plugin = new CAuthPlugin(plugin_spec);
		break;
	case CPlugin::ePluginSecurity:
		// Replace with correct type
		plugin = new CSecurityPlugin(plugin_spec);
		break;
	case CPlugin::ePluginSpelling:
		// Replace with correct type
		plugin = new CSpellPlugin(plugin_spec);
		break;
	case CPlugin::ePluginPswdChange:
		// Replace with correct type
		plugin = new CPswdChangePlugin(plugin_spec);
		break;
	case CPlugin::ePluginConfig:
		// Replace with correct type
		plugin = new CConfigPlugin(plugin_spec);
		break;
	case CPlugin::ePluginSSL:
		// Replace with correct type
		plugin = new CSSLPlugin(plugin_spec);
		break;
	default:
		break;
	}
	
	// Now replace original
	if (plugin && original)
		plugin->ReplacePlugin(original);

	return plugin;
}

// Get names of all auth plugins
void CPluginManager::GetAuthPlugins(cdstrvect& names) const
{
	names.clear();

	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if ((*iter)->GetType() == CPlugin::ePluginNetworkAuthentication)
			names.push_back(static_cast<CAuthPlugin*>(*iter)->GetPrefsDescriptor());
	}
}

CAuthPlugin* CPluginManager::FindAuthPlugin(const char* auth_id) const
{
	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if (((*iter)->GetType() == CPlugin::ePluginNetworkAuthentication) &&
			(static_cast<CAuthPlugin*>(*iter)->GetAuthTypeID() == auth_id))
			return static_cast<CAuthPlugin*>(*iter);
	}

	return NULL;
}

CAuthPlugin* CPluginManager::GetAuthPlugin(const char* prefs_id) const
{
	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if (((*iter)->GetType() == CPlugin::ePluginNetworkAuthentication) &&
			(static_cast<CAuthPlugin*>(*iter)->GetPrefsDescriptor() == prefs_id))
			return static_cast<CAuthPlugin*>(*iter);
	}

	return NULL;
}

CAdbkIOPlugin* CPluginManager::FindAdbkIOPlugin(const char* name) const
{
	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if (((*iter)->GetType() == CPlugin::ePluginAddressIO) &&
			(::strcmp(static_cast<CAdbkIOPlugin*>(*iter)->GetUIName(), name) == 0))
			return static_cast<CAdbkIOPlugin*>(*iter);
	}

	return NULL;
}

void CPluginManager::UpdatePswdChange()
{
	// Update count of enabled plugins
	mCountPswdChange = 0;
	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if (((*iter)->GetType() == CPlugin::ePluginPswdChange) &&
			static_cast<CPswdChangePlugin*>(*iter)->Enabled())
			mCountPswdChange++;
	}
}

CPswdChangePlugin* CPluginManager::FindPswdChangePlugin(const char* name) const
{
	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if (((*iter)->GetType() == CPlugin::ePluginPswdChange) &&
			(::strcmp(static_cast<CPswdChangePlugin*>(*iter)->GetUIName(), name) == 0))
			return static_cast<CPswdChangePlugin*>(*iter);
	}

	return NULL;
}

#pragma mark ____________________________Preferences

// Write data to a stream
void CPluginManager::WriteToMap(COptionsMap* theMap, bool dirty_only)
{
	StMapSection section(theMap, cPluginsSection);

	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
		(*iter)->WriteToMap(theMap, dirty_only);
}

// Read data from a stream
bool CPluginManager::ReadFromMap(COptionsMap* theMap, NumVersion vers_app, NumVersion& vers_prefs)
{
	StMapSection section(theMap, cPluginsSection);
	bool result = false;

	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
		result = (*iter)->ReadFromMap(theMap, vers_app, vers_prefs) | result;

	// Must update number of available pswd change plugins
	UpdatePswdChange();

	return result;
}

#if __dest_os == __win32_os
// Sync menu with AdbkIOs
void CPluginManager::SyncAdbkIOPluginMenu(CMenu* import_menu, CMenu* export_menu) const
{
	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if ((*iter)->GetType() == CPlugin::ePluginAddressIO)
		{
			if (static_cast<CAdbkIOPlugin*>(*iter)->DoesImport())
			{
				// Check for very first one
				if (import_menu->GetMenuItemCount() == 1)
				{
					CString name;
					import_menu->GetMenuString(IDM_AddressImportStart, name, MF_BYCOMMAND);
					if (name.IsEmpty())
						import_menu->DeleteMenu(IDM_AddressImportStart, MF_BYCOMMAND);
				}
				CUnicodeUtils::AppendMenuUTF8(import_menu, MF_STRING, IDM_AddressImportStart + import_menu->GetMenuItemCount(), static_cast<CAdbkIOPlugin*>(*iter)->GetUIName());
			}

			if (static_cast<CAdbkIOPlugin*>(*iter)->DoesExport())
			{
				// Check for very first one
				if (export_menu->GetMenuItemCount() == 1)
				{
					CString name;
					export_menu->GetMenuString(IDM_AddressExportStart, name, MF_BYCOMMAND);
					if (name.IsEmpty())
						export_menu->DeleteMenu(IDM_AddressExportStart, MF_BYCOMMAND);
				}
				CUnicodeUtils::AppendMenuUTF8(export_menu, MF_STRING, IDM_AddressExportStart + export_menu->GetMenuItemCount(), static_cast<CAdbkIOPlugin*>(*iter)->GetUIName());
			}
		}
	}
}
#elif __dest_os == __linux_os
// Sync menu with AdbkIOs
void CPluginManager::SyncAdbkIOPluginMenu(JXTextMenu* import_menu, JXTextMenu* export_menu) const
{
	cdstring importers;
	cdstring exporters;

	for(CPluginList::const_iterator iter = mList.begin(); iter != mList.end(); iter++)
	{
		if ((*iter)->GetType() == CPlugin::ePluginAddressIO)
		{
			if (static_cast<CAdbkIOPlugin*>(*iter)->DoesImport())
			{
				if (!importers.empty())
					importers += "|";
				importers += static_cast<CAdbkIOPlugin*>(*iter)->GetUIName();
			}

			if (static_cast<CAdbkIOPlugin*>(*iter)->DoesExport())
			{
				if (!exporters.empty())
					exporters += "|";
				exporters += static_cast<CAdbkIOPlugin*>(*iter)->GetUIName();
			}
		}
	}

	// Now add to menus
	if (importers.length())
		import_menu->SetMenuItems(importers);
	if (exporters.length())
		export_menu->SetMenuItems(exporters);
}
#endif

// Add plugin to menu
void CPluginManager::AddAdbkIOPluginUI(CAdbkIOPlugin* plugin) const
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	LStr255 title(plugin->GetUIName());

	if (plugin->DoesImport())
	{
		LMenu* import_menu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_ImportAddresses);
		::AppendMenu(import_menu->GetMacMenuH(), (unsigned char*) title);
	}

	if (plugin->DoesExport())
	{
		LMenu* export_menu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_ExportAddresses);
		::AppendMenu(export_menu->GetMacMenuH(), (unsigned char*) title);
	}
#elif __dest_os == __win32_os
	// No need to do anything. Each time the Addresses top-level menu is created
	// in a menu bar it will call SyncAdbkIOPluginMenu to add the plugins.

#if 0
	if (plugin->DoesImport())
	{
		CMenu* menu = AfxGetApp()->m_pMainWnd->GetMenu();
		CMenu* import_menu = menu->GetSubMenu(5)->GetSubMenu(6);

		// Check for very first one
		if (import_menu->GetMenuItemCount() == 1)
		{
			CString name;
			import_menu->GetMenuString(IDM_AddressImportStart, name, MF_BYCOMMAND);
			if (name.IsEmpty())
				import_menu->DeleteMenu(IDM_AddressImportStart, MF_BYCOMMAND);
		}
		import_menu->AppendMenu(MF_STRING, IDM_AddressImportStart + import_menu->GetMenuItemCount(), plugin->GetUIName());
	}

	if (plugin->DoesExport())
	{
		CMenu* menu = AfxGetApp()->m_pMainWnd->GetMenu();
		CMenu* export_menu = menu->GetSubMenu(5)->GetSubMenu(7);

		// Check for very first one
		if (export_menu->GetMenuItemCount() == 1)
		{
			CString name;
			export_menu->GetMenuString(IDM_AddressExportStart, name, MF_BYCOMMAND);
			if (name.IsEmpty())
				export_menu->DeleteMenu(IDM_AddressExportStart, MF_BYCOMMAND);
		}
		export_menu->AppendMenu(MF_STRING, IDM_AddressExportStart + export_menu->GetMenuItemCount(), plugin->GetUIName());
	}
#endif
#elif __dest_os == __linux_os
	// No need to do anything. Each time the Addresses top-level menu is created
	// in a menu bar it will call SyncAdbkIOPluginMenu to add the plugins.
#else
#error __dest_os
#endif
}
