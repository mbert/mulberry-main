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


// CTabs

#include "CTabs.h"

#include <LGATabsControlImp.h>
#include <LPlaceHolder.h>

// Constructor from stream
CTabs::CTabs(LStream *inStream) : LTabsControl(inStream)
{
	mPanelHost = NULL;
}

// Default destructor
CTabs::~CTabs()
{
}

void CTabs::FinishCreateSelf()
{
	// Do inherited
	LTabsControl::FinishCreateSelf();

	// Get panel host
	mPanelHost = (LPlaceHolder*) FindPaneByID('PLAC');
	ThrowIfNil_(mPanelHost);
}

void CTabs::InstallPanel(LPane* inPanel, SInt16 inAlignment, Boolean inRefresh)
{
	// ¥ We add the panel to the pael host and then get the host
	// refreshed so the panel shows up, if needed. The panel host
	// is an instance of the LPlaceHolder class so we let it handle
	// all the details of aligning the panel
	if (mPanelHost)
	{
		// ¥ Install the panel
		mPanelHost->InstallOccupant (inPanel, inAlignment);
		
		// ¥ Refresh the host if needed
		if (inRefresh)
			mPanelHost->Refresh();
	}
}

LPane* CTabs::RemovePanel()
{
	// ¥ We call the placeholder to get the panel removed, the placeholder
	// will restore any size changes that were made when the panel was
	// installed, typically this does not need to be called as the place
	// holder does this automatically when a new panel is installed
	if (mPanelHost)
	{
		// ¥ Remove the panel from the placeholder and return the panel
		return mPanelHost->RemoveOccupant();
	}
	else
		return NULL;
}

void CTabs::RemoveTabButtonAt(ArrayIndexT inAtIndex)
{
	if (UEnvironment::HasFeature (env_HasAppearance))
	{
		// get Max value
		SInt32 outMaxValue = GetMaxValue();
		
		// Move info for buttons at higher pos down by one
		for(SInt32 i = inAtIndex; i < outMaxValue; i++)
		{
			ControlTabInfoRec info;
			info.version = kControlTabInfoVersionZero;
			(mControlSubPane->GetControlImp())->GetDataTag(i + 1, kControlTabInfoTag, sizeof(ControlTabInfoRec), &info, NULL);
			(mControlSubPane->GetControlImp())->SetDataTag(i, kControlTabInfoTag, sizeof(ControlTabInfoRec), &info);
		}
		
		// Reset maximum to remove button
		SetMaxValue(outMaxValue - 1);
	}
	else
	{
		static_cast<LGATabsControlImp*>((mControlSubPane->GetControlImp()))->RemoveTabButtonAt(inAtIndex);
	}
}