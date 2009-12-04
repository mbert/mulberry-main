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


// Source for CMIMETypesMap class

#include "CMIMETypesMap.h"

#include "CStringUtils.h"

#include "cdfstream.h"

#include <JString.h>
#include <jFileUtil.h>
#include <jDirUtil.h>

#include <algorithm>

CMIMETypesMap CMIMETypesMap::sMIMETypesMap;

void CMIMETypesMap::LoadDefaults(const cdstring& path)
{
	// Try these files in this order to make sure overrides succeed
	// /usr/lib/mime.types
	// /usr/local/lib/mime.types
	// /etc/mime.types
	// extra path
	// ~/mime.types

	const char* cUsrLib = "/usr/lib/mime.types";
	const char* cUsrLocalLib = "/usr/local/lib/mime.types";
	const char* cEtc = "/etc/mime.types";
	const char* cUser = ".mime.types";

	if (JFileExists(cUsrLib))
	{
		cdifstream itype(cUsrLib);
		sMIMETypesMap.ReadFromStream(itype);
	}

	if (JFileExists(cUsrLocalLib))
	{
		cdifstream itype(cUsrLocalLib);
		sMIMETypesMap.ReadFromStream(itype);
	}

	if (JFileExists(cEtc))
	{
		cdifstream itype(cEtc);
		sMIMETypesMap.ReadFromStream(itype);
	}

	if (!path.empty())
	{
		cdstring extra(path);
		extra += "mime.types";
		if (JFileExists(extra))
		{
			cdifstream itype(extra);
			sMIMETypesMap.ReadFromStream(itype);
		}
	}

	JString defaultFile;
	JGetHomeDirectory(&defaultFile);
	defaultFile += cUser;
	if (JFileExists(defaultFile))
	{
		cdifstream itype(defaultFile);
		sMIMETypesMap.ReadFromStream(itype);
	}

	// Add these so they override
	sMIMETypesMap.AddDefaults();
}

const cdstring& CMIMETypesMap::GetMIMEType(const cdstring& extension)
{
	cdstrmap::iterator found = mExtToType.find(extension);
	if (found != mExtToType.end())
		return found->second;

	return cdstring::null_str;
}

const cdstring& CMIMETypesMap::GetExtension(const cdstring& MIMEType, const cdstring& original)
{
	MIMETypeInfoMap::iterator found = mTypeToInfo.find(MIMEType);
	if (found != mTypeToInfo.end())
	{
		// Check to see whether original extension is in the matching list
		if (found->second->extensions.size())
		{
			std::list<cdstring>::const_iterator found2 = std::find(found->second->extensions.begin(),
															found->second->extensions.end(), original);

			// Not in list and not appliucation/octet-stream => use first extension in list
			if ((found2 == found->second->extensions.end()) &&
				(::strcmpnocase(MIMEType, "application/octet-stream")))
				return found->second->extensions.front();
		}
	}

	// use original extension
	return original;
}

void CMIMETypesMap::ReadFromStream(std::istream& ins)
{
	while (ins)
	{
		cdstring line;
		cdstring nextline;
		bool netscape = false;

		getline(ins, line);
		if (line.length() == 0) continue;

		while (line[line.length() - 1] == '\\')
		{
			getline(ins, nextline);
			if (nextline.length() == 0) break;
			line[line.length() - 1] = 0;
			line += nextline;
		}
		if (line[0UL] == '#') continue;

		// tokenize the line up
		const char* p = ::strtok(line.c_str_mod(), " \t");
		if (!p)
			//invalid line, skip it
			continue;

		netscape = (::strchr(p, '=') != NULL);
		cdstring type, extlist, desc;
		if (netscape)
		{
		}
		else
		{
			type = p;
			p = ::strtok(NULL, "");
			if (p)
				extlist = p;
			extlist.trimspace();
			desc = "";
		}
		AddEntry(type, extlist, desc);
	}
}

void CMIMETypesMap::AddEntry(const cdstring& mimetype, const cdstring& extensions,
														 const cdstring& description)
{
	MIMETypeInfo* mti = new MIMETypeInfo;
	mti->description = description;

	// Tokenize extensions list
	cdstring temp(extensions);
	const char* p = ::strtok(temp.c_str_mod(), " \t,");
	while(p)
	{
		cdstring ext(p);
		ext.trimspace();
		ext = cdstring(".") + ext;

		// Add to list of extensions
		mti->extensions.push_back(ext);

		// Add to reverse lookup list
		mExtToType.insert(std::make_pair(ext, mimetype));

		// Get next token
		p = ::strtok(NULL, " \t,");
	}

	// Add entry to list
	std::pair<MIMETypeInfoMap::iterator, bool> res = mTypeToInfo.insert(std::make_pair(mimetype, mti));
	if (!res.second)
	{
		//it was already there, delete old info and replace it
		delete res.first->second;
		res.first->second = mti;
	}
}
	
void CMIMETypesMap::AddDefaults()
{
	AddEntry("text/plain", "txt", "");
	AddEntry("text/html", "html", "");
}

