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

#ifndef _CTWISTER_H
#define _CTWISTER_H

#include "JXMultiImageCheckbox.h"

class CTwister : public JXMultiImageCheckbox
{ 
public:
  CTwister 
	(
	JXContainer*		pCont,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y
	)  : JXMultiImageCheckbox( pCont, hSizing, vSizing, x,y, 16,16)
    {
      SetImages(IDI_TWISTUP, IDI_TWISTUPPUSHED, IDI_TWISTDOWN, IDI_TWISTDOWNPUSHED);
      SetBorderWidth(0);
    }
  CTwister 
	(
	JXContainer*		pCont,
	const HSizingOption	hSizing,
	const VSizingOption	vSizing,
	const JCoordinate	x,
	const JCoordinate	y,
	const JCoordinate w, // Note! w and h will be ignored and are only here 
	const JCoordinate h  // for use with jxlayout
	)  : JXMultiImageCheckbox( pCont, hSizing, vSizing, x,y, 16,16)
    {
      SetImages(IDI_TWISTUP, IDI_TWISTUPPUSHED, IDI_TWISTDOWN, IDI_TWISTDOWNPUSHED);
      SetBorderWidth(0);
    }
};


#endif
