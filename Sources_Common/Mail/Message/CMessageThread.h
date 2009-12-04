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


// Header for CMessageThread class

#ifndef __CMESSAGETHREAD__MULBERRY__
#define __CMESSAGETHREAD__MULBERRY__

#include "CMboxFwd.h"

#include "cdstring.h"

#include <map>

class CMbox;
class CMessage;
class CMessageList;

class CMessageThread
{
public:
			explicit CMessageThread(const CMessage* msg);
			CMessageThread(const CMessageThread &copy);				// Copy constructor
			~CMessageThread();

	static void				ThreadResults(CMbox* mbox, const threadvector& results, CMessageList* list, bool reverse);
	static void				ThreadMessages(CMessageList* list);

protected:
	const CMessage*	mMsg;
	CMessageThread*	mParent;
	CMessageThread*	mChild;
	CMessageThread*	mNext;
	bool			mReFwd;

	typedef std::map<cdstring, CMessageThread*> CThreadMap;

	// static to build tree from server results
	static void				ThreadResult(CMbox* mbox, unsigned long msgnum, unsigned long msg_depth,
											CMessageThread*& parent, CMessageThread*& last, unsigned long& last_depth, bool reverse);

	// statics to build tree from cached list
	static CThreadMap*		RefLinkMessages(const CMessageList* list);
	static CMessageThread*	MakeRoot(CThreadMap* map, unsigned long& nroot);
	static void				PruneTree(CMessageThread* root);
	static void				ThreadSort(CMessageThread* root, bool do_children = true);
	static void				GroupSubjects(CMessageThread* root, unsigned long nroot);

	static void				ExplodeReferencesHeader(const cdstring& hdr, cdstrvect& results);
	static char*			ExtractID(char** msg_id);
	static void				NormaliseID(char* msg_id);

	// common statics for local and server threading
	static void				FakeBuild(CMessageThread* root);
	static void				ListBuild(CMessageThread* root, CMessageList* list, unsigned long depth);

	// Members
	bool HasChild(const CMessageThread* child) const;
	void PrependChild(CMessageThread* child);
	void AppendChild(CMessageThread* child);
	void OrphanChild();
	const CMessage* FindChildMsg() const;

private:
	void	InitMessageThread();									// Common init

	static bool thread_sort_comp(CMessageThread*, CMessageThread*);
};

#endif
