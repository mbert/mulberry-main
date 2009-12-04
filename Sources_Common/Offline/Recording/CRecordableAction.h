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


// Header for an Recordable Action class

#ifndef __CRECORDABLEACTION__MULBERRY__
#define __CRECORDABLEACTION__MULBERRY__

#include <istream>
#include <ostream>

class CRecordableAction
{
public:
	CRecordableAction() {}
	virtual ~CRecordableAction() {}

	virtual void WriteToStream(std::ostream& out, bool text = false) const = 0;
	virtual void ReadFromStream(std::istream& in, unsigned long vers) = 0;
};

#endif
