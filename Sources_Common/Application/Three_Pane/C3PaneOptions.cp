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


// Source for C3PaneOptions class

#include "C3PaneOptions.h"

#include "char_stream.h"
#include "CPreferenceVersions.h"
#include "CStringUtils.h"
#include "CUtils.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________C3PaneOptions

// Static

// Static members

// Default constructor
C3PaneOptions::C3PaneOptions()
{
	mInitialView = N3Pane::eView_Mailbox;

	GetViewOptions(N3Pane::eView_Mailbox).SetListVisible(true);
	GetViewOptions(N3Pane::eView_Mailbox).SetItemsVisible(true);
	GetViewOptions(N3Pane::eView_Mailbox).SetPreviewVisible(true);
	GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListPreview).SetSingleClick(true);
	GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView).SetDoubleClick(true);
	GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView).SetKey('\r');
	GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsPreview).SetSelection(true);
	GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView).SetDoubleClick(true);
	GetViewOptions(N3Pane::eView_Mailbox).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView).SetKey('\r');

	GetViewOptions(N3Pane::eView_Contacts).SetListVisible(true);
	GetViewOptions(N3Pane::eView_Contacts).SetItemsVisible(true);
	GetViewOptions(N3Pane::eView_Contacts).SetPreviewVisible(true);
	GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListPreview).SetSingleClick(true);
	GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView).SetDoubleClick(true);
	GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView).SetKey('\r');
	GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsPreview).SetSelection(true);
	GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView).SetDoubleClick(true);
	GetViewOptions(N3Pane::eView_Contacts).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView).SetKey('\r');

	GetViewOptions(N3Pane::eView_Calendar).SetListVisible(true);
	GetViewOptions(N3Pane::eView_Calendar).SetItemsVisible(true);
	GetViewOptions(N3Pane::eView_Calendar).SetPreviewVisible(false);
	GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListPreview).SetSingleClick(true);
	GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView).SetDoubleClick(true);
	GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eListFullView).SetKey('\r');
	GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsPreview).SetSelection(true);
	GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView).SetDoubleClick(true);
	GetViewOptions(N3Pane::eView_Calendar).GetUserAction(C3PaneOptions::C3PaneViewOptions::eItemsFullView).SetKey('\r');
}

// Default destructor
C3PaneOptions::~C3PaneOptions()
{
}

// Default constructor
void C3PaneOptions::_copy(const C3PaneOptions& copy)
{
	mInitialView = copy.mInitialView;

	for(int i = 0; i < N3Pane::eView_Total; i++)
		mViewOptions[i] = copy.mViewOptions[i];
	
	mMailViewOptions = copy.mMailViewOptions;
	mAddressViewOptions = copy.mAddressViewOptions;
	mCalendarViewOptions = copy.mCalendarViewOptions;
}

const char* cViewDescriptors[] =
	{"None",
	 "Mailbox",
	 "Contacts",
	 "Calendar",
	 //"IM",
	 //"Bookmarks",
	 NULL};

cdstring C3PaneOptions::GetInfo() const
{
	cdstring details;
	
	details += cViewDescriptors[mInitialView];
	details += cSpace;
	
	// For v4 write out the number of view options so we can be backwards
	// compatible if we add more later
	unsigned long num = N3Pane::eView_Total;
	details += cdstring(num);
	
	for(int i = 0; i < N3Pane::eView_Total; i++)
	{
		details += cSpace;
		details += mViewOptions[i].GetInfo();
	}

	details += cSpace;
	details += '(';
	details += 	mMailViewOptions.GetInfo();
	details += ')';

	details += cSpace;
	details += '(';
	details += 	mAddressViewOptions.GetInfo();
	details += ')';

	details += cSpace;
	details += '(';
	details += 	mCalendarViewOptions.GetInfo();
	details += ')';

	return details;
}

bool C3PaneOptions::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cViewDescriptors, N3Pane::eView_Mailbox);
		mInitialView = static_cast<N3Pane::EViewType>(i);
	}
	
	// For v4 read in the number of view options so we can be backwards
	// compatible if we add more later
	unsigned long num = N3Pane::eView_Total;
	if (::VersionTest(vers_prefs, VERS_3_0_0) != 0)
		txt.get(num);
	else
		// v3 only had up to contacts
		num = N3Pane::eView_Contacts + 1;
	
	for(int i = 0; i < num; i++)
		mViewOptions[i].SetInfo(txt, vers_prefs);

	txt.start_sexpression();
	mMailViewOptions.SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	txt.start_sexpression();
	mAddressViewOptions.SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	txt.start_sexpression();
	mCalendarViewOptions.SetInfo(txt, vers_prefs);
	txt.end_sexpression();

	return true;
}

#pragma mark ____________________________C3PaneViewOptions

// Default constructor
C3PaneOptions::C3PaneViewOptions::C3PaneViewOptions()
{
	mGeometry = N3Pane::eListVert;
		
	mSplit1Pos = 0x08000000;		// 50%
	mSplit2Pos = 0x08000000;		// 50%

	mListVisible = true;
	mItemsVisible = true;
	mPreviewVisible = true;
	
	mFocusedPanel = eListView;
}

// Default destructor
C3PaneOptions::C3PaneViewOptions::~C3PaneViewOptions()
{
}

// Default constructor
void C3PaneOptions::C3PaneViewOptions::_copy(const C3PaneOptions::C3PaneViewOptions& copy)
{
	mGeometry = copy.mGeometry;

	mSplit1Pos = copy.mSplit1Pos;
	mSplit2Pos = copy.mSplit2Pos;

	mListVisible = copy.mListVisible;
	mItemsVisible = copy.mItemsVisible;
	mPreviewVisible = copy.mPreviewVisible;

	for(int i = 0; i < eUserAction_Total; i++)
	{
		mUserActions[i] = copy.mUserActions[i];
	}	
}

const char* cGeometryDescriptors[] =
	{"None",
	 "ListVert",
	 "ListHoriz",
	 "ItemsVert",
	 "ItemsHoriz",
	 "PreviewVert",
	 "PreviewHoriz",
	 "AllVert",
	 "AllHoriz",
	 NULL};

cdstring C3PaneOptions::C3PaneViewOptions::GetInfo() const
{
	cdstring details;
	
	details += cGeometryDescriptors[mGeometry];
	details += cSpace;
	
	details += cdstring(mSplit1Pos);
	details += cSpace;
	details += cdstring(mSplit2Pos);
	details += cSpace;

	details += mListVisible ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mItemsVisible ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;

	details += mPreviewVisible ? cValueBoolTrue : cValueBoolFalse;
	details += cSpace;
	
	for(int i = 0; i < eUserAction_Total; i++)
	{
		details += cSpace;
		details += mUserActions[i].GetInfo();
	}	
	
	return details;
}

bool C3PaneOptions::C3PaneViewOptions::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	{
		cdstring item;
		txt.get(item);
		long i = ::strindexfind(item, cGeometryDescriptors, N3Pane::eListVert);
		mGeometry = static_cast<N3Pane::EGeometry>(i);
	}

	txt.get(mSplit1Pos);
	txt.get(mSplit2Pos);

	txt.get(mListVisible);
	txt.get(mItemsVisible);
	txt.get(mPreviewVisible);
	
	for(int i = 0; i < eUserAction_Total; i++)
		mUserActions[i].SetInfo(txt, vers_prefs);
	
	return true;
}
