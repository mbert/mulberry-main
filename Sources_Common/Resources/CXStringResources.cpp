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
	CXStringResources.cpp

	Author:			
	Description:	<describe the CXStringResources class here>
*/

#include "CXStringResources.h"

#include "CLocalCommon.h"

#ifdef __MULBERRY
#include "XMLDocument.h"
#include "XMLSAXSimple.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x

#include "MyCFString.h"
#include <PPxPrimaryBundle.h>
#include <SysCFURL.h>

#elif __dest_os == __win32_os

#include "CConnectionManager.h"

#elif __dest_os == __linux_os

#include "CConnectionManager.h"
#include "CLocalCommon.h"

#include <JString.h>
#include <jDirUtil.h>

#endif

// The DTD for the XML resource file is:
/*

	<!ELEMENT stringlist	(string* stringarray*)>
	<!ELEMENT stringarray	(string*)>
	<!ATTLIST stringarray	id	PCDATA	#REQUIRED>
	<!ELEMENT string		(#PCDATA)>
	<!ATTLIST string		id	PCDATA	#REQUIRED>	// Required in top-level string, not in stringarray

*/

using namespace rsrc; 
#ifdef __MULBERRY
using namespace xmllib; 
#endif

CXStringResources CXStringResources::sStringResources;
cdstring CXStringResources::sEmptyString;

const char cResourcesDirName[] = "Resources";

