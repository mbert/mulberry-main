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


// CPSPrinter.cp - UI widget that implements a 3D divider

#include "CPSPrinter.h"
#include "CStringUtils.h"

#include <JXPSPrinter.h>
#include <JXWindow.h>

#include <stdlib.h>

CPSPrinter CPSPrinter::sPSPrinter;

JXPSPrinter* CPSPrinter::GetPSPrinter(JXWindow* wnd)
{
	// Make sure it is init'd
	InitPSPrinter(wnd);
	
	return mPSPrinter;
}

void CPSPrinter::InitPSPrinter(JXWindow* wnd)
{
	if (!mPSPrinter)
	{
		mPSPrinter = new JXPSPrinter(wnd->GetDisplay(), wnd->GetColormap());

		// Get environment variable PAPERSIZE
		char* paper = ::getenv("PAPERSIZE");
		if (paper)
		{
			if (!::strcmpnocase(paper, "USLetter"))
				mPSPrinter->SetPaperType(JPSPrinter::kUSLetter);
			else if (!::strcmpnocase(paper, "USLegal"))
				mPSPrinter->SetPaperType(JPSPrinter::kUSLegal);
			else if (!::strcmpnocase(paper, "USExecutive"))
				mPSPrinter->SetPaperType(JPSPrinter::kUSExecutive);
			else if (!::strcmpnocase(paper, "A4Letter"))
				mPSPrinter->SetPaperType(JPSPrinter::kA4Letter);
			else if (!::strcmpnocase(paper, "B5Letter"))
				mPSPrinter->SetPaperType(JPSPrinter::kB5Letter);
		}

		// Get environment variable PRINTCMD
		char* cmd = ::getenv("PRINTCMD");
		if (cmd)
			mPSPrinter->SetPrintCmd(cmd);
	}
}
