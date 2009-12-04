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


// Class to implement a 32-bit flag

#ifndef __SBITFLAGS__MULBERRY__
#define __SBITFLAGS__MULBERRY__

struct SBitFlags
{
	SBitFlags()
		{ mFlags = 0; }
	SBitFlags(const SBitFlags& copy)
		{ mFlags = copy.mFlags; }
	SBitFlags(unsigned long flags)
		{ mFlags = flags; }
	~SBitFlags() {}

	SBitFlags& operator=(const SBitFlags& copy)	// Assignment with same type
		{ if (this != &copy) mFlags = copy.mFlags; return *this; }
	SBitFlags& operator=(unsigned long flags)	// Assignment with flags
		{ mFlags = flags; return *this; }

	bool IsSet(unsigned long flag) const
		{ return (mFlags & flag) == flag; }
	bool IsUnset(unsigned long flag) const
		{ return (mFlags & flag) == 0; }
	void Set(unsigned long flag, bool add = true)
		{ mFlags  = (add ? (mFlags | flag) : (mFlags & ~flag)); }

	unsigned long Get() const
		{ return mFlags; }

private:
	unsigned long mFlags;
};

#endif