void CXStringResources::LoadAllStrings()
{
	InitRsrcPaths();
	rsrc::CXStringResources::sStringResources.LoadStrings("Alerts.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("ComponentDescriptions.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("DateTimeStrings.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("ErrorDialog.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("MacOS.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("Preferences.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("ProtocolErrors.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("Status.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("Strings.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("Toolbars.xml");
	rsrc::CXStringResources::sStringResources.LoadStrings("UI.xml");
	
	mInitialised = true;
}

void CXStringResources::LoadStrings(const cdstring& rsrc_path)
{
	// Iterate over each path in turn, this will add most specific to least specific localised
	// strings to the map
	for(cdstrvect::const_iterator iter = mPaths.begin(); iter != mPaths.end(); iter++)
	{
		cdstring fpath(*iter);
		::addtopath(fpath, rsrc_path);
		if (::fileexists(fpath))
			LoadStringsFile(fpath);
	}
}

void CXStringResources::LoadStringsFile(const cdstring& file_path)
{
#ifdef __MULBERRY
	// XML parse the data
	XMLSAXSimple parser;
	parser.ParseFile(file_path.c_str());

	// See if we got any valid XML
	if (parser.Document())
	{
		// Check root node
		XMLNode* root = parser.Document()->GetRoot();
		if (root->Name() != "stringlist")
			return;
		
		// Now look at each child
		for(XMLNodeList::const_iterator iter = root->Children().begin(); iter != root->Children().end(); iter++)
		{
			// Check child name
			XMLNode* child = *iter;
			if (child->Name() == "string")
			{
				// Check attribute
				const XMLAttribute* attrib = child->Attribute("id");
				if (attrib == NULL)
					continue;
				
				// Make sure c-escapes are expanded and that line ends match the OS
				cdstring str(child->Data());
				str.FilterOutEscapeChars();
				str.ConvertEndl();

				// Add to map
				mStrings.insert(CStringResourceMap::value_type(attrib->Value(), str));
			}
			else if (child->Name() == "stringarray")
			{
				// Check attribute
				const XMLAttribute* attrib = child->Attribute("id");
				if (attrib == NULL)
					continue;
				
				// Now look at each child
				cdstrvect items;
				for(XMLNodeList::const_iterator iter2 = child->Children().begin(); iter2 != child->Children().end(); iter2++)
				{
					// Check child name
					XMLNode* item = *iter2;
					if (item->Name() == "string")
					{
						// Make sure c-escapes are expanded and that line ends match the OS
						cdstring str(item->Data());
						str.FilterOutEscapeChars();
						str.ConvertEndl();

						// Add to list
						items.push_back(str);
					}
				}

				// Add to map
				mIndexedStrings.insert(CIndexedStringResourceMap::value_type(attrib->Value(), items));
			}
		}
	}
#endif
}

const cdstring& CXStringResources::GetString(const char* rsrc_id) const
{
	if (!mInitialised)
		const_cast<CXStringResources*>(this)->LoadAllStrings();

	if ((rsrc_id == NULL) || (*rsrc_id == 0))
		return sEmptyString;
	
	// Look up in the map
	CStringResourceMap::const_iterator found = mStrings.find(rsrc_id);
	if (found != mStrings.end())
		return (*found).second;
	else
		return sEmptyString;
}

const cdstring& CXStringResources::GetIndexedString(const char* rsrc_id, uint32_t index) const
{
	if (!mInitialised)
		const_cast<CXStringResources*>(this)->LoadAllStrings();

	if ((rsrc_id == NULL) || (*rsrc_id == 0))
		return sEmptyString;
	
	// Look up in the map
	CIndexedStringResourceMap::const_iterator found = mIndexedStrings.find(rsrc_id);
	if (found != mIndexedStrings.end())
	{
		// Verify index is in range
		const cdstrvect& items = (*found).second;
		if (index < items.size())
			return items[index];
	}
	
	return sEmptyString;
}

// Get the full  path to the required resoyurce file
// Each OS will do this differently based on the appropriate paths
void CXStringResources::InitRsrcPaths()
{
	if (mPaths.empty())
	{
		// Get the path to the resources directory
		cdstring resources;
		
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		{
			PPx::CFURL url = PPx::PrimaryBundle::Instance().GetBundleURL();
			MyCFString path = url.GetFileSystemPath();
			resources = path.GetString();
			resources += "/";
		}
		{
			PPx::CFURL url = PPx::PrimaryBundle::Instance().GetResourcesDirectoryURL();
			MyCFString path = url.GetFileSystemPath();
			cdstring temp = path.GetString();
			temp += "/";
			resources += temp;
		}
#elif __dest_os == __win32_os
		// Use Plug-in path/Resources directory
		resources = CConnectionManager::sConnectionManager.GetApplicationCWD();
		::addtopath(resources, cResourcesDirName);
#elif __dest_os == __linux_os
		// Try users home directory first
		JString rsrcdir;
		JGetHomeDirectory(&rsrcdir);
		resources = rsrcdir.GetCString();
		::addtopath(resources, ".mulberry");
		::addtopath(resources, cResourcesDirName);
		if (!::direxists(resources))
			resources = cdstring::null_str;

		// Try application directory next
		if (resources.empty())			
		{
			resources.reserve(FILENAME_MAX);
			if (::getcwd(resources, FILENAME_MAX) != NULL)
			{
				::addtopath(resources, cResourcesDirName);
				if (!::direxists(resources))
					resources = cdstring::null_str;
			}
		}

		// Then /usr/local/lib/mulberry
		if (resources.empty())			
		{
			resources = "/usr/local/lib/mulberry";
			::addtopath(resources, cResourcesDirName);
			if (!::direxists(resources))
				resources = cdstring::null_str;
		}

		// Then /usr/lib/mulberry
		if (resources.empty())			
		{
			resources = "/usr/lib/mulberry";
			::addtopath(resources, cResourcesDirName);
			if (!::direxists(resources))
				resources = cdstring::null_str;
		}
#endif

		// Determine the name of the locale
		cdstring locale;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		CFLocaleRef cflocale = ::CFLocaleCopyCurrent();
		if (cflocale != NULL)
		{
			MyCFString cfstr(::CFLocaleGetIdentifier(cflocale));
			locale = cfstr.GetString();
			
			::CFRelease(cflocale);
		}

#elif __dest_os == __win32_os

#elif __dest_os == __linux_os

		// Use $LANG for locale, but strip off ".UTF8" etc
		const char* lang = ::getenv("LANG");
		if (lang != NULL)
		{
			locale.assign(lang, std::min(::strlen(lang), (size_t)5));
		}
		
#endif

		// Now add paths for most specific to least specific locales
		while(!locale.empty())
		{
			cdstring rpath(resources);
			::addtopath(rpath, locale);
			mPaths.push_back(rpath);
			
			// Now chop of most specific part
			cdstring::size_type pos = locale.find_last_of('_');
			if (pos != cdstring::npos)
				locale.erase(pos);
			else
				locale.clear();
		}
		
		// Now add top level path
		mPaths.push_back(resources);
	}
}
