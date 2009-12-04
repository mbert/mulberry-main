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

/******************************************************************************
 JXTextCheckbox3.h

 ******************************************************************************/

#ifndef _H_JXTextCheckbox3
#define _H_JXTextCheckbox3

#include <JXTextCheckbox.h>

class JXTextCheckbox3 : public JXTextCheckbox
{
public:

	JXTextCheckbox3(const JCharacter* label, JXContainer* enclosure,
		const HSizingOption hSizing, const VSizingOption vSizing,
		const JCoordinate x, const JCoordinate y,
		const JCoordinate w, const JCoordinate h);

	virtual void	SetValue(int value);
	virtual int		GetValue() const
		{ return mValue; }

	virtual void	SetState(const JBoolean on);

protected:
	int mValue;

	virtual void	Draw(JXWindowPainter& p, const JRect& rect);

private:
	// not allowed

	JXTextCheckbox3(const JXTextCheckbox3& source);
	const JXTextCheckbox3& operator=(const JXTextCheckbox3& source);
};

#endif
