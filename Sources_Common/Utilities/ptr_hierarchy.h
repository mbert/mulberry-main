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


// ptr_hierarchy template class

// Used to implement a fully hierarchic list of pointers to objects.
// Type of data stored for node or leaf can be different.

#ifndef __PTR_HIERARCHY__MULBERRY__
#define __PTR_HIERARCHY__MULBERRY__

#ifdef __MSL__
#include <mslconfig>
#endif

#include <vector>
#include <algorithm>

#pragma mark ____________________________class ptr_hierarchy
template <class L, class N> class ptr_hierarchy;

template <class L, class N> class ptr_hierarchy
{
public:
#ifdef __GNUC__
	typedef ptr_hierarchy<L, N> node_type;
	typedef std::vector<ptr_hierarchy<L, N>*> node_list;
	typedef L select_type;
	typedef N noselect_type;
#else
	typedef typename ptr_hierarchy<L, N> node_type;
	typedef typename std::vector<ptr_hierarchy<L, N>*> node_list;
	typedef typename L select_type;
	typedef typename N noselect_type;
#endif

	ptr_hierarchy(bool noinferiors);
	ptr_hierarchy(select_type* data, bool noinferiors);
	ptr_hierarchy(noselect_type* data, bool noinferiors);
	~ptr_hierarchy();
		
	// Getters/Setters
	bool IsHierarchic() const
		{ return !_noinferiors && (_noselect || (_children && !_children->empty())); }
	bool IsSelectable() const
		{ return !_noselect; }
	bool HasChildren() const
		{ return !_noinferiors && _children && !_children->empty(); }

	const node_list* GetChildren() const
		{ return _children; }
	node_list* GetChildren()
		{ return _children; }

	void SetChildren(node_list* children)
		{ _children = children; }

	size_t CountChildren() const
		{ return (_children ? _children->size() : 0); }

	select_type* GetSelectData()
		{ return _select_data; }
	const select_type* GetSelectData() const
		{ return _select_data; }
	noselect_type* GetNoSelectData()
		{ return _noselect_data; }
	const noselect_type* GetNoSelectData() const
		{ return _noselect_data; }

	void SetData(select_type* select)
		{ tidy_data(); _select_data = select; _noselect = false; }
	void SetData(noselect_type* noselect)
		{ tidy_data(); _noselect_data = noselect; _noselect = true; }

	node_type* push_back(node_type* node);
	node_type* push_back(select_type* data, bool noinferiors);
	node_type* push_back(noselect_type* data, bool noinferiors);

	bool find(const select_type* data, int& pos) const;
	bool find(const noselect_type* data, int& pos) const;
	bool findval(const select_type* data, int& pos) const;
	bool findval(const noselect_type* data, int& pos) const;

	void erase_children();
	int erase(int& pos);
	int erase(node_type* node);
	int erase(select_type* data);
	int erase(noselect_type* data);

	size_t size() const
		{ return (_children ? _children->size() : 0); }

private:
	node_list*	_children;
	bool		_noinferiors;
	bool		_noselect;
	bool		_expanded;
	union
	{
		select_type*	_select_data;
		noselect_type*	_noselect_data;
	};
	
	void tidy_data();
};

#pragma mark ____________________________List

template <class L, class N> ptr_hierarchy<L, N>::ptr_hierarchy(bool noinferiors)
{
	_children = NULL;
	_noinferiors = noinferiors;
	_noselect = false;
	_expanded = false;
	_select_data = NULL;
}

template <class L, class N> ptr_hierarchy<L, N>::ptr_hierarchy(select_type* data, bool noinferiors)
{
	_children = NULL;
	_noinferiors = noinferiors;
	_noselect = false;
	_expanded = false;
	_select_data = data;
}

template <class L, class N> ptr_hierarchy<L, N>::ptr_hierarchy(noselect_type* data, bool noinferiors)
{
	_children = NULL;
	_noinferiors = noinferiors;
	_noselect = true;
	_expanded = false;
	_noselect_data = data;
}

template <class L, class N> ptr_hierarchy<L, N>::~ptr_hierarchy()
{
	// Dump all children
	erase_children();

	// Dump data
	tidy_data();
}

template <class L, class N> typename ptr_hierarchy<L, N>::node_type* ptr_hierarchy<L, N>::push_back(node_type* node)
{
	// Create list
	if (!_children)
		_children = new node_list;

	// Create new node and add to list
	_children->push_back(node);
	return node;
}

