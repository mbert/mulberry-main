/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Header for CAddressFieldContainer class

#ifndef __CADDRESSFIELDCONTAINER__MULBERRY__
#define __CADDRESSFIELDCONTAINER__MULBERRY__

// Constants

// Messages

// Resources

// Classes
class CAdbkAddress;
class CAddressFieldBase;
class CAddressFieldSubContainer;

class CAddressFieldContainer : public LView
{
	friend class CAddressFieldBase;
    friend class CAddressFieldSubContainer;

public:
	enum { class_ID = 'AflC' };

					CAddressFieldContainer();
					CAddressFieldContainer(LStream *inStream);
	virtual 		~CAddressFieldContainer();

	unsigned long	GetCount() const
		{ return mFields.GetCount(); }

	void	SetAddress(const CAdbkAddress* addr);
	bool	GetAddress(CAdbkAddress* addr);

protected:
	LWindow*		mContainer;

	virtual void	FinishCreateSelf(void);

	void	DoActivate();

	CAddressFieldBase*          AddField(bool multi=false);
	CAddressFieldSubContainer*	AddContainer(int ctype);
	void                AddView(LView* view);
    
    void                Layout();

private:
	TArray<LPane*>	mFields;
};

#endif
