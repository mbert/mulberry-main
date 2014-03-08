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


// Header for ptrvector template class

#ifndef __PTRVECTOR__MULBERRY__
#define __PTRVECTOR__MULBERRY__

#include <cstddef>
#include <vector>
#include <stddef.h>

// Classes

#ifdef MSIPL_USING_NAMESPACE
	namespace std {
#endif

template <class T> class ptrvector : public std::vector<T*>
{
public:
		typedef typename std::vector<T*>::iterator           iterator;
		typedef typename std::vector<T*>::size_type         size_type;

		ptrvector() {_delete_data = true;};
		ptrvector(const ptrvector& copy)
			{ _copy(copy); };
	~ptrvector()
		{ _tidy(); };											// Force object deletion

    ptrvector<T>& operator= (const ptrvector<T>& copy)
    	{ if (this != &copy) { _tidy(); _copy(copy); } return *this; }

	void set_delete_data(bool delete_data)
		{ _delete_data = delete_data; }

	// Special to delete objects
	void resize(size_type sz, T* c = NULL);
	void pop_back ();									// Delete address as well
	iterator erase(iterator position);						// Delete address as well
    iterator erase(iterator first, iterator last);			// Delete addresses as well
	void  clear();

private:
	bool _delete_data;

	void _copy(const ptrvector& copy);
	void _tidy()
		{ erase(this->begin(), this->end()); }
};

// Copy construct
template <class T> void ptrvector<T>::_copy(const ptrvector<T>& copy)
{
	// Since we're creating new ones we must delete them
	_delete_data = true;

	// Copy all objects
	for(typename ptrvector<T>::const_iterator iter = copy.begin(); iter != copy.end(); iter++)
		this->push_back(new T(**iter));
}

template <class T>
inline
bool
operator==(const ptrvector<T>& x, const ptrvector<T>& y)
{
    if (x.size() != y.size())
    	return false;
    
    // Compare each object
    typename ptrvector<T>::const_iterator iter1 = x.begin();
    typename ptrvector<T>::const_iterator iter2 = y.begin();
	for(; iter1 != x.end(); iter1++, iter2++)
	   if (!(**iter1 == **iter2)) return false;
	  
	return true;
}

template <class T>
inline
bool
operator!=(const ptrvector<T>& x, const ptrvector<T>& y)
{
   	return !(x == y);
}

template <class T> void ptrvector<T>::resize(size_type sz, T* c)
{
	if (sz > this->size())
		insert(this->end(), sz - this->size(), c);
	else if (sz < this->size())
		erase(this->begin() + sz, this->end());
}

// Delete object as well
template <class T> void ptrvector<T>::pop_back()
{
	if (_delete_data)
		delete this->back();
	std::vector<T*>::pop_back();
}

// Delete object as well
template <class T> typename ptrvector<T>::iterator ptrvector<T>::erase(iterator position)
{
	if (_delete_data)
		delete *position;
	return std::vector<T*>::erase(position);
}

// Delete object as well
template <class T> typename ptrvector<T>::iterator ptrvector<T>::erase(iterator first, iterator last)
{
	if (_delete_data)
	{
		for(iterator iter = first; iter != last; iter++)
			delete *iter;
	}

	return std::vector<T*>::erase(first, last);
}

// Delete objects as well
template <class T> void ptrvector<T>::clear()
{
	erase(this->begin(), this->end());
}

#ifdef MSIPL_USING_NAMESPACE
	} // namespace std 
#endif

#endif
