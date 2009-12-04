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


// Source for CMailcapMap class

// cd: 18-Jan-2001
// This class maintains a mapping of MIME types to commands for doing
// application launching of saved MIME attachments.
// It parses the UNIX mailcap file (RFC1524) from various locations
// (if present) to create the map.
// The caller is responsible for doing any '%s' substitutions in the
// command.

#include "CMailcapMap.h"

#include "CStringUtils.h"

#include "cdfstream.h"

#include <JString.h>
#include <jFileUtil.h>
#include <jDirUtil.h>

CMailcapMap CMailcapMap::sMailcapMap;

// LoadDefaults : read mailcap files into our cache
// 
void CMailcapMap::LoadDefaults(const cdstring& path)
{
	// Try these files in this order to make sure first item found
	// is the one used if there are duplicate entries
	//
	// ~/mailcap
	// extra path
	// /etc/mailcap
	// /usr/local/lib/mailcap
	// /usr/lib/mailcap

	const char* cUser = ".mailcap";
	const char* cEtc = "/etc/mailcap";
	const char* cUsrLocalLib = "/usr/local/etc/mailcap";
	const char* cUsrLib = "/usr/etc/mailcap";

	JString defaultFile;
	JGetHomeDirectory(&defaultFile);
	defaultFile += cUser;
	if (JFileExists(defaultFile))
	{
		cdifstream itype(defaultFile);
		sMailcapMap.ReadFromStream(itype);
	}

	if (!path.empty())
	{
		cdstring extra(path);
		extra += "mailcap";
		if (JFileExists(extra))
		{
			cdifstream itype(extra);
			sMailcapMap.ReadFromStream(itype);
		}
	}

	if (JFileExists(cEtc))
	{
		cdifstream itype(cEtc);
		sMailcapMap.ReadFromStream(itype);
	}

	if (JFileExists(cUsrLocalLib))
	{
		cdifstream itype(cUsrLocalLib);
		sMailcapMap.ReadFromStream(itype);
	}

	if (JFileExists(cUsrLib))
	{
		cdifstream itype(cUsrLib);
		sMailcapMap.ReadFromStream(itype);
	}
}

// GetAppName : return the name of the application that will be launched for a given MIME type
// type       : MIME type to match
// <return>   : application name
// 
cdstring CMailcapMap::GetAppName(const cdstring& type)
{
	// Get command
	const cdstring& cmd = GetCommand(type);
	if (cmd.empty())
		return cdstring::null_str;

	cdstring temp = cmd;
	char* p = ::strchr(temp.c_str_mod(), ' ');
	if (p)
		*p = 0;

	return temp;
}

// GetCommand : return the command that matches the MIME type
// type       : MIME type to match
// <return>   : command string
// 
const cdstring& CMailcapMap::GetCommand(const cdstring& type)
{
	// Do straight comparison for exact match
	cdstrmap::iterator found = mTypeToApp.find(type);
	if (found != mTypeToApp.end())
		return found->second;

	// No exact match => do pattern match
	for(cdstrmap::const_iterator iter = mTypeToApp.begin(); iter != mTypeToApp.end(); iter++)
	{
		if (::strpmatch(type, (*iter).first))
			return (*iter).second;
	}

	return cdstring::null_str;
}

// ReadFromStream : read lines of data from mailcap file and add them to the mailcap map
// ins            : (file) input stream to read from
// 
void CMailcapMap::ReadFromStream(std::istream& ins)
{
	while (ins)
	{
		// Read one line
		cdstring line;
		getline(ins, line);
		if (line.length() == 0) continue;

		// Check for continuation
		while (line[line.length() - 1] == '\\')
		{
			// Get the next line
			cdstring nextline;
			getline(ins, nextline);

			// Empty next line => ignore it
			if (nextline.length() == 0) break;

			// Append to current line, ignoring continuation
			line[line.length() - 1] = 0;
			line += nextline;
		}

		// Ignore comments
		if (line[0UL] == '#') continue;

		// tokenize the line up
		char* start_tok = line.c_str_mod();
		char* end_tok = ::strchr(start_tok, ';');
		if (!end_tok)
			// invalid line, skip it
			continue;
		*end_tok = 0;

		// Grab the typefield (remove leading/trailing spaces)
		cdstring typefield(start_tok);
		typefield.trimspace();

		// Look for view_command (we must have one)
		start_tok = end_tok + 1;
		end_tok = ::strchr(start_tok, ';');
		if (start_tok == end_tok) continue;
		if (end_tok)
			*end_tok = 0;		

		// Grab the view-command (remove leading/trailing spaces)
		cdstring view_command(start_tok);
		view_command.trimspace();

		// Look for the %s and see if quoted
		const char* p = ::strstr(view_command.c_str(), "%s");
		if (p)
		{
			// Check for quotes either side
			if ((*(p-1) == '"') && (*(p+2) == '"'))
			{
				// Just ignore - quoting is OK
			}
			else if ((*(p-1) == '\'') && (*(p+2) == '\''))
			{
				// Replace single with double-quotes
				*const_cast<char*>(p-1) = '"';
				*const_cast<char*>(p+2) = '"';
			}
			else
			{
				// Add quotes around %s
				cdstring temp(view_command.c_str(), p - view_command.c_str());
				temp += "\"%s\"";
				temp += p+2;
				view_command = temp;
			}
		}

		// Add it to the map if not empty
		if (typefield.length() && view_command.length())
			AddEntry(typefield, view_command);
	}
}

// AddEntry : add an entry into the mailcap map
// mimetype : the mimetype to match
// app      : mailcap view-command data to add
// 
void CMailcapMap::AddEntry(const cdstring& mimetype, const cdstring& app)
{
	// Must be valid mime specification
	if (!::strchr(mimetype.c_str(), '/'))
		return;

	// Add entry to list - this will change an existing entry, only add a new one
	// ensuring that the first entry found is the one used if there are duplicates
	std::pair<cdstrmap::iterator, bool> ins = mTypeToApp.insert(std::make_pair(mimetype, app));
}
