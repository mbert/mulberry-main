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


// Header for CMboxList class

#ifndef __CMBOXLIST__MULBERRY__
#define __CMBOXLIST__MULBERRY__

#include "CTreeNodeList.h"

// Classes

class CMbox;
class CMboxProtocol;
class CMboxRef;
class CMboxRefList;

class CMboxList : public CTreeNodeList
{
public:
			CMboxList();
	explicit CMboxList(CMboxProtocol* owner);
	explicit CMboxList(CMboxProtocol* owner, const char* root);
	virtual ~CMboxList();

	CMboxProtocol* GetProtocol()
		{ return mProtocol; }

	virtual bool		IsRootName(char dir) const;

	long GetHierarchyIndex() const;

	bool CanCreateInferiors() const;								// Can this root be used for inferiors?

	bool NeedsHierarchyDescovery() const;							// Needs hierarchy descovery

	void	Add(CMboxProtocol* proto, bool not_dirs = true);		// Add all mailboxes in protocol to this list
	void	Add(CMboxList* proto, bool not_dirs = true);			// Add all mailboxes in another list to this one
	void	Add(CMboxRefList* proto, bool not_dirs = true);			// Add all mailboxes in another list to this one
	void	Add(CMbox* mbox, bool not_dirs = true);					// Add mailbox to this list
	void	Add(CMboxRef* mbox_ref, bool not_dirs = true);			// Add mailbox to this list

	
	CMbox*			FindMbox(const char* mbox) const;					// Return mbox with this name
	CMbox*			FindMboxURL(const char* url) const;					// Return mbox with this url
	void			FindWildcard(const cdstring& pattern,
									CMboxList& list,
									bool no_dir = false) const;			// Return all mboxes matching pattern

	long			FetchIndexOf(const CMbox* mbox) const;				// Get position of mbox in the list

	void		CloseAll();											// Close all items

	void		RemoveDuplicates();									// Remove duplicate items

	void		DumpHierarchy(std::ostream& out);
	void		ReadHierarchy(std::istream& in, CMboxProtocol* local, bool add_items = true);
	
protected:
	CMboxProtocol*	mProtocol;

	virtual CTreeNode*	MakeHierarchy(const cdstring& name, const CTreeNode* child);
	virtual void		MakeHierarchic(CTreeNode* node, bool hier);

private:
	void InitMboxList();

};

typedef std::vector<CMboxList*> CHierarchies;

#endif
