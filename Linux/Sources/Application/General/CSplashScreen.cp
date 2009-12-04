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


// Source for CSplashScreen class

#include "CSplashScreen.h"

#include "CPreferences.h"
#include "CRegistration.h"
#include "CStaticText.h"

#include "HBackgroundImage.h"
#include "HResourceMap.h"

#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>
#include <sys/time.h>

// __________________________________________________________________________________________________
// C L A S S __ C A B O U T D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSplashScreen::CSplashScreen(JXDirector* supervisor)
	: CDialogDirector(supervisor, kFalse)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSplashScreen::OnCreate()
{
    //JXWindow* window = new JXWindow(this, 435,205, "", kFalse, NULL, kTrue);
// begin JXLayout

    JXWindow* window = new JXWindow(this, 435,205, "", kFalse, NULL, kTrue);
    assert( window != NULL );
    SetWindow(window);

    mSplash =
        new HBackgroundImage(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 435,205);
    assert( mSplash != NULL );

    CStaticText* obj1 =
        new CStaticText("Version Number", mSplash,
                    JXWidget::kHElastic, JXWidget::kVElastic, 280,184, 95,17);
    assert( obj1 != NULL );
    obj1->SetFontSize(10);

    mVersion =
        new CStaticText("v2.0", mSplash,
                    JXWidget::kHElastic, JXWidget::kVElastic, 375,184, 55,17);
    assert( mVersion != NULL );
    mVersion->SetFontSize(10);

    CStaticText* obj2 =
        new CStaticText("Copyright Cyrus Daboo, 2006-2009.", mSplash,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,184, 275,17);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);

// end JXLayout

	obj1->SetTransparent(true);
	obj2->SetTransparent(true);
	//obj3->SetTransparent(true);
	mVersion->SetTransparent(true);
	//mSerial->SetTransparent(true);
	//mLicensee->SetTransparent(true);
	//obj4->SetTransparent(true);

	mSplash->SetImage(bmpFromResource(IDB_SPLASH, mSplash), kTrue);

	mVersion->SetText(CPreferences::sPrefs->GetVersionText());
#if 0
	mSerial->SetText(CRegistration::sRegistration.GetSerialNumber());
	cdstring licensee = CRegistration::sRegistration.GetLicensee();
	if (CRegistration::sRegistration.GetOrganisation().length())
	{
		licensee += "\n";
		licensee += CRegistration::sRegistration.GetOrganisation();
	}
	mLicensee->SetText(licensee);
#endif
}

bool CSplashScreen::PoseDialog()
{
	CSplashScreen* dlog = new CSplashScreen(JXGetApplication());

	// Test for OK
	//dlog->DoModal();

	// Create it and start dialog director
	dlog->OnCreate();

	JXApplication* app = JXGetApplication();
	app->DisplayInactiveCursor();

	JXWindow* window = dlog->GetWindow();
	window->PlaceAsDialogWindow();
	window->LockCurrentSize();

	dlog->BeginDialog();

	// Wait
	timeval tv;
	::gettimeofday(&tv, NULL);
	unsigned long old_time = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
	unsigned long new_time = old_time;
	do
	{
		app->HandleOneEventForWindow(window);

		::gettimeofday(&tv, NULL);
		new_time = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
	}
	while((new_time - old_time) / 1000 < 3);

	// Close the dialog and return
	dlog->Close();

	return true;
}
