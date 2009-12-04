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

#define HELP_ITEM(x) \
	extern const JCharacter* cHelpName_##x; \
	extern const JCharacter* cHelpTitle_##x; \
	extern const JCharacter* cHelpText_##x;
	
#define HELP_NAME(x) cHelpName_##x
#define HELP_TITLE(x) cHelpTitle_##x
#define HELP_TEXT(x) cHelpText_##x
	
HELP_ITEM(TOC)

const JCharacter* cHelpSectionName[] =
	{
		HELP_NAME(TOC)
	};

const JCharacter* cHelpSectionTitle[] =
	{
		HELP_TITLE(TOC)
	};

const JCharacter* cHelpSectionText[] =
	{
		HELP_TEXT(TOC)
	};

const JSize cHelpSectionCount = sizeof(cHelpSectionName) / sizeof(JCharacter*);
