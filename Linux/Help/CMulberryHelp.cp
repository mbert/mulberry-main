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


// CMulberryHelp

#include "jTypes.h"

#define HELP_SET(w, x, y) \
	const JCharacter* cHelpName_##w = x; \
	const JCharacter* cHelpTitle_##w = y; \
	const JCharacter* cHelpText_##w =

HELP_SET(TOC, "HelpTOC", "Table Of Contents")
#include "toc.html"
