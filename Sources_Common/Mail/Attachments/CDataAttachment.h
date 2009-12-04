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


//	CDataAttachment.h

#ifndef __CDATAATTACHMENT__MULBERRY__
#define __CDATAATTACHMENT__MULBERRY__

#include "CAttachment.h"

// Classes

class	CDataAttachment : public CAttachment {

public:
						CDataAttachment();
	explicit			CDataAttachment(char* text);					// Create text body
						CDataAttachment(const CDataAttachment& copy);
	virtual				~CDataAttachment();

	virtual bool		CanDisplay() const;
	virtual bool		CanEdit() const;
	virtual bool		CanChange() const
							{ return true; }
};

#endif
