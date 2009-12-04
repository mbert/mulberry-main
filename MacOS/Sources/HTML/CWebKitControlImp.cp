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

// ===========================================================================
//	CWebKitControlImp.cp			PowerPlant 2.2.2	©1997-2001 Metrowerks Inc.
// ===========================================================================
//
//	Implementation for Appearance Manager EditText item.
//
//	The class does not create a ControlHandle and it inherits from
//	LControlImp rather than LAMControlImp.
//
//	The LEditText ControlPane handles everything related to the text.
//	The only thing this class does is draw the frame and focus ring
//	around the text area.

#ifdef PowerPlant_PCH
#include PowerPlant_PCH
#endif

#include "CWebKitControlImp.h"

#include "CWebKitUtils.h"

#include "MyCFString.h"

#include <UControlMgr.h>

#include <CoreFoundation/CoreFoundation.h>

PP_Begin_Namespace_PowerPlant


// ---------------------------------------------------------------------------
//	¥ CWebKitControlImp						Stream Constructor		  [public]
// ---------------------------------------------------------------------------
//	Do not access inStream. It may be nil. Read Stream data in Init()

CWebKitControlImp::CWebKitControlImp(
									 LStream*	inStream)

: LAMControlImp(inStream)
{
}


// ---------------------------------------------------------------------------
//	¥ ~CWebKitControlImp						Destructor				  [public]
// ---------------------------------------------------------------------------

CWebKitControlImp::~CWebKitControlImp()
{
}

// ---------------------------------------------------------------------------
//	¥ MakeMacControl											   [protected]
// ---------------------------------------------------------------------------

void
CWebKitControlImp::MakeMacControl(
								  ConstStringPtr	inTitle,
								  SInt32			inRefCon)
{
	Rect	frame;					// Get Frame in Local coordinates
	if (!CalcLocalFrameRect(frame)) {
		
		// Frame is outside of QuickDraw space. NewControl requires
		// a rectangle in QuickDraw space, so we have to use an
		// artificial location. The Control won't get drawn until
		// its SuperView scrolls it into view.
		
		SDimension16	frameSize;
		mControlPane->GetFrameSize(frameSize);
		
		::MacSetRect(&frame, 0, 0, frameSize.width, frameSize.height);
	}
	
	StFocusAndClipIfHidden	focus(mControlPane);
	
	// Controls must be created within a Window. First
	// try the port containing the ControlPane. If that's
	// nil, the ControlPane hasn't been installed yet so
	// we use the current port and make it the responsibility
	// of the caller to set the port correctly.
	
	WindowPtr		macWindowP = mControlPane->GetMacWindow();
	if (macWindowP == nil) {
		macWindowP = UQDGlobals::GetCurrentWindowPort();
	}
	
	// Check if there is a root control
	ControlHandle	rootControl;
	
	OSErr	err = ::GetRootControl(macWindowP, &rootControl);
	
	if (err == errNoRootControl) {
		
		// There is no root control, so we must create one.
		// This happens for the first AM control in a window.
		
		err = ::CreateRootControl(macWindowP, &rootControl);
		
		if (err == errControlsAlreadyExist) {
			
			// This error (-30589) means that a Standard (old-style)
			// Toolbox control was created in the window before the
			// first Appearance Toolbox control. If you are mixing old
			// and new style Toolbox controls in the same window, you
			// must create a new style one first.
			//
			// If you are creating a window from a PPob resource, you
			// can satisfy this requirement by opening the PPob in
			// Constructor and choosing "Show Object Hierarchy" from
			// the Layout menu. In the Hierarchy window, arrange the
			// Panes via drag-and-drop so that an Appearance control
			// is created before any old-style control.
			//
			// Note that LListBox and LScroller will create old-style
			// scroll bar controls, and that the Appearance EditText
			// pane is not a new-style controls (since it doesn't
			// actually use a ControlHandle). Use LScrollerView
			// in place of LScroller/LActiveScroller.
			//
			// If you are installing controls at runtime, create an
			// Appearance control first, or call ::CreateRootControl()
			// yourself before creating any Toolbox controls.
			//
			// This workaround is not necessary on Mac OS X, where all
			// windows automatically get a root control.
			
			SignalStringLiteral_("Must create an Appearance control before "
						 		 "creating an old-style control. See comments "
								 "in the source file.");
		}
		
		ThrowIfOSErr_(err);
	}
	
	// Create Toolbox Control. During construction Panes are
	// always invisible and disabled, so we pass "false" for
	// the "visible" parameter to NewControl. Since NewControl
	// always creates enabled controls, we then explicitly
	// disable the control.
	
	HIViewRef viewRef = NULL;
	OSStatus status = ::CreateHIWebView(&viewRef);
	if (status != noErr)
		throw "HIWebViewCreate failed";
	status = ::HIViewAddSubview(rootControl, viewRef);
	if (status != noErr)
		throw "HIWebViewCreate failed";
	
	
	mViewRef = mMacControlH = viewRef;
	
	ThrowIfNil_(mMacControlH);
	
	::DeactivateControl(mMacControlH);
	
	// We need to create a custom control color proc in order to
	// set up the correct background and text color when drawing
	// a control. However, this isn't supported on Mac OS 68K.
	
#if 0
#if !(TARGET_OS_MAC && TARGET_CPU_68K)
	
	if (UEnvironment::HasFeature(env_HasAppearance11)) {
		
		static StControlColorUPP	sControlColorUPP(CustomControlColorProc);
		
		sControlColorUPP.SetColorProc(mMacControlH);
		
		// Store ControlImp and ControlPane as properties so we
		// can retrieve them within the custom color proc
		
		LAMControlImp*	theImp = this;
		
		::SetControlProperty(mMacControlH, PropType_PowerPlant, PropTag_ControlImp, sizeof(LAMControlImp*), &theImp);
		::SetControlProperty(mMacControlH, PropType_PowerPlant, PropTag_ControlPane, sizeof(LControlPane*), &mControlPane);
	}
	
#endif
#endif
	
	if (mTextTraitsID != 0) {				// Use custom text traits
		ResIDT	saveID = mTextTraitsID;		// Save and set member var to 0
		mTextTraitsID = 0;					//   so SetTextTraits() knows
		SetTextTraitsID(saveID);			//   the value is changing
	}
}


void CWebKitControlImp::SetURL(const cdstring& urltxt)
{
	MyCFString cfstring(urltxt, kCFStringEncodingUTF8);
	CFURLRef url = CFURLCreateWithString( NULL, cfstring, NULL );
	URLToWebView(mViewRef, url);
	CFRelease(url);
}

void CWebKitControlImp::SetData(const cdstring& datatxt)
{
	MyCFString cfstring(datatxt, kCFStringEncodingUTF8);
	DataToWebView(mViewRef, cfstring);
}

PP_End_Namespace_PowerPlant
