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


// CPrintText.h - UI widget that allows plain text captions to be printed

#ifndef __CPrintText__MULBERRY__
#define __CPrintText__MULBERRY__

#include "LCaption.h"

class CPrintText : public LCaption
{
public:
	enum { class_ID = 'Ptxt' };

					CPrintText(LStream *inStream);
	virtual 		~CPrintText();

	void SetText(const cdstring& text)
	{
		mText = text;
	}

protected:
	virtual void	DrawSelf();

private:
	cdstring			mText;
};

#endif
