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


// Source for CURLHelpersMap class

#include "CURLHelpersMap.h"

#include "cdfstream.h"

#include <JSimpleProcess.h>
#include <JString.h>
#include <JThisProcess.h>
#include <jFileUtil.h>
#include <jDirUtil.h>

#include <JXHelpManager.h>
#include <jXGlobals.h>

CURLHelpersMap CURLHelpersMap::sURLHelpersMap;

void CURLHelpersMap::LoadDefaults(const cdstring& path)
{
	// Try these files in this order to make sure overrides succeed
	// /usr/lib/url.helpers
	// /usr/local/lib/url.helpers
	// /etc/url.helpers
	// extra path
	// ~/url.helpers

	const char* cUsrLib = "/usr/lib/url.helpers";
	const char* cUsrLocalLib = "/usr/local/lib/url.helpers";
	const char* cEtc = "/etc/url.helpers";
	const char* cUser = ".url.helpers";

	if (JFileExists(cUsrLib))
	{
		cdifstream itype(cUsrLib);
		sURLHelpersMap.ReadFromStream(itype);
	}

	if (JFileExists(cUsrLocalLib))
	{
		cdifstream itype(cUsrLocalLib);
		sURLHelpersMap.ReadFromStream(itype);
	}

	if (JFileExists(cEtc))
	{
		cdifstream itype(cEtc);
		sURLHelpersMap.ReadFromStream(itype);
	}

	if (!path.empty())
	{
		cdstring extra(path);
		extra += "url.helpers";
		if (JFileExists(extra))
		{
			cdifstream itype(extra);
			sURLHelpersMap.ReadFromStream(itype);
		}
	}

	JString defaultFile;
	JGetHomeDirectory(&defaultFile);
	defaultFile += cUser;
	if (JFileExists(defaultFile))
	{
		cdifstream itype(defaultFile);
		sURLHelpersMap.ReadFromStream(itype);
	}
	
	// Setup the JXHelpManager url helper
	SetupHelp();
}

bool CURLHelpersMap::LaunchURL(const char* url)
{
	// Tokenize to get scheme
	cdstring scheme;
	const char* p = ::strchr(url, ':');
	if (p)
		scheme.assign(url, p - url);
	else
		// Use fake http scheme if none present in url
		scheme = "http";
		

	// Lookup scheme
	cdstrmap::iterator found = sURLHelpersMap.mHelpers.find(scheme);

	// If explicit map not found try wildcard
	if (found == sURLHelpersMap.mHelpers.end())
		found = sURLHelpersMap.mHelpers.find(cdstring("*"));

	if (found != sURLHelpersMap.mHelpers.end())
	{
		// Get launch spec
		cdstring launch = found->second;

		// Now snprintf the URL into the spec
		cdstring cmd;
		size_t cmd_reserve = launch.length() + ::strlen(url);
		cmd.reserve(cmd_reserve);
		::snprintf(cmd.c_str_mod(), cmd_reserve, launch, url);

		// Now shell execute this
		JSimpleProcess* execd = NULL;
		const JError err = JSimpleProcess::Create(&execd, cmd, kTrue);
		if (err.OK())
		{
			JThisProcess::Ignore(execd);
			return true;
		}
	}

	// If we got here the URL was not launched
	return false;
}

void CURLHelpersMap::SetupHelp()
{
	// Locate an http url
	cdstring scheme("http");
	cdstring url("$u");			// JXHelpManager substitute item

	// Lookup scheme
	cdstrmap::iterator found = sURLHelpersMap.mHelpers.find(scheme);

	// If explicit map not found try wildcard
	if (found == sURLHelpersMap.mHelpers.end())
		found = sURLHelpersMap.mHelpers.find(cdstring("*"));

	if (found != sURLHelpersMap.mHelpers.end())
	{
		// Get launch spec
		cdstring launch = found->second;

		// Now snprintf the URL into the spec
		cdstring cmd;
		size_t cmd_reserve = launch.length() + ::strlen(url);
		cmd.reserve(cmd_reserve);
		::snprintf(cmd.c_str_mod(), cmd_reserve, launch, url.c_str());
		
		// Now set this command as the JXHelpManager view item
		//JXGetHelpManager()->SetViewURLCmd(cmd);	
	}
}

void CURLHelpersMap::ReadFromStream(std::istream& ins)
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

		// Parse line into scheme and lauch spec
		cdstring scheme = p;
		p = ::strtok(NULL, "");
		cdstring launch;
		if (p)
			launch = p;
		launch.trimspace();

		// Add to map
		std::pair<cdstrmap::iterator, bool> ins = mHelpers.insert(std::make_pair(scheme, launch));
		if (!ins.second)
		{
			// Already present - replace with the new one
			ins.first->second = launch;
		}
	}
}
