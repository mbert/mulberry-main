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
//	CWebKitControl.h				PowerPlant 2.2.2	©1997-2001 Metrowerks Inc.
// ===========================================================================

#ifndef _H_CWebKitControl
#define _H_CWebKitControl
#pragma once

#include <LControlView.h>

#include "cdstring.h"

PP_Begin_Namespace_PowerPlant

// ---------------------------------------------------------------------------

class	CWebKitControl : public LControlView {
public:
	enum { class_ID		= FOUR_CHAR_CODE('webc'),
		imp_class_ID	= FOUR_CHAR_CODE('iweb') };
	
	CWebKitControl(
				   LStream*			inStream,
				   ClassIDT			inImpID = imp_class_ID);
	
	CWebKitControl(
				   const SPaneInfo&	inPaneInfo,
				   const SViewInfo&	inViewInfo,
				   MessageT			inMessage,
				   SInt16				inControlKind,
				   ClassIDT			inImpID = imp_class_ID);
	
	virtual				~CWebKitControl();
	
	void SetURL(const cdstring& url);
	void SetData(const cdstring& data);
	
protected:
		virtual void		AdjustMouseSelf(
											Point				inPortPt,
											const EventRecord&	inMacEvent,
											RgnHandle			outMouseRgn);
	
private:
		CWebKitControl();
	CWebKitControl( const CWebKitControl& );
	CWebKitControl&		operator = ( const CWebKitControl& );
	
	void				InitCWebKitControl();
};

PP_End_Namespace_PowerPlant

#endif
