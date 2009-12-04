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

// CVCard.cp
//
// Copyright 2006, Cyrus Daboo.  All Rights Reserved.
//
// Created: 03-Aug-2002
// Author: Cyrus Daboo
// Platforms: Mac OS, Win32, Unix
//
// Description:
// This class implements a vCard object.
//
// History:
// 03-Aug-2002: Created initial header and implementation.
//

#include "CVCard.h"

#include "CStringUtils.h"
#include "quotedprintable.h"

#include <algorithm>
#include <strstream>

CVCardItem& CVCard::AddItem(const cdstring& name, const cdstring& value)
{
	CVCardItems::iterator item = mItems.insert(CVCardItems::value_type(name, CVCardItem(value)));
	return (*item).second;
}

void CVCard::AddItem(const cdstring& name, const CVCardItem& item)
{
	mItems.insert(CVCardItems::value_type(name, item));
}

unsigned long CVCard::CountItems(const cdstring& name)
{
	return mItems.count(name);
}

const cdstring& CVCard::GetValue(const cdstring& name)
{
	CVCardItems::const_iterator found = mItems.find(name);
	if (found != mItems.end())
		return (*found).second.GetValue();
	else
		return cdstring::null_str;
}

unsigned long CVCard::CountItems(const cdstring& name, const cdstring& param_name, const cdstring& param_value)
{
	unsigned long ctr = 0;

	std::pair<CVCardItems::const_iterator, CVCardItems::const_iterator> iters1 = mItems.equal_range(name);
	for(CVCardItems::const_iterator iter1 = iters1.first; iter1 != iters1.second; iter1++)
	{
		pair<cdstrmultimap::const_iterator, cdstrmultimap::const_iterator> iters2 = (*iter1).second.GetParams().equal_range(param_name);
		for(cdstrmultimap::const_iterator iter2 = iters2.first; iter2 != iters2.second; iter2++)
		{
			if (!::strcmpnocase(param_value, (*iter2).second))
				ctr++;
		}
	}

	return ctr;
}

const cdstring& CVCard::GetValue(const cdstring& name, const cdstring& param_name, const cdstring& param_value)
{
	std::pair<CVCardItems::const_iterator, CVCardItems::const_iterator> iters1 = mItems.equal_range(name);
	for(CVCardItems::const_iterator iter1 = iters1.first; iter1 != iters1.second; iter1++)
	{
		pair<cdstrmultimap::const_iterator, cdstrmultimap::const_iterator> iters2 = (*iter1).second.GetParams().equal_range(param_name);
		for(cdstrmultimap::const_iterator iter2 = iters2.first; iter2 != iters2.second; iter2++)
		{
			if (!::strcmpnocase(param_value, (*iter2).second))
				return (*iter1).second.GetValue();
		}
	}

	return cdstring::null_str;
}

unsigned long CVCard::CountItems(const cdstring& name, const cdstrmap& params)
{
	unsigned long ctr = 0;

	std::pair<CVCardItems::const_iterator, CVCardItems::const_iterator> iters1 = mItems.equal_range(name);
	for(CVCardItems::const_iterator iter1 = iters1.first; iter1 != iters1.second; iter1++)
	{
		for(cdstrmap::const_iterator iter2 = params.begin(); iter2 != params.end(); iter2++)
		{
			pair<cdstrmultimap::const_iterator, cdstrmultimap::const_iterator> iters3 = (*iter1).second.GetParams().equal_range((*iter2).first);
			for(cdstrmultimap::const_iterator iter3 = iters3.first; iter3 != iters3.second; iter3++)
			{
				if (!::strcmpnocase((*iter2).second, (*iter3).second))
					ctr++;
			}
		}
	}

	return ctr;
}

const cdstring& CVCard::GetValue(const cdstring& name, const cdstrmap& params)
{
	std::pair<CVCardItems::const_iterator, CVCardItems::const_iterator> iters1 = mItems.equal_range(name);
	for(CVCardItems::const_iterator iter1 = iters1.first; iter1 != iters1.second; iter1++)
	{
		bool foundall = true;
		for(cdstrmap::const_iterator iter2 = params.begin(); iter2 != params.end(); iter2++)
		{
			bool found_param = false;
			pair<cdstrmultimap::const_iterator, cdstrmultimap::const_iterator> iters3 = (*iter1).second.GetParams().equal_range((*iter2).first);
			for(cdstrmultimap::const_iterator iter3 = iters3.first; iter3 != iters3.second; iter3++)
			{
				if (!::strcmpnocase((*iter2).second, (*iter3).second))
				{
					found_param = true;
					break;
				}
			}
			
			if (!found_param)
			{
				foundall = false;
				break;
			}
		}
		
		if (foundall)
			return (*iter1).second.GetValue();
	}

	return cdstring::null_str;
}

