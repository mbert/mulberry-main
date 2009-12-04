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


// Header for CSafetySave class

#ifndef __CSAFETYSAVE__MULBERRY__
#define __CSAFETYSAVE__MULBERRY__

#include "cdstring.h"

// Classes

class CSafetySave
{
public:

	static unsigned long SafetySaveDrafts(unsigned long& recovered);
	static unsigned long SafetyRestoreDrafts();

private:
	static cdstring sSafetySaveDirectory;		// Path to save recover drafts to

	static bool RecoverDraft(const cdstring& path);

	// Always static
	CSafetySave() {}
	~CSafetySave() {}
};

#endif
