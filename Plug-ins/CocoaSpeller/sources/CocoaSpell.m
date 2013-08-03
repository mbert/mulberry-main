//
//  CocoaSpell.m
//  CocoaSpeller
//
//  Created by Cyrus Daboo on 10/21/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import "CocoaSpell.h"

int UniqueSpellDocumentTag(void)
{
    int  tag;
 
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    tag  = [NSSpellChecker uniqueSpellDocumentTag];
    [pool release];
 
    return tag;
}

void CloseSpellDocumentWithTag(int tag)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [[NSSpellChecker sharedSpellChecker] closeSpellDocumentWithTag:tag];
    [pool release];
}

CFRange CheckSpellingOfString(CFStringRef stringToCheck, int startingOffset, int tag)
{
    NSRange range = {0,0};
 
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    range = [[NSSpellChecker sharedSpellChecker]
                    checkSpellingOfString:(NSString *) stringToCheck
                    startingAt:startingOffset
					language:NULL
					wrap:(BOOL)false
					inSpellDocumentWithTag:tag
					wordCount:NULL];

    [pool release];
    return ( *(CFRange *)&range );
}

void IgnoreWord(CFStringRef wordToIgnore, int tag)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [[NSSpellChecker sharedSpellChecker] ignoreWord:(NSString *) wordToIgnore inSpellDocumentWithTag:tag];
    [pool release];
}

void LearnWord(CFStringRef word)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    [[NSSpellChecker sharedSpellChecker] learnWord:(NSString *)word];
    [pool release];
}

CFArrayRef CopyGuessesForWord(CFStringRef stringToGuess)
{
	NSArray *guesses = NULL;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    guesses  = [[[NSSpellChecker sharedSpellChecker] guessesForWord:(NSString *) stringToGuess] retain];
    [pool release];
	
	return (CFArrayRef)guesses;
}

int SetLanguage(CFStringRef language)
{
	BOOL result = false;
	
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    result = [[NSSpellChecker sharedSpellChecker] setLanguage:(NSString *) language];
    [pool release];
	
	return result;
}

CFStringRef CopyLanguage(void)
{
	NSString* result = NULL;
	
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    result = [[[NSSpellChecker sharedSpellChecker] language] retain];
    [pool release];
	
	return (CFStringRef)result;
}

CFArrayRef CopyAvailableLanguages(void)
{
	NSArray *languages = NULL;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    languages  = [[[NSSpellChecker sharedSpellChecker] availableLanguages] retain];
    [pool release];
	
	return (CFArrayRef)languages;
}

