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


// Source for CMailViewOptions class

#include "CMailViewOptions.h"

#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CMailViewOptions

// Static

// Static members

// Default constructor
CMailViewOptions::CMailViewOptions()
{
	mUseTabs = true;
	mRestoreTabs = true;
	mShowPreview = true;
	mShowAddressPane = true;
	mShowSummary = false;
	mShowPartsToolbar = true;
	mPreviewFlagging = eMarkSeen;
	mPreviewDelay = 3;
}

// Default destructor
CMailViewOptions::~CMailViewOptions()
{
}

// Default constructor
void CMailViewOptions::_copy(const CMailViewOptions& copy)
{
	mUseTabs = copy.mUseTabs;
	mRestoreTabs = copy.mRestoreTabs;
	mShowPreview = copy.mShowPreview;
	mShowAddressPane = copy.mShowAddressPane;
	mShowSummary = copy.mShowSummary;
	mShowPartsToolbar = copy.mShowPartsToolbar;
	mPreviewFlagging = copy.mPreviewFlagging;
	mPreviewDelay = copy.mPreviewDelay;
}

// Compare with same type
int CMailViewOptions::operator==(const CMailViewOptions& comp) const
{
	return (mUseTabs == comp.mUseTabs) &&
			(mRestoreTabs == comp.mRestoreTabs) &&
			(mShowPreview == comp.mShowPreview) &&
			(mShowAddressPane == comp.mShowAddressPane) &&
			(mShowSummary == comp.mShowSummary) &&
			(mShowPartsToolbar == comp.mShowPartsToolbar) &&
			(mPreviewFlagging == comp.mPreviewFlagging) &&
			(mPreviewDelay == comp.mPreviewDelay);
}

const char* cPreviewFlaggingDescriptors[] =
	{"Mark Seen",
	 "Delayed Seen",
	 "No Seen",
	 NULL};

cdstring CMailViewOptions::GetInfo() const
{
	cdstring details;

	details += mUseTabs ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mRestoreTabs ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mShowPreview ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mShowAddressPane ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mShowSummary ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mShowPartsToolbar ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;
	
	cdstring temp = cPreviewFlaggingDescriptors[mPreviewFlagging];
	temp.quote();
	details += temp;
	details += cSpace;

	details += cdstring(mPreviewDelay);
	
	return details;
}

bool CMailViewOptions::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mUseTabs);
	txt.get(mRestoreTabs);
	txt.get(mShowPreview);
	txt.get(mShowAddressPane);
	txt.get(mShowSummary);
	txt.get(mShowPartsToolbar);

	char* p = txt.get();
	mPreviewFlagging = static_cast<EPreviewFlagging>(::strindexfind(p, cPreviewFlaggingDescriptors, eMarkSeen));

	txt.get(mPreviewDelay);

	return true;
}
