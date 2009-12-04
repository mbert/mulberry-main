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


// Header for CGroupComparators class

#ifndef __CGROUPCOMPARATORS__MULBERRY__
#define __CGROUPCOMPARATORS__MULBERRY__

// Classes

class CGroup;

typedef bool (*GroupCompare)(const CGroup*, const CGroup*);

class CGroupComparator {

public:
	CGroupComparator();
	virtual ~CGroupComparator();

	virtual GroupCompare	GetComparator()
						{ return CompareGroupName; }

	static bool	CompareGroupName(const CGroup* aGrp1, const CGroup* aGrp2);

};


#endif
