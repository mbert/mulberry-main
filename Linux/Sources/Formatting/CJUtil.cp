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


//#include <iostream>
#include "CJUtil.h"
#include "cdstring.h"
#include "math.h"
#include "stdlib.h"
#include <functional>

bool strrangecmp(const char * match, const char *piece, int start, int stop){
	if(strlen(match) < stop - start){
	}
	else{
		bool matching = true;
		
		for(int i = 0; (i < (stop - start)) && matching; i++){
			if(match[i] != piece[i + start])
				return false;
		}
		
		return true;
	}
return false;
}



char *longtohex(long number){
	bool adding = false;
	long it = 0;
	char values[16];
	cdstring string;
	long double s = 16;
	long powered;
	strcpy(values, "0123456789ABCDEF");
	
	for(long double power = 5; power >= 0; power--){
		powered = pow(s,power);
		it = number / powered;
		if(adding || it > 0){
			string += values[it];
			adding = true;	
		}	
		number = number % powered;
	}
	return string.grab_c_str();
}	


char hextochar(const char *theString){
    register char digit;

    digit = (theString[0] >= 'A' ? ((theString[0] & 0xdf) - 'A')+10 : (theString[0] - '0'));
    digit *= 16;
    digit += (theString[1] >= 'A' ? ((theString[1] & 0xdf) - 'A')+10 : (theString[1] - '0'));
    return(digit);
}
