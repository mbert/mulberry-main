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


//	This class defines a header and footer class. This is essentially an
//	edit field with a specialized PrintPanelSelf() member function.

#ifndef __CHEADANDFOOT__MULBERRY__
#define __CHEADANDFOOT__MULBERRY__

const long cHeadAndFootInset = 4;

class CHeadAndFoot : public LEditField {

public:

				CHeadAndFoot(bool use_box = true);
	virtual		~CHeadAndFoot();
				CHeadAndFoot(LStream *inStream);


	virtual void	PrintPanelSelf(const PanelSpec &inPanel);
	virtual	void	SetFileName(ConstStr255Param inFileName);

protected:
	Str255	mFileName;
};


#endif
