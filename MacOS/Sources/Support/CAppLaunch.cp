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


// CAppLaunch.cp

// Class that launches an app and opens a document in it

#include "CAppLaunch.h"

#include <SysCFArray.h>
#include <SysCFString.h>
#include <SysCFURL.h>

#include <LaunchServices.h>

// OpenSpecifiedDocument searches to see if the application which
// created the document is already running.  If so, it sends
// an OpenSpecifiedDocuments Apple event to the target application
// (remember that, because of puppet strings, this works even
// if the target application is not Apple event-aware.)

OSErr CAppLaunch::OpenDocumentWithApp(const PPx::FSObject* doc, const cdstring& mimeType, OSType appCreator)
{
	// verify the document file exists and get its creator type

	if (!doc->Exists())
		return fnfErr;

    if (!appCreator)
    {
        // Try mime type first, then just use file name (extension)
        PPx::CFString inMIMEType(mimeType, kCFStringEncodingUTF8);
        CFURLRef outAppURL;
        if (::LSCopyApplicationForMIMEType(inMIMEType, kLSRolesAll, &outAppURL) == noErr)
        {
            OSErr err = LaunchApplicationWithDocument(outAppURL, doc);
            ::CFRelease(outAppURL);
            return err;
        }
        
        return ::LSOpenFSRef(&doc->UseRef(), NULL);
    }
    else
        return LaunchApplicationWithDocument(appCreator, doc);
}

// Launch the URL
OSErr CAppLaunch::LaunchURL(const cdstring& url)
{
    PPx::CFString urlStr(url, kCFStringEncodingUTF8);
    PPx::CFURL inURL(urlStr);
    return ::LSOpenCFURLRef(inURL, NULL);
}

//----------------------------------------------------------------------------
// LaunchApplicationWithDocument
//
// given an application and any number of documents,
// LaunchApplicationWithDocument launches the application and passes the
// application an OpenDocuments event for the document(s)
//----------------------------------------------------------------------------
OSErr CAppLaunch::LaunchApplicationWithDocument(OSType appCreator, const PPx::FSObject* doc)
{
	
	// Find the application on disk
	FSRef outAppRef;
	OSErr err = ::LSGetApplicationForInfo(kLSUnknownType, appCreator, NULL, kLSRolesAll, &outAppRef, NULL);
	
	// Try to launch it with the document

	LSLaunchFSRefSpec inLaunchSpec;
	inLaunchSpec.appRef = (err == noErr) ? &outAppRef : NULL;
	inLaunchSpec.numDocs = 1;
	inLaunchSpec.itemRefs = &doc->UseRef();
	inLaunchSpec.passThruParams = NULL;
	inLaunchSpec.launchFlags = kLSLaunchDefaults;
	inLaunchSpec.asyncRefCon = NULL;

	err = ::LSOpenFromRefSpec(&inLaunchSpec, NULL);

	return err;
}

OSErr CAppLaunch::LaunchApplicationWithDocument(CFURLRef appURL, const PPx::FSObject* doc)
{
    PPx::CFURL docURL(doc->UseRef());
    PPx::CFArray<CFURLRef> array(1);
    array.AppendValue(docURL);
    
	
	// Try to launch it with the document
    
	LSLaunchURLSpec inLaunchSpec;
	inLaunchSpec.appURL = appURL;
	inLaunchSpec.itemURLs = array;
	inLaunchSpec.passThruParams = NULL;
	inLaunchSpec.launchFlags = kLSLaunchDefaults;
	inLaunchSpec.asyncRefCon = NULL;
    
	OSErr err = ::LSOpenFromURLSpec(&inLaunchSpec, NULL);
    
	return err;
}

