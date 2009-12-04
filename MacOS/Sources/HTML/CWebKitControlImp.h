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

// ===========================================================================
//	CWebKitControlImp.h			PowerPlant 2.2.2	©1997-2000 Metrowerks Inc.
// ===========================================================================

#ifndef _H_CWebKitControlImp
#define _H_CWebKitControlImp
#pragma once

#include <LAMControlImp.h>

#include "cdstring.h"

PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------

class	CWebKitControlImp : public LAMControlImp {
public:
	CWebKitControlImp( LStream* inStream = nil );
	
	virtual				~CWebKitControlImp();
	
	void SetURL(const cdstring& url);
	void SetData(const cdstring& data);
	
protected:
		HIViewRef	mViewRef;
	
	virtual void		MakeMacControl(
									   ConstStringPtr	inTitle,
									   SInt32			inRefCon);
	
};

PP_End_Namespace_PowerPlant

#endif
