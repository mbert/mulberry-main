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


// Source for Envelope class

#include "CMessageThread.h"

#include "CEnvelope.h"
#include "CMbox.h"
#include "CMessage.h"
#include "CMessageComparators.h"
#include "CMessageList.h"

//#include <UProfiler.h>

#include <algorithm>
#include <memory>

#pragma mark ____________________________CMessageThread

// Default constructor
CMessageThread::CMessageThread(const CMessage* msg)
{
	InitMessageThread();
	mMsg = msg;
}

// Copy constructor
CMessageThread::CMessageThread(const CMessageThread &copy)
{
	InitMessageThread();
	
	mMsg = copy.mMsg;
}

// Default destructor
CMessageThread::~CMessageThread()
{
	mMsg = NULL;
	mParent = NULL;
	
	// Do recursive delete of child as these may not be too deep
	delete mChild;
	mChild = NULL;
	
	// Do iterative delete of next as these can be deep for a large mailbox at the top level
	CMessageThread* next = mNext;
	while(next)
	{
		// Save and prevent next from deleting its next
		CMessageThread* next_next = next->mNext;
		next->mNext = NULL;
		
		// Delete the next
		delete next;
		
		// Move to the next next
		next = next_next;
	}
	mNext = NULL;
}

// Do common init
void CMessageThread::InitMessageThread()
{
	mMsg = NULL;
	mParent = NULL;
	mChild = NULL;
	mNext = NULL;
	mReFwd = false;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________statics

void CMessageThread::ThreadResults(CMbox* mbox, const threadvector& results, CMessageList* list, bool reverse)
{
	// Look at the results and generate a root set
	std::auto_ptr<CMessageThread> root(new CMessageThread(NULL));
	
	CMessageThread* parent = root.get();
	CMessageThread* last = NULL;
	unsigned long depth = 0;

	for(threadvector::const_iterator iter = results.begin(); iter != results.end(); iter++)
		ThreadResult(mbox, (*iter).first, (*iter).second, parent, last, depth, reverse);
	
	// Convert NULL messages to fakes
	FakeBuild(root->mChild);

	// Now do insertion into list
	ListBuild(root->mChild, list, 0);
}

void CMessageThread::ThreadResult(CMbox* mbox, unsigned long msgnum, unsigned long msg_depth, CMessageThread*& parent, CMessageThread*& last, unsigned long& last_depth, bool reverse)
{
	// Check for depth change
	if (last_depth == msg_depth)
	{
		// Fall through - don't need to adjust current parent
	}
	
	// Check for increase in depth
	else if (last_depth < msg_depth)
	{
		// Use last node as parent for this child
		parent = last;
	}
	
	// Check for decrease in depth
	else
	{
		// Back up parent by appropriate amount
		for(unsigned long i = last_depth; i > msg_depth; i--)
			parent = parent->mParent;
	}
		
	// Now add child to adjusted parent
	last = new CMessageThread(msgnum ? mbox->GetMessage(msgnum) : NULL);
	if (reverse)
		parent->PrependChild(last);
	else
		parent->AppendChild(last);

	last_depth = msg_depth;
}

void CMessageThread::ThreadMessages(CMessageList* list)
{
	//StProfileSection profile("\pThreading Profile", 1000, 1000);

	// Only bother if something present in the list
	if (!list->size())
		return;

	// Step 1 - link messages via header fields
	std::auto_ptr<CThreadMap> map(RefLinkMessages(list));
	
	// Step 2 - gather orphans to root
	unsigned long nroot = 0;
	std::auto_ptr<CMessageThread> root(MakeRoot(map.get(), nroot));

	// Delete map - nodes will remain
	map.reset();

	// Step 3 - prune empty threads
	PruneTree(root.get());

	// Step 4 - sort root level only
	ThreadSort(root.get(), false);

	// Step 5
	GroupSubjects(root.get(), nroot);

	// Step 6
	ThreadSort(root.get());

	// Convert NULL messages to fakes
	FakeBuild(root->mChild);

	// Now do insertion into list
	list->clear();
	ListBuild(root->mChild, list, 0);
}

CMessageThread::CThreadMap* CMessageThread::RefLinkMessages(const CMessageList* list)
{
	CThreadMap* map = new CThreadMap;

	// Add each message ID
	for(CMessageList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		// Get Message-ID
		cdstring msg_id = ((*iter)->IsFullyCached() ? (*iter)->GetEnvelope()->GetMessageID() : cdstring::null_str);

		// Create fake entry if its empty
		if (msg_id.empty())
		{
			msg_id = "MulberryMessageID-faked@";
			msg_id += cdstring((*iter)->GetMessageNumber());
		}
		else
			// Must normalise message id
			NormaliseID(msg_id.c_str_mod());

		// Step 1A: fill the map with messages
		// if we already have an entry, use it

		// See if entry exists
		CMessageThread* entry = NULL;
		CThreadMap::const_iterator found = map->find(msg_id);
		if (found != map->end())
		{
			// See if duplicate has a message associated with it
			entry = (*found).second;
			if (entry->mMsg)
			{
				// Must make it unique
				msg_id += "-duplicate";
				msg_id += cdstring((*iter)->GetMessageNumber());

				// clear cur so that we create a new container
				entry = NULL;
			}
			else
			{
				entry->mMsg = *iter;
			}
		}
		
		// Create a new entry if required
		if (!entry)
		{
			entry = new CMessageThread(*iter);
			map->insert(CThreadMap::value_type(msg_id, entry));
		}
		
		// Step 1B
		const cdstring& refshdr = ((*iter)->IsFullyCached() ? (*iter)->GetEnvelope()->GetReferences() : cdstring::null_str);
		cdstrvect refs;
		if (!refshdr.empty())
			ExplodeReferencesHeader(refshdr, refs);

		// If no references, use In-Reply-To
		if (refs.empty())
		{
			cdstring temp = ((*iter)->IsFullyCached() ? (*iter)->GetEnvelope()->GetInReplyTo() : cdstring::null_str);
			char* p = temp.c_str_mod();
			char* refid = ExtractID(&p);
			if (refid && *refid)
			{
				NormaliseID(refid);
				refs.push_back(refid);
			}
		}
		
		// Do references chaining
		CMessageThread* parent = NULL;
		for(cdstrvect::const_iterator iter2 = refs.begin(); iter2 != refs.end(); iter2++)
		{
			// Look for existing entry in hash table
			CThreadMap::iterator found = map->find(*iter2);
			CMessageThread* ref = NULL;
			if (found != map->end())
				ref = (*found).second;
			else
			{
				// Need to create new dummy entry
				ref = new CMessageThread(NULL);
				map->insert(CThreadMap::value_type(*iter2, ref));
			}

			// link the references together as parent-child iff:
			// - we won't change existing links, AND
			// - we won't create a loop
			if (!ref->mParent && parent && !ref->HasChild(parent))
				parent->PrependChild(ref);

			parent = ref;
		}

		// Step 1C
		// if we have a parent already, it is probably bogus (the result
		// of a truncated references field), so unlink from it because
		// we now have the actual parent
		if (entry->mParent)
			entry->OrphanChild();

		// Now try to link parent and child
		if (parent && !entry->HasChild(parent))
			parent->PrependChild(entry);
	}
	
	return map;
}

CMessageThread* CMessageThread::MakeRoot(CThreadMap* map, unsigned long& nroot)
{
	// Create dummy node for root
	CMessageThread* root = new CMessageThread(NULL);
	
	// Look for all nodes without a parent and make root adopt them
	for(CThreadMap::iterator iter = map->begin(); iter != map->end(); iter++)
	{
		CMessageThread* node = (*iter).second;
		if (!node->mParent)
		{
			if (node->mNext)
				continue;
			node->mNext = root->mChild;
			root->mChild = node;
			nroot++;
		}
	}
	
	return root;
}

void CMessageThread::PruneTree(CMessageThread* parent)
{
	CMessageThread* cur = parent->mChild;
	CMessageThread* prev = NULL;
	CMessageThread* next = (cur ? cur->mNext : NULL);
	
	for(; cur; prev = cur, cur = next, next = (cur ? cur->mNext : NULL))
	{
		// if we have an empty container with no children, delete it
		if (!cur->mMsg && !cur->mChild)
		{
			// Reset parent or sibling node
			if (!prev)
				parent->mChild = cur->mNext;
			else
				prev->mNext = cur->mNext;

			// make cur childless and siblingless and delete it
			cur->mChild = cur->mNext = NULL;
			delete cur;

			// Keep the same prev for the next pass
			cur = prev;
		}

		// if we have empty container with children, AND
		// we're not at the root OR we only have one child
		// then remove the container but promote its children to this level
		// (splice them into the current child list)
		else if (!cur->mMsg && cur->mChild &&
			 		(cur->mParent || !cur->mChild->mNext))
		{
			// Move cur's children into cur's place
			if (!prev)
				parent->mChild = cur->mChild;
			else
				prev->mNext = cur->mChild;

			// make cur's parent the new parent of cur's children
			CMessageThread* child = cur->mChild;
			do
			{
				child->mParent = cur->mParent;
			} while (child->mNext && (child = child->mNext));

			// make the cur's last child point to cur's next sibling
			child->mNext = cur->mNext;

			// we just replaced cur with it's children
			// so make it's first child the next node to process
			next = cur->mChild;

			// make cur childless and siblingless and delete it
			cur->mChild = cur->mNext = NULL;
			delete cur;

			// Keep the same prev for the next pass
			cur = prev;
		}

		// if we have a message with children, prune it's children
		else if (cur->mChild)
			PruneTree(cur);
	}
}

bool CMessageThread::thread_sort_comp(CMessageThread* thread1, CMessageThread* thread2)
{
	// Find a real message somewhere in the chain
	while(!thread1->mMsg)
		thread1 = thread1->mChild;
	const CMessage* msg1 = thread1->mMsg;

	// Find a real message somewhere in the chain
	while(!thread2->mMsg)
		thread2 = thread2->mChild;
	const CMessage* msg2 = thread2->mMsg;

	// Sort them
	return (*CMessageDateComparator::GetDateComparator())(const_cast<CMessage*>(msg1), const_cast<CMessage*>(msg2));
}

void CMessageThread::ThreadSort(CMessageThread* root, bool do_children)
{
	CMessageThread* child;

	// sort the grandchildren
	std::vector<CMessageThread*> list;
	child = root->mChild;
	while(child)
	{
		// Add to sort list
		list.push_back(child);
		// if the child has children, sort them
		if (do_children && child->mChild)
			ThreadSort(child);
		child = child->mNext;
	}

	// sort the children
	if (list.size())
	{
		std::sort(list.begin(), list.end(), thread_sort_comp);

		child = root->mChild = list.front();
		for(std::vector<CMessageThread*>::iterator iter = list.begin() + 1; iter != list.end(); iter++)
		{
			child->mNext = *iter;
			child = child->mNext;
		}
		child->mNext = NULL;
	}
}

void CMessageThread::GroupSubjects(CMessageThread* root, unsigned long nroot)
{
	CMessageThread* cur;
	CMessageThread* old;
	CMessageThread* prev;
	CMessageThread* next;

    // Step 5A: create a subject hash map
	CThreadMap subj_map;

    // Step 5B: populate the map with a container for each subject
    for (cur = root->mChild; cur; cur = cur->mNext)
    {
		bool re_fwd = false;
		const char* xsubj = NULL;
	
		// if the container is not empty, use it's subject
		if (cur->mMsg)
			xsubj = (cur->mMsg->IsFullyCached() ? cur->mMsg->GetEnvelope()->GetThread(cur->mReFwd) : NULL);
		// otherwise, use the subject of it's first child
		else
			xsubj = (cur->mChild->mMsg->IsFullyCached() ? cur->mChild->mMsg->GetEnvelope()->GetThread(cur->mReFwd) : NULL);

		// if subject is empty, skip it
		if (!xsubj || !*xsubj) continue;

		// lookup this subject in the table
		CThreadMap::iterator found = subj_map.find(xsubj);
		old = (found != subj_map.end()) ? (*found).second : NULL;

		// insert the current container into the table iff:
		// - this subject is not in the table, OR
		// - this container is empty AND the one in the table is not
		//   (the empty one is more interesting as a root), OR
		// - the container in the table is a re/fwd AND this one is not
		//   (the non-re/fwd is the more interesting of the two)

		if (!old ||
			(!cur->mMsg && old->mMsg) ||
			(old->mMsg && old->mReFwd &&
			cur->mMsg && !cur->mReFwd))
		{
			// Insert or replace in the map
			std::pair<CThreadMap::iterator, bool> result = subj_map.insert(CThreadMap::value_type(xsubj, cur));
			if (!result.second)
				(*result.first).second = cur;
		}
	}

    // 5C - group containers with the same subject together
	for(prev = NULL, cur = root->mChild, next = (cur ? cur->mNext : NULL);
		cur;
		prev = cur, cur = next, next = (next ? next->mNext : NULL))
	{
		bool re_fwd = false;
		const char* xsubj = NULL;
	
		// if the container is not empty, use it's subject
		if (cur->mMsg)
			xsubj = (cur->mMsg->IsFullyCached() ? cur->mMsg->GetEnvelope()->GetThread(cur->mReFwd) : NULL);
		// otherwise, use the subject of it's first child
		else
			xsubj = (cur->mChild->mMsg->IsFullyCached() ? cur->mChild->mMsg->GetEnvelope()->GetThread(cur->mReFwd) : NULL);

		// if subject is empty, skip it
		if (!xsubj || !*xsubj) continue;

		// lookup this subject in the table
		CThreadMap::iterator found = subj_map.find(xsubj);
		old = (found != subj_map.end()) ? (*found).second : NULL;

		// if we found ourselves, skip it
		if (old == cur) continue;

		// ok, we already have a container which contains our current subject,
		// so pull this container out of the root set, because we are going to
		// merge this node with another one
		if (!prev)	// we're at the root
			root->mChild = cur->mNext;
		else
			prev->mNext = cur->mNext;
		cur->mNext = NULL;

		// if both containers are dummies, append cur's children to old's
		if (!old->mMsg && !cur->mMsg)
		{
			// find old's last child
			CMessageThread* child;
			for(child = old->mChild; child->mNext; child = child->mNext) {}

			// append cur's children to old's children list
			child->mNext = cur->mChild;

			// make old the parent of cur's children
			for (child = cur->mChild; child; child = child->mNext)
				child->mParent = old;

			// make cur childless
			cur->mChild = NULL;
		}

		// if:
		// - old container is empty, OR
		// - the current message is a re/fwd AND the old one is not,
		// make the current container a child of the old one
		//
		// Note: we don't have to worry about the reverse cases
		// because step 5B guarantees that they won't happen
		else if (!old->mMsg || (cur->mMsg && cur->mReFwd && !old->mReFwd))
		{
			old->PrependChild(cur);
		}

		// if both messages are re/fwds OR neither are re/fwds,
		// then make them both children of a new dummy container
		// (we don't want to assume any parent-child relationship between them)
		//
		// perhaps we can create a parent-child relationship
		// between re/fwds by counting the number of re/fwds
		//
		// Note: we need the hash table to still point to old,
		// so we must make old the dummy and make the contents of the
		// new container a copy of old's original contents
		else
		{
		    CMessageThread* new_node = new CMessageThread(NULL);

		    // make new_node a copy of old (except parent and next)
	 	    new_node->mMsg = old->mMsg;
		    new_node->mChild = old->mChild;
		    new_node->mNext = NULL;

		    // make new_node the parent of it's newly adopted children
			CMessageThread* child;
		    for (child = new_node->mChild; child; child = child->mNext)
				child->mParent = new_node;

		    // make old the parent of cur and new
		    cur->mParent = old;
		    new_node->mParent = old;

		    // empty old and make it have two children (cur and new)
		    old->mMsg = NULL;
		    old->mChild = cur;
		    cur->mNext = new_node;
		}

		// we just removed cur from our list,
		// so we need to keep the same prev for the next pass
		cur = prev;
    }
}

// Replace NULL messages with fakes
void CMessageThread::FakeBuild(CMessageThread* node)
{
	while(node)
	{
		// May need to create a fake
		if (!node->mMsg)
		{
			// Create fake, using a child message as an example
			CMessage* fake = new CMessage((CMbox*) NULL);
			fake->MakeFake();
			node->mMsg = fake;
		}

		// Do children
		if (node->mChild)
			FakeBuild(node->mChild);
		node = node->mNext;
	}	
}

void CMessageThread::ListBuild(CMessageThread* node, CMessageList* list, unsigned long depth)
{
	CMessageThread* previous = NULL;
	while(node)
	{
		// Add to list
		list->push_back(const_cast<CMessage*>(node->mMsg));

		// Set thread details
		const_cast<CMessage*>(node->mMsg)->SetThreadInfo(depth,
															node->mParent ? node->mParent->mMsg : NULL,
															node->mChild ? node->mChild->mMsg : NULL,
															previous ? previous->mMsg : NULL,
															node->mNext ? node->mNext->mMsg : NULL);

		// Do children
		if (node->mChild)
			ListBuild(node->mChild, list, depth + 1);
		
		// Move to the next one, remember the current as the next's previous
		previous = node;
		node = node->mNext;
	}	
}

void CMessageThread::ExplodeReferencesHeader(const cdstring& hdr, cdstrvect& results)
{
	// Header has been unfolded and 1522 decoded
	cdstring temp(hdr);
	char* p = temp.c_str_mod();

	results.reserve(20);

	while(*p)
	{
		// Get each message-id from references line
		char* msgid = ExtractID(&p);
		if (msgid && *msgid)
		{
			NormaliseID(msgid);
			results.push_back(msgid);
		}
	}
}

char* CMessageThread::ExtractID(char** msg_id)
{
	if (!msg_id || !*msg_id)
		return NULL;

	// References/Message-ID format: '<...@...>'

	// Punt up to opening '<'
	while(**msg_id && (**msg_id != '<')) (*msg_id)++;

	// '<' must be present at start for valid message-id
	if (**msg_id)
	{
		// This is the start of the message-id
		char* start = *msg_id;
		
		// Punt up to '@'
		while(**msg_id && (**msg_id != '@')) (*msg_id)++;
		
		// '@' must be present in valid message-id
		if (**msg_id)
		{
			// Punt up to '>'
			while(**msg_id && (**msg_id != '>')) (*msg_id)++;
			
			// '>' must be present in valid message-id
			if (**msg_id)
			{
				// Step over '>'
				(*msg_id)++;
				
				// If the current position is not already NULL, set it to NULL
				// to terminate the string and step over it. Don't step over it if
				// we're at the very end of the input string as we want to terminate here
				if (**msg_id)
				{
					**msg_id = 0;
					(*msg_id)++;
				}
				return start;
			}
		}
	}
	
	// If we fall through there is no valid message-id in field
	return NULL;
}

// Normalise the message id to remove any quoting artifacts
void CMessageThread::NormaliseID(char* msg_id)
{
	// Assume its already in <x@y> format
	char* p = ++msg_id;
	char* q = p;
	
	// Remove any quoting on the first part
	if (*p == '\"')
	{
		// Ignore start quote
		p++;
		while(*p && (*p != '"'))
		{
			switch(*p)
			{
			case '\\':
				// Skip to next char
				p++;
				if (*p)
					*q++ = *p++;
				break;
			default:
				*q++ = *p++;
				break;
			}
		}
		
		// Ignore trailing quote
		if (*p)
			p++;
	}
	
	// Punt to/past '@'
	while(*p && (*p != '@'))
		*q++ = *p++;
	*q++ = *p++;
	
	// Check for '[...]'
	if (*p == '[')
	{
		// Remove any \ quotes to the end of the string
		while(*p)
		{
			switch(*p)
			{
			case '\\':
				p++;
				if (*p)
					*q++ = *p++;
				break;
			default:
				*q++ = *p++;
				break;
			}
		}
	}
	else
	{
		// Copy remainder of string
		while(*p)
			*q++ = *p++;
	}
	
	// Add the NULL
	*q = 0;
}

#pragma mark ____________________________members

// Check whether child is a descendent of this
bool CMessageThread::HasChild(const CMessageThread* child) const
{
	// self
	if (child == this)
		return true;

	// search each child's decendents
	for(const CMessageThread* cid = mChild; cid; cid = cid->mNext)
	{
		if (cid->HasChild(child))
			return true;
	}
	
	// Fall through => no match
	return false;
}

// Add child to start of child list in this
void CMessageThread::PrependChild(CMessageThread* child)
{
	child->mParent = this;
	child->mNext = mChild;
	mChild = child;
}

// Add child to end of child list in this
void CMessageThread::AppendChild(CMessageThread* child)
{
	child->mParent = this;
	child->mNext = NULL;
	
	// Check whether existing childern exist
	CMessageThread* last = mChild;
	if (!last)
		mChild = child;
	else
	{
		while(last->mNext)
			last = last->mNext;
		last->mNext = child;
	}
}

// Unlink child from parent's children.
void CMessageThread::OrphanChild()
{
	CMessageThread* prev;
	CMessageThread* cur;

	// sanity check -- make sure child is actually a child of parent
	for(prev = NULL, cur = mParent->mChild;
		cur != this && cur != NULL; prev = cur, cur = cur->mNext) {}

	if (!cur)
		return;

	// unlink child
	if (!prev)	// first child
		mParent->mChild = mNext;
	else
		prev->mNext = mNext;
	mParent = mNext = NULL;
}