// Read a single vCard address from the input stream
bool CVCard::Read(istream& in)
{
	// Read lines looking for starting line
	cdstring line;
	while(true)
	{
		// Get line and trim out whitespace
		::getline(in, line, 0);
		if (in.fail())
			return false;
		line.trimspace();
		
		// Look for begin item
		bool old_version = true;
		if (!::strcmpnocase(line, "BEGIN:VCARD"))
		{
			// vCard allows for folding of long lines
			// Use this to accumulate folded lines into one
			cdstring unfoldedline;

			// Loop until end
			while(true)
			{
				// Get next line
				::getline(in, line, 0);
				if (in.fail())
					return false;
				
				// Look for fold
				if (line.length() && isspace(line[0UL]))
				{
					// Add to existing line
					unfoldedline += &line[1UL];
					
					// Continue with loop
					continue;
				}
				
				// Look for existing line that now needs to be processed
				else if (unfoldedline.length())
				{
					ReadItem(unfoldedline, old_version);
				}
				
				// Just copy the new line into fold
				unfoldedline = line;

				// Look for end item and break out of loop
				line.trimspace();
				if (!::strcmpnocase(line, "END:VCARD"))
					break;
				if (!::strcmpnocase(line, "VERSION:3.0"))
					old_version = false;
			}
			
			// Done with outer loop
			break;
		}
	}
	
	return true;
}

// Write a single vCard address to the output stream
void CVCard::Write(std::ostream& out)
{
	out << os_endl;
	out << "BEGIN:VCARD" << os_endl;
	out << "VERSION:3.0" << os_endl;

	// Output the FN item first - must exist
	std::pair<CVCardItems::const_iterator, CVCardItems::const_iterator> iters = mItems.equal_range("FN");
	for(CVCardItems::const_iterator iter = iters.first; iter != iters.second; iter++)
		WriteItem(out, (*iter).first, (*iter).second);

	// Output the N item first - must exist
	iters = mItems.equal_range("N");
	for(CVCardItems::const_iterator iter = iters.first; iter != iters.second; iter++)
		WriteItem(out, (*iter).first, (*iter).second);

	// Output remaining items in multimap order
	for(CVCardItems::const_iterator iter = mItems.begin(); iter != mItems.end(); iter++)
	{
		if (((*iter).first != "FN") && ((*iter).first != "N"))
			WriteItem(out, (*iter).first, (*iter).second);
	}
	
	out << "END:VCARD" << os_endl;
}

// Read a single vCard item into an address
void CVCard::ReadItem(cdstring& str, bool old_version)
{
	const char* p = str.c_str();
	const char* q = p;

	// Look for name
	while(*p && (*p != ';') && (*p != ':')) p++;
	if (!*p)
		return;

	// Get the name
	cdstring name(q, p - q);
	name.trimspace();
	
	// See if parameters follow
	CVCardItem item;

	if (*p == ';')
	{
		// Loop over each parameter
		while(true)
		{
			// Read param in one blob
			q = ++p;
			
			// v2.1 allows TYPE= to be omitted
			// v3.0 requires all parameters to be named
			cdstring param_name;
			if (old_version)
			{
				// Look for '=', ',', ';' or ':'
				while(*p && (*p != '=') && (*p != ',') && (*p != ';') && (*p != ':')) p++;
				if (!*p)
					return;
				
				// If '=' we have a parameter name
				if (*p == '=')
					// Grab the parameter name
					param_name.assign(q, p - q);
				else
				{
					// Implicit 'TYPE=' parameter
					param_name = "TYPE";
					
					// Move p back to q - 1 so that it will bump back to the param value
					p = q - 1;
				}
			}
			else
			{
				// Look for '='
				while(*p && (*p != '=')) p++;
				if (!*p)
					return;

				// Grab the parameter name
				param_name.assign(q, p - q);
			}

			// Loop over values
			while(true)
			{
				// Look for values
				q = ++p;
				if (*q == '"')
				{
					while(*p && (*p != '"')) p++;
					if (!*p)
						return;
				}
				else
				{
					while(*p && (*p != ',') && (*p != ';') && (*p != ':')) p++;
					if (!*p)
						return;
				}
				
				// Get the value
				cdstring param_value(q, p - q);
				if (*p == '"')
					p++;

				// Add to map
				item.AddParam(param_name, param_value);
				
				// Continue with loop only if another param-value
				if (*p != ',')
					break;
			}
			
			// Continue with loop only if another param
			if (*p != ';')
				break;
		}
	}

	// Must be ':'
	if (*p != ':')
		return;
	p++;
	
	// Remainder is the value
	cdstring value(p);
	cdstring decoded;
	if (!::strcmpnocase(name, "ADR"))
		decoded = DecodeTextAddrValue(value);
	else if (!::strcmpnocase(name, "N"))
		decoded = DecodeTextNValue(value);
	else
		decoded = DecodeTextValue(value);
	
	// For v2.1 might have q-p encoding
	if (old_version && ::count(item.GetParams().begin(), item.GetParams().end(), cdstrmultimap::value_type("TYPE", "QUOTED-PRINTABLE")))
	{
		::qp_decode(decoded.c_str_mod());
		::FilterEndls(decoded.c_str_mod());
	}
	item.SetValue(decoded);

	// Add the item
	AddItem(name, item);
}

