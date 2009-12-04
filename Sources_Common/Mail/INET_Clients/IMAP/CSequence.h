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


// Header for CSequence class

#ifndef __CSEQUENCE__MULBERRY__
#define __CSEQUENCE__MULBERRY__

#include "cdstring.h"
#include "templs.h"

// Classes

class CSequence : public ulvector
{

public:
	CSequence();
	CSequence(const ulvector& copy);						// Copy from raw list
	virtual ~CSequence();

	const char*	GetSequenceText();						// Get text of this sequence

	void	ParseSequence(const char** txt, unsigned long size_estimate = 0);			// Parse sequence text

private:
	cdstring		mSequenceText;

};

#endif
