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


#include "CStack.h"
#include <iostream.h>

CStackElement::CStackElement(char *string){
	value = string;
	next = NULL;
}

CStackElement::~CStackElement(){
}


CStack::CStack(){
	stack = NULL;
}

CStack::~CStack(){
	deleteStack(stack);
}

void CStack::deleteStack(CStackElement *member){
	if(member){
		deleteStack(member->next);
		delete member;
	}
}

void CStack::push(char *string){
	CStackElement *element = new CStackElement(string);
	element->next = stack;
	stack = element;
}

char *CStack::pop(){
	CStackElement *give = stack;
	if(stack)
		stack = stack->next;
	else
		stack = NULL;
	
	delete give;
	
	return give->value;
}