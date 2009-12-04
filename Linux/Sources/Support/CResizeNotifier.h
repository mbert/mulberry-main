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


// Header for CResizeNotifier class

#ifndef __CRESIZENOTIFIER__MULBERRY__
#define __CRESIZENOTIFIER__MULBERRY__

#include <JXUpRect.h>

class CResizeNotifier : public JXUpRect
{
public:
	CResizeNotifier(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h) :
		JXUpRect(enclosure, hSizing, vSizing, x, y, w, h) {}

protected:
	virtual void	EnclosingBoundsResized(const JCoordinate dw, const JCoordinate dh);

public:

	// JBroadcaster messages

	static const JCharacter* kResized;

	class Resized : public JBroadcaster::Message
	{
	public:

		Resized(const JCoordinate dw, const JCoordinate dh)
			: JBroadcaster::Message(kResized), dw_(dw), dh_(dh) {}

		JCoordinate dw() const
			{ return dw_; }
		JCoordinate dh() const
			{ return dh_; }

	private:
			JCoordinate dw_;
			JCoordinate dh_;
	};
};

#endif