template <class L, class N> typename ptr_hierarchy<L, N>::node_type* ptr_hierarchy<L, N>::push_back(select_type* data, bool noinferiors)
{
	// Create list
	if (!_children)
		_children = new node_list;

	// Create new node and add to list
	node_type* node = new node_type(data, noinferiors);
	_children->push_back(node);
	return node;
}

template <class L, class N> typename ptr_hierarchy<L, N>::node_type* ptr_hierarchy<L, N>::push_back(noselect_type* data, bool noinferiors)
{
	// Create list
	if (!_children)
		_children = new node_list;

	// Create new node and add to list
	node_type* node = new node_type(data, noinferiors);
	_children->push_back(node);
	return node;
}

template <class L, class N> bool ptr_hierarchy<L, N>::find(const select_type* data, int& pos) const
{
	// Test self
	pos++;
	if (IsSelectable() && (_select_data == data))
		return true;

	// Only if children
	if (!_children)
		return false;

	// Look at all items
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++)
		if ((*iter)->find(data, pos))
			return true;

	return false;
}

template <class L, class N> bool ptr_hierarchy<L, N>::find(const noselect_type* data, int& pos) const
{
	// Test self
	pos++;
	if (!IsSelectable() && (_noselect_data == data))
		return true;

	// Only if children
	if (!_children)
		return false;

	// Look at all items
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++)
		if ((*iter)->find(data, pos))
			return true;

	return false;
}

template <class L, class N> bool ptr_hierarchy<L, N>::findval(const select_type* data, int& pos) const
{
	// Test self
	pos++;
	if (IsSelectable() && (*_select_data == *data))
		return true;

	// Only if children
	if (!_children)
		return false;

	// Look at all items
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++)
		if ((*iter)->findval(data, pos))
			return true;

	return false;
}

template <class L, class N> bool ptr_hierarchy<L, N>::findval(const noselect_type* data, int& pos) const
{
	// Test self
	pos++;
	if (!IsSelectable() && (*_noselect_data == *data))
		return true;

	// Only if children
	if (!_children)
		return false;

	// Look at all items
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++)
		if ((*iter)->findval(data, pos))
			return true;

	return false;
}

template <class L, class N> void ptr_hierarchy<L, N>::erase_children()
{
	// Dump all children
	if (_children)
		for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++)
			delete *iter;
	delete _children;
	_children = NULL;
}

template <class L, class N> int ptr_hierarchy<L, N>::erase(int& pos)
{
	// Test self
	pos--;
	if (!pos)
		return 1;

	// Only if children
	if (!_children)
		return 0;

	// Look at all items
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++)
		if ((*iter)->erase(pos))
		{
			erase(*iter);
			return 0;
		}

	return 0;
}

template <class L, class N> int ptr_hierarchy<L, N>::erase(node_type* node)
{
	// Only if children
	if (!_children)
		return -1;

	// Look at all items
	typename node_list::iterator iter = std::find(_children->begin(), _children->end(), node);
	if (iter != _children->end())
	{
		int pos = (iter - _children->begin()) >> 2;
		_children->erase(iter);
		return pos;
	}

	return -1;
}

template <class L, class N> int ptr_hierarchy<L, N>::erase(select_type* data)
{
	// Only if children
	if (!_children)
		return -1;

	// Look at all items
	int pos = 0;
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++, pos++)
		if ((*iter)->IsSelectable() && ((*iter)->_select_data == data))
		{
			_children->erase(iter);
			return pos;
		}

	return -1;
}

template <class L, class N> int ptr_hierarchy<L, N>::erase(noselect_type* data)
{
	// Only if children
	if (!_children)
		return -1;

	// Look at all items
	int pos = 0;
	for(typename node_list::iterator iter = _children->begin(); iter != _children->end(); iter++, pos++)
		if (!(*iter)->IsSelectable() && ((*iter)->_noselect_data == data))
		{
			_children->erase(iter);
			return pos;
		}

	return -1;
}

template <class L, class N> void ptr_hierarchy<L, N>::tidy_data()
{
	// Dump data
	if (_noselect)
	{
		delete _noselect_data;
		_noselect_data = NULL;
	}
	else
	{
		delete _select_data;
		_select_data = NULL;
	}
}

#endif
