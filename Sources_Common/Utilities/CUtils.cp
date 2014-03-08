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


// Source for common utilities

#include "CUtils.h"

#include <stdio.h>
#include <string.h>

// Check version number against long
short VersionTest(long vers1, long vers2)
{
	if (vers1 > vers2)
		return 1;
	else if (vers1 < vers2)
		return -1;
	else
		return 0;
}

// Check version number against long
short VersionTest(NumVersion vers1, long vers2)
{
	unsigned char value;

	// Check majorRev
	value = (vers2 >> 24) & 0x000000FF;
	if (vers1.majorRev > value)
		return 1;
	if (vers1.majorRev < value)
		return -1;

	// Check minorAndBugRev
	value = (vers2 >> 16) & 0x000000FF;
	if (vers1.minorAndBugRev > value)
		return 1;
	if (vers1.minorAndBugRev < value)
		return -1;

	// Check stage
	value = (vers2 >> 8) & 0x000000FF;
	if (vers1.stage > value)
		return 1;
	if (vers1.stage < value)
		return -1;

	// Check nonRelRev
	value = vers2 & 0x000000FF;
	if (vers1.nonRelRev > value)
		return 1;
	if (vers1.nonRelRev < value)
		return -1;

	return 0;
}


// Get version as string
cdstring GetVersionText(long vers)
{
	NumVersion nvers;
	nvers.majorRev = (vers >> 24) & 0x000000FF;
	nvers.minorAndBugRev = (vers >> 16) & 0x000000FF;
	nvers.stage = (vers >> 8) & 0x000000FF;
	nvers.nonRelRev = (vers) & 0x000000FF;
	return GetVersionText(nvers);
}


// Get version as string
cdstring GetVersionText(NumVersion vers)
{
	char txt[256];

	if (vers.majorRev > 0)
	{
		char format[64];
		bool has_numbers = false;

		switch (vers.stage) {

			case developStage:
				::strcpy(format, "%d.%d.%dd%d");
				has_numbers = true;
				break;

			case alphaStage:
				::strcpy(format, "%d.%d.%da%d");
				has_numbers = true;
				break;

			case betaStage:
				::strcpy(format, "%d.%d.%db%d");
				has_numbers = true;
				break;

			case finalStage:
				if (vers.nonRelRev)
					::strcpy(format, "%d.%d.%d.%d");
				else
					::strcpy(format, "%d.%d.%d");
				has_numbers = true;
				break;

		}

		if (has_numbers)
			::snprintf(txt, 256, format, vers.majorRev, (vers.minorAndBugRev >> 4) & 0x0F,
								vers.minorAndBugRev & 0x0F, vers.nonRelRev);
		else
			::snprintf(txt, 256, "Unknown");
	}
	else
		::snprintf(txt, 256, "Unknown");

	return txt;
}

#pragma mark -

// Calculate CRC code from text
unsigned long CRCCalculate(const char* text, unsigned long key)
{
	short i;
	size_t len = ::strlen(text);
	unsigned long value = len;

	while(len--) {

		value ^= (unsigned char) *text++;
		for(i=0; i<8; i++) {
			if (value & 0x0001)
				value = (value >> 1) ^ key;
			else
				value >>= 1;
		}
	}

	return value;
}
