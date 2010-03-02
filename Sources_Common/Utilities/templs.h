/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Header for basic instantiated template type

#ifndef __TEMPLS__MULBERRY__
#define __TEMPLS__MULBERRY__

#include <map>
//#include <multimap>
#include <set>
#include <vector>

typedef std::vector<bool> boolvector;
typedef std::vector<long> lvector;
typedef std::vector<unsigned long> ulvector;
typedef std::map<unsigned long, unsigned long> ulmap;
typedef std::multimap<unsigned long, unsigned long> ulmultimap;
typedef std::set<unsigned long> ulset;

#endif
