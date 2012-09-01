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


// CAppLaunch.h

// Class that launches an app and opens a document in it

#ifndef __CAPPLAUNCH__MULBERRY__
#define __CAPPLAUNCH__MULBERRY__

class CAppLaunch
{
public:

	static OSErr OpenDocumentWithApp(const PPx::FSObject* doc, const cdstring& mimeType, OSType appCreator);
	
    static OSErr LaunchURL(const cdstring& url);

private:
	static OSErr LaunchApplicationWithDocument(OSType appCreator, const PPx::FSObject* doc);
	static OSErr LaunchApplicationWithDocument(CFURLRef appURL, const PPx::FSObject* doc);

	CAppLaunch();		// Never create
	~CAppLaunch();
};

#endif
