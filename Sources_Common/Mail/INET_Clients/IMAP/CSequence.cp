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


// Source for CSequence class

#include "CSequence.h"

#include <stdio.h>
#include <string.h>

#include <strstream>
#include <algorithm>

// __________________________________________________________________________________________________
// C L A S S __ C S E Q U E N C E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CSequence::CSequence()
{
}

// Copy from raw list
CSequence::CSequence(const ulvector& copy)
	: ulvector(copy)
{
	// Make sure its sorted
	sort(begin(), end());

} // CSequence::CSequence

// Default destructor
CSequence::~CSequence()
{
} // CSequence::~CSequence

// O T H E R  M E T H O D S ____________________________________________________________________________

// Insert non-duplicates
const char* CSequence::GetSequenceText()
{
	// Only do if no text already
	if (mSequenceText.empty())
	{

		// Create stream to append chars to
		std::ostrstream out;

		unsigned long firstNum = 0;
		unsigned long lastNum = 0;
		unsigned long numCtr = 0;
		bool seq = false;
		bool to_end = false;
		for(CSequence::const_iterator iter = begin(); iter != end(); iter++)
		{
			unsigned long theNum = *iter;
			if (!theNum)
				to_end = true;

			if (lastNum)
			{
				if (theNum > lastNum+1)
				{

					// Add comma if some already there
					if (numCtr)
						out << ",";

					// Add numbers
					if (seq)
					{
						// Must convert to string to avoid thousands separator appear in formatted <<
						cdstring firstStr(firstNum);
						cdstring lastStr(lastNum);
						out << firstStr << ":" << lastStr;
					}
					else
					{
						// Must convert to string to avoid thousands separator appear in formatted <<
						cdstring lastStr(lastNum);
						out << lastStr;
					}
					numCtr++;
					seq = false;
				}
				else
					seq = true;
			}

			if (!seq) firstNum = theNum;
			lastNum = theNum;
		}

		// Copy final one in
		if (lastNum)
		{

			// Add comma if some already there
			if (numCtr)
				out << ",";

			// Add numbers
			if (seq)
			{
				// Must convert to string to avoid thousands separator appear in formatted <<
				cdstring firstStr(firstNum);
				cdstring lastStr(lastNum);
				out << firstStr << ":" << lastStr;
			}
			else
			{
				// Must convert to string to avoid thousands separator appear in formatted <<
				cdstring lastStr(lastNum);
				out << lastStr;
				if (to_end)
					out << ":*";
			}
		}
		else
			// Single 0 in sequence - do entire list
			out << "1:*";

		out << std::ends;
		mSequenceText.steal(out.str());

	}

	return mSequenceText;

} // CSequence::GetSequenceText

// Parse sequence text
void CSequence::ParseSequence(const char** txt, unsigned long size_estimate)
{
	if (size_estimate != 0)
		reserve(size_estimate);

	const char* p = *txt;

	while(*p == ' ') p++;

	while(isdigit(*p))
	{
		unsigned long temp1 = ::strtoul(p, const_cast<char**>(&p), 10);
		push_back(temp1);

		// Process sequence first
		if (*p == ':')
		{
			// Sequence
			p++;
			unsigned long temp2 = ::strtoul(p, const_cast<char**>(&p), 10);
			for(unsigned long i = temp1 + 1; i <= temp2; i++)
				push_back(i);
		}

		if (*p == ',')
			// Item to follow
			p++;
		else
			// Done
			break;
	}

	*txt = p;
}