const char* cParamSpecials = ";,:";

void CVCard::WriteItem(std::ostream& out, const cdstring& name, const CVCardItem& item)
{
	// We need to fold lines so write into temp output stream first
	std::ostrstream tout;

	// Output the name
	tout << name;
	
	// Output each parameter accumulating multiple into comma list
	cdstring last_param;
	for(cdstrmultimap::const_iterator iter = item.GetParams().begin(); iter != item.GetParams().end(); iter++)
	{
		// Is it the same as the last one
		if ((*iter).first == last_param)
		{
			// Comma separate list of param-values
			tout << ',';
		}
		else
		{
			// New parameter
			last_param = (*iter).first;
			tout << ';' << last_param << '=';
		}

		// Look for param specials
		if (::strpbrk((*iter).second, cParamSpecials))
		{
			// Quote it
			tout << '"' << (*iter).second << '"';
		}
		else
			tout << (*iter).second;
	}
	
	// Output encoded value
	tout << ':';
	if (!::strcmpnocase(name, "ADR"))
		tout << EncodeTextAddrValue(item.GetValue());
	else if (!::strcmpnocase(name, "N"))
		tout << EncodeTextNValue(item.GetValue());
	else
		tout << EncodeTextValue(item.GetValue());
	tout << os_endl << ends;
	
	// Put into string
	cdstring result;
	result.steal(tout.str());
	
	// Fold it
	
	// Write to real output stream
	out << result;
}

#pragma mark ____________________________text-value

// text-value - utf8/escaped
cdstring CVCard::DecodeTextValue(const cdstring& str)
{
	// Escape it
	std::ostrstream out;
	const char* p = str.c_str();
	while(*p)
	{
		switch(*p)
		{
		case '\\':
			p++;
			switch(*p)
			{
			case '\\':
			case ';':
			case ',':
				out << *p++;
				break;
			case 'N':
			case 'n':
				// Always output \n
				out << os_endl;
				p++;
				break;
			default:
				out << '\\';
				break;
			}
			break;
		default:
			out << *p++;
			break;
		}
	}
	
	out << ends;
	
	cdstring result;
	result.steal(out.str());
	
	return result;
}

cdstring CVCard::EncodeTextValue(const cdstring& str)
{
	// Escape it
	std::ostrstream out;
	const char* p = str.c_str();
	while(*p)
	{
		switch(*p)
		{
		case '\\':
		case ';':
		case ',':
			out << '\\' << *p++;
			break;
		case '\n':
			// Always output \n
			out << "\\n";
			p++;
			break;
		case '\r':
			// Ignore if trailing \n, otherwise output \n
			if (*(p+1) != '\n')
				out << "\\n";
			p++;
			break;
		default:
			out << *p++;
			break;
		}
	}
	
	out << ends;
	
	cdstring result;
	result.steal(out.str());
	
	return result;
}

#pragma mark ____________________________addr-value

cdstring CVCard::DecodeTextAddrValue(const cdstring& str)
{
	// Escape it
	cdstrvect items;
	unsigned long items_with_content = 0;
	const char* p = str.c_str();
	const char* q = str.c_str();
	while(*p)
	{
		switch(*p)
		{
		// Look for item separator
		case ';':
			if (p - q)
			{
				items.push_back(DecodeTextValue(cdstring(q, p - q)));
				items_with_content++;
			}
			else
				items.push_back(cdstring::null_str);

			// Bump past blank line
			q = ++p;
			break;
		
		// Look for quote
		case '\\':
			p++;
			switch(*p)
			{
			case '\\':
			case ';':
			case ',':
			case 'N':
			case 'n':
				// Valid quote - punt over both chars
				p++;
				break;
			default:;
			}
			break;
		
		// Punt over normal char
		default:
			p++;
			break;
		}
	}
	
	// Write out the remainder
	items.push_back(DecodeTextValue(q));

	// Now convert to multiline item
	cdstring result;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
	{
		if (result.length())
			result += os_endl;
		result += *iter;
	}
	
	return result;
}

cdstring CVCard::EncodeTextAddrValue(const cdstring& str)
{
	// po-box;ext-addr;street;city;state;zip;country
	cdstring result;
	result += ";";
	result += EncodeTextValue(str);

	return result;
}

#pragma mark ____________________________n-value

cdstring CVCard::DecodeTextNValue(const cdstring& str)
{
	// We ignore this and use FN for now
	return DecodeTextValue(str);
}

cdstring CVCard::EncodeTextNValue(const cdstring& str)
{
	// Family; Given; Middle; Prefix; Suffix
	cdstring result;
	if (::strchr(str, ' '))
	{
		cdstring copy(str);
		const char* first = ::strtok(copy.c_str_mod(), " ");
		const char* second = ::strtok(NULL, "");
		
		result += EncodeTextValue(second);
		result += ";";
		result += EncodeTextValue(first);
	}
	else
		result += EncodeTextValue(str);

	return result;
}
