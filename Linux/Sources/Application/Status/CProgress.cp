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


// Source for CProgress classes

#include "CProgress.h"

// __________________________________________________________________________________________________
// C L A S S __ C P R O G R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CProgress::CProgress()
{
	mPercentage = 0;
	mCount = 0;
	mTotal = 0;
}

#pragma mark ____________________________CBarPane

// __________________________________________________________________________________________________
// C L A S S __ C B A R P A N E
// __________________________________________________________________________________________________

void CBarPane::SetCount(unsigned long count)
{
	CProgress::SetCount(count);
	JXProgressIndicator::SetValue(count);
}

void CBarPane::SetTotal(unsigned long total)
{
	CProgress::SetTotal(total);
	SetMaxValue(total ? total : 1);
}
