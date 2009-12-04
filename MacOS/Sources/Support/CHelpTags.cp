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


// Source for CHelpTagWindow class

#include "CHelpTags.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default destructor
CHelpTagWindow::~CHelpTagWindow()
{
	if (mHelpCallback != NULL)
		DisposeHMWindowContentUPP(mHelpCallback);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set size
void CHelpTagWindow::SetupHelpTags()
{
	mHelpCallback = ::NewHMWindowContentUPP(CHelpTagWindow::TagCallback);

	::HMInstallWindowContentCallback(mWindow->GetMacWindow(), mHelpCallback);
}

pascal OSStatus	CHelpTagWindow::TagCallback(WindowRef inWindow, Point inGlobalMouse,
									HMContentRequest inRequest, HMContentProvidedType *outContentProvided, HMHelpContentPtr ioHelpContent)
{
	*outContentProvided = kHMContentNotProvided;
	
	if (inRequest == kHMSupplyContent)
	{
		// Get the matching window object
		LWindow* window = LWindow::FetchWindowObject(inWindow);

		if (window)
		{
			Point portMouse = inGlobalMouse;
			window->GlobalToPortPoint(portMouse);

			// Find the pane under the mouse
			LPane* pane = window->FindDeepSubPaneContaining(portMouse.h, portMouse.v);
			
			// Now dynamic cast it and its super view chain until we find a CHelpTagPane
			while((pane != NULL) && (dynamic_cast<CHelpTagPane*>(pane) == NULL))
				pane = pane->GetSuperView();
			CHelpTagPane* tag = dynamic_cast<CHelpTagPane*>(pane);

			if ((tag != NULL) && tag->SetupHelp(pane, portMouse, ioHelpContent))
			{
				*outContentProvided = kHMContentProvided;
			}
		}
	}
	
	return noErr;
}

bool CHelpTagPane::SetupHelp(LPane* pane, Point inPortMouse, HMHelpContentPtr ioHelpContent)
{
	Rect frame;
	if (mHelpEnabled && pane->CalcPortExposedRect(frame) && !::EmptyRect(&frame))
	{
		pane->PortToGlobalPoint( topLeft(frame) );
		pane->PortToGlobalPoint( botRight(frame) );

		ioHelpContent->absHotRect = frame;
		ioHelpContent->tagSide = kHMDefaultSide;
		ioHelpContent->content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
		ioHelpContent->content[kHMMinimumContentIndex].u.tagCFString = GetHelpContent();; 
		ioHelpContent->content[kHMMaximumContentIndex].contentType = kHMNoContent; 

		return true;
	}
	else
		return false;
}

void CHelpTagPane::SetTagText(const char* txt)
{
	MyCFString cftxt(txt, kCFStringEncodingUTF8);
	mHelpTag = cftxt;
}

void CHelpTagPane::SetupTagText()
{
	// Does nothing here - sub-class musxt override to set static help text
}

// Sub-class should override to return dynamic help tag - this method
// just returns the static text
const MyCFString& CHelpTagPane::GetHelpContent() const
{
	return mHelpTag;
}
