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


// CAttachmentManager.h

// Header file for CAttachmentManager class that will handle all address books,
// local and remote (on multiple servers). This class will be used for global
// nick-name resolution and searches. It can also be serialized to prefs and recovered.

#ifndef __CATTACHMENTMANAGER__MULBERRY__
#define __CATTACHMENTMANAGER__MULBERRY__

#include "cdstring.h"
#include "CLocalCommon.h"

// Classes
struct SAttachmentDesc
{
	fspectype	mSpec;
	cdstring	mMimeType;
	
	SAttachmentDesc(const fspectype& spec, const cdstring& mime)
		{ mSpec = spec; mMimeType = mime; }
};

typedef std::map<cdstring, SAttachmentDesc> CAttachmentFileMap;

class CAttachment;
class CFileAttachment;
class CFullFileStream;
class CMessage;
class LStream;

class CAttachmentManager
{
public:
	static CAttachmentManager sAttachmentManager;

	CAttachmentManager() {}
	~CAttachmentManager() {}

	bool CanQuit(cdstrvect& still_open);

	bool ViewAttachment(CMessage* owner, CAttachment* attach);
	bool ExtractAttachment(CMessage* owner, CAttachment* attach, bool launch_app, bool view);
	
	void ViewFile(const CFileAttachment* fattach);

private:
	CAttachmentFileMap mMap;
	
	bool Exists(const cdstring& url) const;
	CAttachmentFileMap::const_iterator GetEntry(const cdstring& url) const;
	void RemoveEntry(const cdstring& url);

	LStream* GetFileStream(CAttachment& attach, bool view);					// Get a file stream for some content
	bool MapToFile(const cdstring& name,									// Get local filename for MIME part
							fspectype& file, bool view);

	bool ExtractADAttachment(CMessage* owner, CAttachment* attach, bool launch_app, bool view);
	bool ExtractSingleAttachment(CMessage* owner, CAttachment* attach, bool launch_app, bool view);
	
	bool LaunchURL(const cdstring& url) const;
	void TryLaunch(const CAttachment* attach, CFullFileStream* aFile) const;		// Try to launch the decoded file
	void TryLaunch(const CAttachment* attach, const fspectype& fspec) const;		// Try to launch the decoded file
	
	bool HandleInternal(CMessage* owner, CAttachment* attach) const;				// See if this attachment should be handled internally by Mulberry
};

#endif
