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

// GPGTest.cp

// Test application for ISpell plugin testing

#include "CSpellPluginDLL.h"
#include "CISpellPluginDLL.h"

#include <iostream.h>

extern "C"
{
long MulberryPluginEntry(long, void*, long);
}

const char* text1 = "This is sme txt to cheack\nwith a scond bbbbbb line of txt.";

int main()
{
	long code;
	long refCon = 0;

	cout << sizeof(CPluginDLL) << endl;
	cout << sizeof(CSpellPluginDLL) << endl;
	cout << sizeof(CISpellPluginDLL) << endl;
	cout << "ISpellTest: Application Startup" << endl;
	cout << "ISpellTest: Contsruct and Initialise" << endl;
	
	refCon = MulberryPluginEntry(CSpellPluginDLL::ePluginConstruct, NULL, refCon);
	long err = MulberryPluginEntry(CSpellPluginDLL::ePluginInitialise, NULL, refCon);

	// Can run?
	err = MulberryPluginEntry(CSpellPluginDLL::ePluginCanRun, NULL, refCon);
	if (!err)
	{
		cout << "ISpellTest: CanRun is false" << endl;
		goto done;
	}
	
	err = MulberryPluginEntry(CSpellPluginDLL::eSpellInitialise, NULL, refCon);

	// Do tests here
#if 0
	{
		char* word = "test";
		err = MulberryPluginEntry(CSpellPluginDLL::eSpellContainsWord, word, refCon);
	}
	{
		char* word = "poo";
		err = MulberryPluginEntry(CSpellPluginDLL::eSpellContainsWord, word, refCon);
	}
#endif
#if 1
	cout << "ISpellText: Check text: " << text1 << endl;

	err = MulberryPluginEntry(CSpellPluginDLL::eSpellCheckText, (void*) text1, refCon);

	if (MulberryPluginEntry(CSpellPluginDLL::eSpellHasErrors, NULL, refCon))
	{
		CSpellPluginDLL::SpError* sperr = NULL;
		int ctr = 1;
		while((sperr = (CSpellPluginDLL::SpError*) MulberryPluginEntry(CSpellPluginDLL::eSpellNextError, NULL, refCon)) != NULL)
		{
			cout << "ISpellText: Got an error:" << endl;
			cout << "            word:  " << sperr->word << endl;
			cout << "            start: " << sperr->sel_start << endl;
			cout << "            end:   " << sperr->sel_end << endl;

			// Check for replacement
			if (sperr->do_replace)
			{
				cout << "            repl:  " << sperr->replacement << endl;
			}
			else if (!sperr->ask_user)
			{
				cout << "            skip" << endl;
			}
			else
			{
				CSpellPluginDLL::SpGetSuggestionsData data;
				const char* suggestions = (const char*) MulberryPluginEntry(CSpellPluginDLL::eSpellGetSuggestions, &data, refCon);
				cout << "            suggestions:" << endl << (suggestions ? suggestions : "None") << endl;
			}
			cout << endl;

			// Fake replace all
			if (ctr == 2)
			{
				sperr->replacement = "text";
				MulberryPluginEntry(CSpellPluginDLL::eSpellSkipAll, NULL, refCon);
			}
			ctr++;
		}
	}
#endif

	cout << "ISpellTest: Terminate and Destruct" << endl;

	err = MulberryPluginEntry(CSpellPluginDLL::eSpellTerminate, NULL, refCon);

done:
	err = MulberryPluginEntry(CSpellPluginDLL::ePluginTerminate, NULL, refCon);
	err = MulberryPluginEntry(CSpellPluginDLL::ePluginDestroy, NULL, refCon);

	cout << "ISpellTest: Application Shutdown" << endl;
}
