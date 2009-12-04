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


// Header for CModelessDialog class

#ifndef __CModelessDialog__MULBERRY__
#define __CModelessDialog__MULBERRY__

#include <JXDialogDirector.h>

#include <stdint.h>

class CModelessDialog : public JXDialogDirector
{
public:
	CModelessDialog(JXDirector* supervisor);

protected:
	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

	virtual void	OnOK();
	virtual void	OnCancel();

	virtual uint32_t&	TitleCounter() = 0;
};
#endif
