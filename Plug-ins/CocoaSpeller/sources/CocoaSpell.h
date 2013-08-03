//
//  CocoaSpell.h
//  CocoaSpeller
//
//  Created by Cyrus Daboo on 10/21/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#ifdef __cplusplus
extern "C" {
#endif

int UniqueSpellDocumentTag(void);
void CloseSpellDocumentWithTag(int tag);

CFRange CheckSpellingOfString(CFStringRef stringToCheck, int startingOffset, int tag);
void IgnoreWord(CFStringRef wordToIgnore, int tag);
void LearnWord(CFStringRef word);
CFArrayRef CopyGuessesForWord(CFStringRef stringToGuess);

int SetLanguage(CFStringRef language);
CFStringRef CopyLanguage(void);
CFArrayRef CopyAvailableLanguages(void);

#ifdef __cplusplus
}
#endif
