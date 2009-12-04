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

// GPGTest.cp

// Test application for GPG plugin testing

#include "CSecurityPluginDLL.h"

#include <iostream.h>
#include <string.h>

#if __dest_os == __mac_os
#include "FullPath.h"
#endif

extern "C"
{
long MulberryPluginEntry(long, void*, long);
typedef long (*MulberryPluginEntryPtr)(long, void*, long);
}

bool Callback(CSecurityPluginDLL::ESecurityPluginCallback type, void* data);
void TestSignData(long refCon);
void TestEncryptData(long refCon);
void TestEncryptSignData(long refCon);
void TestVerifyData(long refCon);
void TestDecryptData(long refCon);
void TestSignFile(long refCon);
void TestEncryptFile(long refCon);
void TestEncryptSignFile(long refCon);
void TestDecryptFile(long refCon);

const char* in_file = "input";
const char* out_file = "output";

#if __dest_os == __mac_os
#define lendl		"\n"
#define lendl_len	1
#elif __dest_os == __win32_os
#define lendl	"\r\n"
#define lendl_len	2
#elif __dest_os == __linux_os || __dest_os == __mac_os_x
#define lendl	"\n"
#define lendl_len	1
#endif

#if __dest_os == __mac_os
static OSErr FindApplicationDirectory(short *theVRefNum, long *theParID);
static OSErr FindApplicationDirectory(short *theVRefNum, long *theParID)
{
	OSErr theErr;
	ProcessSerialNumber thePSN;
	ProcessInfoRec theInfo;
	FSSpec theSpec;
	
	thePSN.highLongOfPSN = 0;
	thePSN.lowLongOfPSN = kCurrentProcess;
	
	theInfo.processInfoLength = sizeof(theInfo);
	theInfo.processName = NULL;
	theInfo.processAppSpec = &theSpec;
	
	/* Find the application FSSpec */
	theErr = GetProcessInformation(&thePSN, &theInfo);
	
	if (theErr == noErr)
	{
		/* Return the folder which contains the application */
		*theVRefNum = theSpec.vRefNum;
		*theParID = theSpec.parID;
	}
	
	return theErr;
}

static OSErr CreateBundleFromFSSpec(FSSpec *theSpec, CFBundleRef *theBundle);
static OSErr CreateBundleFromFSSpec(FSSpec *theSpec, CFBundleRef *theBundle)
{
	OSErr theErr;
	FSRef theRef;
	CFURLRef thePluginDirURL;
	CFURLRef theBundleURL;
	
	/* Turn the FSSpec pointing to the Bundle into a FSRef */
	theErr = FSpMakeFSRef(theSpec, &theRef);
	
	/* Turn the FSRef into a CFURL */
	thePluginDirURL = CFURLCreateFromFSRef(kCFAllocatorSystemDefault, &theRef);
	if (thePluginDirURL == NULL)
		return -1;

	theBundleURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorSystemDefault, thePluginDirURL, CFSTR("GPG.bundle"), false);
	CFRelease(thePluginDirURL);
	
	if (theBundleURL != NULL)
	{
		/* Turn the CFURL into a bundle reference */
		*theBundle = CFBundleCreate(kCFAllocatorSystemDefault, theBundleURL);
		
		CFRelease(theBundleURL);
	}
	
	return theErr;
}

static OSErr LoadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr);
static OSErr LoadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr)
{
	OSErr theErr;
	Boolean isLoaded;
	short theVRefNum;
	long theParID;
	FSSpec theSpec;
	
	/* Start with no bundle */
	*theBundle = NULL;
	
	/* This returns the directory which contains the application */
	theErr = FindApplicationDirectory(&theVRefNum, &theParID);
	
	/* Create the FSSpec pointing to the Bundle */
	if (theErr == noErr)
		theErr = FSMakeFSSpec(theVRefNum, theParID, "\p", &theSpec);
	
	/* Create a bundle reference based on a FSSpec */
	if (theErr == noErr)
		theErr = CreateBundleFromFSSpec(&theSpec, theBundle);
	
	if ((theErr == noErr) && (*theBundle != NULL))
	{
		CFShow(*theBundle);
		isLoaded = CFBundleLoadExecutable(*theBundle);
		
		if (isLoaded)
		{
			/* Lookup the function in the bundle by name */
			*theFnPtr = (MulberryPluginEntryPtr) CFBundleGetFunctionPointerForName(*theBundle, CFSTR("MulberryPluginEntry"));
			//*theFnPtr = (MulberryPluginEntryPtr) CFBundleGetFunctionPointerForName(*theBundle, CFSTR("SayHello"));
		}
	}
	
	return theErr;
}

static OSErr UnloadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr);
static OSErr UnloadFunction(CFBundleRef* theBundle, MulberryPluginEntryPtr* theFnPtr)
{
	/* Call the function if it was found */
	if (*theFnPtr != NULL)
	{
		/* Dispose of the function pointer to the bundled Mach-O routine */
		//DisposePtr((Ptr) *theFnPtr);
	}
			
		CFShow(*theBundle);
	CFBundleUnloadExecutable(*theBundle);
		CFShow(*theBundle);
		
	CFRelease(*theBundle);
	
	return noErr;
}
#endif

static long LoggingCallback(const char* text);
long LoggingCallback(const char* text)
{
	const char* p = text;
	unsigned long l = 0;
	while(*p++) l++;
	return 0;
}

#if __dest_os == __mac_os
static MulberryPluginEntryPtr theFnPtr = NULL;
#else
static MulberryPluginEntryPtr theFnPtr = MulberryPluginEntry;
#endif
int main()
{
	long code;
	long refCon = 0;

	cout << "GPGTest: Application Startup" << endl;
	cout << "GPGTest: Contsruct and Initialise" << endl;
	
#if __dest_os == __mac_os
	CFBundleRef theBundle;
	LoadFunction(&theBundle, &theFnPtr);
#endif

	refCon = (*theFnPtr)(CSecurityPluginDLL::ePluginConstruct, NULL, refCon);
	long err = (*theFnPtr)(CSecurityPluginDLL::ePluginInitialise, NULL, refCon);
	(*theFnPtr)(CPluginDLL::ePluginSetLoggingCallback, (void*) LoggingCallback, refCon);
	(*theFnPtr)(CSecurityPluginDLL::eSecuritySetCallback, (void*) Callback, refCon);

	//TestSignData(refCon);
	//TestEncryptData(refCon);
	//TestEncryptSignData(refCon);
	//TestSignFile(refCon);
	TestEncryptFile(refCon);
	//TestEncryptSignFile(refCon);
	//TestVerifyData(refCon);
	//TestDecryptData(refCon);

	cout << "GPGTest: Terminate and Destruct" << endl;

	err = (*theFnPtr)(CSecurityPluginDLL::ePluginTerminate, NULL, refCon);
	err = (*theFnPtr)(CSecurityPluginDLL::ePluginDestroy, NULL, refCon);

#if __dest_os == __mac_os
	UnloadFunction(&theBundle, &theFnPtr);
#endif

	cout << "GPGTest: Application Shutdown" << endl;
}

bool Callback(CSecurityPluginDLL::ESecurityPluginCallback type, void* data)
{
	switch(type)
	{
	case CSecurityPluginDLL::eCallbackPassphrase:
	{
		CSecurityPluginDLL::SCallbackPassphrase* context = reinterpret_cast<CSecurityPluginDLL::SCallbackPassphrase*>(data);
		::strcpy(context->passphrase, "how much is that mulberry");
		context->chosen = 0;
		return true;
	}
	default:
		return false;
	}
}


void TestSignData(long refCon)
{
	cout << "GPGTest: Sign data" << endl;
	CSecurityPluginDLL::SSignData sign;
	char* out = NULL;
	unsigned long out_len = 0L;
	sign.mInputData = "Content-Type: text/plain" lendl lendl "test" lendl;
	sign.mKey = "daboo@cyrusoft.com";
	sign.mOutputData = &out;
	sign.mOutputDataLength = &out_len;
	sign.mUseMIME = true;

	cout << " DATA: " << sign.mInputData << endl;
	cout << " KEY: " << sign.mKey << endl;

	long err = (*theFnPtr)(CSecurityPluginDLL::eSecuritySignData, &sign, refCon);
	if (!err)
	{
		cout << " FAILED" << endl;
		return;
	}

	cout << " SIGNED: " << endl;
	
	{
		char* p = out;
		long len = out_len;
		while(len--)
			if (*p == '\r')
				*p++ = ' ';
			else
				p++;
	}
	cout.write(out, out_len);
	cout << endl;

	cout << "GPGTest: Dispose Data" << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDisposeData, (void*) *sign.mOutputData, refCon);
}

void TestEncryptData(long refCon)
{
	cout << "GPGTest: Encrypt data" << endl;
	const char* keys[10];
	keys[0] = "daboo@cyrusoft.com";
	keys[1] = NULL;
	CSecurityPluginDLL::SEncryptData encrypt;
	char* out = NULL;
	unsigned long out_len = 0L;
	encrypt.mInputData = "Testing PGP";
	encrypt.mKeys = keys;
	encrypt.mOutputData = &out;
	encrypt.mOutputDataLength = &out_len;
	encrypt.mUseMIME = true;

	cout << " DATA: " << encrypt.mInputData<< endl;
	cout << " KEY: " << *encrypt.mKeys << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityEncryptData, &encrypt, refCon);

	cout << " ENCRYPTED: " << endl;
	
	{
		char* p = out;
		long len = out_len;
		while(len--)
			if (*p == '\r')
				*p++ = ' ';
			else
				p++;
	}
	cout.write(out, out_len);
	cout << endl;

	cout << "GPGTest: Dispose Data" << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDisposeData, (void*) *encrypt.mOutputData, refCon);
}

void TestEncryptSignData(long refCon)
{
	cout << "GPGTest: Encrypt-sign data" << endl;
	const char* keys[10];
	keys[0] = "daboo@cyrusoft.com";
	keys[1] = NULL;
	CSecurityPluginDLL::SEncryptSignData esign;
	char* out = NULL;
	unsigned long out_len = 0L;
	esign.mInputData = "Testing PGP";
	esign.mKeys = keys;
	esign.mSignKey = "daboo@cyrusoft.com";
	esign.mOutputData = &out;
	esign.mOutputDataLength = &out_len;
	esign.mUseMIME = true;

	cout << " DATA: " << esign.mInputData<< endl;
	cout << " KEY: " << *esign.mKeys << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityEncryptSignData, &esign, refCon);

	cout << " ENCRYPTED-SIGNED: " << endl;
	
	{
		char* p = out;
		long len = out_len;
		while(len--)
			if (*p == '\r')
				*p++ = ' ';
			else
				p++;
	}
	cout.write(out, out_len);
	cout << endl;

	cout << "GPGTest: Dispose Data" << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDisposeData, (void*) *esign.mOutputData, refCon);
}

const char* cWholeSignature =
	"-----BEGIN PGP SIGNED MESSAGE-----" lendl
	"Hash: SHA1" lendl
	lendl
	"test" lendl
	lendl
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: PGP for Personal Privacy 5.5" lendl
	lendl
	"iQA/AwUBN4ItnCK4BGrbmMvFEQINhQCg51/6qAOfaQygGY1DYzXVFGm8ZK4AoOw/" lendl
	"CM3QrMdHsHY0AGQ338JBUROI" lendl
	"=wdX/" lendl
	"-----END PGP SIGNATURE-----" lendl;

const char* cSeparateSignature =
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: PGP for Personal Privacy 5.5" lendl
	lendl
	"iQA/AwUBN4ItnCK4BGrbmMvFEQINhQCg51/6qAOfaQygGY1DYzXVFGm8ZK4AoOw/" lendl
	"CM3QrMdHsHY0AGQ338JBUROI" lendl
	"=wdX/" lendl
	"-----END PGP SIGNATURE-----" lendl;

const char* cSeparateData =
	"test" lendl;

const char* cOpenWhole1 =
	"From daboo@cyrusoft.com Tue Jul 06 12:36:07 1999 -0400" lendl
	"Date: Tue, 06 Jul 1999 12:36:07 -0400" lendl
	"From: Cyrus Daboo <daboo@cyrusoft.com>" lendl
	"To: Cyrus Daboo <daboo@cyrusoft.com>" lendl
	"Subject: test" lendl
	"Message-ID: <497881.3140253366@sardis.cyrusoft.com>" lendl
	"Originator-Info: login-id=daboo; server=imap.cyrusoft.com:431" lendl
	"X-Mailer: Mulberry (MacOS) [2.0.0a3, s/n S1-000001]" lendl
	"MIME-Version: 1.0" lendl
	"Content-Type: Multipart/signed; micalg=pgp-sha1;" lendl
	" protocol=\"application/pgp-signature\";" lendl
	" Boundary=\"==========00503649==========\"" lendl
	lendl
	"--==========00503649==========" lendl
	"Content-Type: text/plain; charset=us-ascii" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	"Content-Disposition: inline" lendl
	lendl
	"This test " lendl
	" " lendl
	"--==========00503649==========" lendl
	"Content-Type: application/pgp-signature" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	lendl
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: Mulberry PGP Plugin v1.4b2" lendl
	"Comment: processed by Mulberry PGP Plugin" lendl
	lendl
	"iQA/AwUBN4IwdyK4BGrbmMvFEQJooQCfTCX+nxc2IqhWp/uPhRo7G7i0jhoAnAmT" lendl
	"vcsUkQVu1SZEoulB7c8h9f/R" lendl
	"=dEzG" lendl
	"-----END PGP SIGNATURE-----" lendl
	lendl
	"--==========00503649==========--" lendl;

const char* cOpenSignature1 =
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: Mulberry PGP Plugin v1.4b2" lendl
	"Comment: processed by Mulberry PGP Plugin" lendl
	lendl
	"iQA/AwUBN4IwdyK4BGrbmMvFEQJooQCfTCX+nxc2IqhWp/uPhRo7G7i0jhoAnAmT" lendl
	"vcsUkQVu1SZEoulB7c8h9f/R" lendl
	"=dEzG" lendl
	"-----END PGP SIGNATURE-----" lendl;

const char* cOpenData1 =
	"Content-Type: text/plain; charset=us-ascii" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	"Content-Disposition: inline" lendl
	lendl
	"This test" lendl;

const char* cOpenWhole2 =
	"From daboo@cyrusoft.com Tue Jul 06 12:36:15 1999 -0400" lendl
	"Date: Tue, 06 Jul 1999 12:36:15 -0400" lendl
	"From: Cyrus Daboo <daboo@cyrusoft.com>" lendl
	"To: Cyrus Daboo <daboo@cyrusoft.com>" lendl
	"Subject: test" lendl
	"Message-ID: <498381.3140253375@sardis.cyrusoft.com>" lendl
	"Originator-Info: login-id=daboo; server=imap.cyrusoft.com:431" lendl
	"X-Mailer: Mulberry (MacOS) [2.0.0a3, s/n S1-000001]" lendl
	"MIME-Version: 1.0" lendl
	"Content-Type: multipart/signed; micalg=pgp-sha1;" lendl
	" protocol=\"application/pgp-signature\";" lendl
	" boundary=\"==========00508501==========\"" lendl
	lendl
	"--==========00508501==========" lendl
	"Content-Type: text/plain; charset=us-ascii" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	"Content-Disposition: inline" lendl
	lendl
	"This test " lendl
	lendl
	"--==========00508501==========" lendl
	"Content-Type: application/pgp-signature" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	lendl
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: Mulberry PGP Plugin v1.4b2" lendl
	"Comment: processed by Mulberry PGP Plugin" lendl
	lendl
	"iQA/AwUBN4IwfyK4BGrbmMvFEQIboACg5yZtGb7LOkgwS8+l/tgNkVq/28IAnR/g" lendl
	"0a4z5sRexybzwCFOhDvd94lr" lendl
	"=+JO5" lendl
	"-----END PGP SIGNATURE-----" lendl
	lendl
	"--==========00508501==========--" lendl;

const char* cOpenSignature2 =
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: Mulberry PGP Plugin v1.4b2" lendl
	"Comment: processed by Mulberry PGP Plugin" lendl
	lendl
	"iQA/AwUBN4IwfyK4BGrbmMvFEQIboACg5yZtGb7LOkgwS8+l/tgNkVq/28IAnR/g" lendl
	"0a4z5sRexybzwCFOhDvd94lr" lendl
	"=+JO5" lendl
	"-----END PGP SIGNATURE-----" lendl;

const char* cOpenData2 =
	"Content-Type: text/plain; charset=us-ascii" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	"Content-Disposition: inline" lendl
	lendl
	"This test " lendl;

const char* cOpenWhole3 =
	"From steve@execmail.com Tue Mar 02 22:58:55 1999 -0500" lendl
	"Return-Path: <steve@execmail.com>" lendl
	"Received: from demo.esys.ca (demo.esys.ca [207.167.22.130])" lendl
	"	by darius.cyrusoft.com (8.8.5/8.8.5) with ESMTP id WAA08886" lendl
	"	for <daboo@cyrusoft.com>; Tue, 2 Mar 1999 22:58:53 -0500 (EST)" lendl
	"Received: from galileo.esys.ca (dhcp198-58.esys.ca [198.161.92.58])" lendl
	"	by demo.esys.ca (2.0.4/SMS 2.0.4-beta-5) with ESMTP id MAA11654" lendl
	"	for <daboo@cyrusoft.com>; Wed, 3 Mar 1999 12:08:20 -0700" lendl
	"From: Steve Hole <steve@execmail.com>" lendl
	"Date: Tue, 2 Mar 1999 21:01:23 -0700" lendl
	"To: Cyrus Daboo <daboo@cyrusoft.com>" lendl
	"Subject: Signed only message" lendl
	"Message-ID: <EXECMAIL.990302210123.P@galileo.execmail.com>" lendl
	"Priority: NORMAL" lendl
	"X-Mailer: Execmail for Win32 Version 5.0 pc5 Build (37)" lendl
	"MIME-Version: 1.0" lendl
	"Content-Type: Multipart/signed; boundary=\"Part9903022101.Q\"; protocol=\"application/pgp-signature\"; micalg=\"pgp-sha1\"" lendl
	lendl
	lendl
	"--Part9903022101.Q" lendl
	"Content-Type: Text/Plain; charset=\"us-ascii\"; name=\"ipm.txt\"" lendl
	"Content-Disposition: inline; filename=\"ipm.txt\"" lendl
	lendl
	"Here is a singed only message." lendl
	lendl
	"---  " lendl
	"Steve Hole                           " lendl
	"Execmail Inc." lendl
	"Mailto:Steve.Hole@execmail.com " lendl
	"Phone: 780-424-4922" lendl
	lendl
	lendl
	"--Part9903022101.Q" lendl
	"Content-Type: Application/pgp-signature" lendl
	lendl
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: PGPsdk version 1.5.2 (C) 1997-1998 Network Associates, Inc. and its affiliated companies." lendl
	lendl
	"iQA/AwUBNty0GNi5Jj9Fn5KMEQI8XACgnzgWI71xJRCYDj0UkLHjgGTUuHcAnAmj" lendl
	"rcRM6c9uFQmxTobe6rHUN8nE" lendl
	"=9fS1" lendl
	"-----END PGP SIGNATURE-----" lendl
	lendl
	"--Part9903022101.Q--" lendl;

const char* cOpenSignature3 =
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: PGPsdk version 1.5.2 (C) 1997-1998 Network Associates, Inc. and its affiliated companies." lendl
	lendl
	"iQA/AwUBNty0GNi5Jj9Fn5KMEQI8XACgnzgWI71xJRCYDj0UkLHjgGTUuHcAnAmj" lendl
	"rcRM6c9uFQmxTobe6rHUN8nE" lendl
	"=9fS1" lendl
	"-----END PGP SIGNATURE-----" lendl;

const char* cOpenData3 =
	"Content-Type: Text/Plain; charset=\"us-ascii\"; name=\"ipm.txt\"" lendl
	"Content-Disposition: inline; filename=\"ipm.txt\"" lendl
	lendl
	"Here is a singed only message." lendl
	lendl
	"---  " lendl
	"Steve Hole                           " lendl
	"Execmail Inc." lendl
	"Mailto:Steve.Hole@execmail.com " lendl
	"Phone: 780-424-4922" lendl
	lendl;

const char* cOpenData4 = "Content-Type: text/plain; charset=us-ascii\rContent-Transfer-Encoding: 7bit\rContent-Disposition: inline\r\rThis test\r";
const char* cOpenSignature4 = "-----BEGIN PGP SIGNATURE-----\rVersion: Mulberry PGP Plugin v1.4b2\rComment: processed by Mulberry PGP Plugin\r\riQA/AwUBN4IwdyK4BGrbmMvFEQJooQCfTCX+nxc2IqhWp/uPhRo7G7i0jhoAnAmT\rvcsUkQVu1SZEoulB7c8h9f/R\r=dEzG\r-----END PGP SIGNATURE-----\r";

const char* cOpenData5 = 
	"Content-Type: multipart/mixed; boundary=\"==========01106357==========\"" lendl
	lendl
	"--==========01106357==========" lendl
	"Content-Type: text/plain; charset=us-ascii" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	"Content-Disposition: inline" lendl
	lendl
	lendl
	lendl
	"--" lendl
	"Cyrus" lendl
	"--==========01106357==========" lendl
	"Content-Type: application/octet-stream; name=Unnamed part #1_2" lendl
	"Content-Transfer-Encoding: base64" lendl
	"Content-Disposition: attachment; filename=Unnamed part #1_2; size=334" lendl
	lendl
	"LS0tLS1CRUdJTiBQR1AgUFVCTElDIEtFWSBCTE9DSy0tLS0tDVZlcnNpb246IDIuNi4yDQ1tUUNO" lendl
	"QXkvR1N1MEFBQUVFQU1QQUlTZkY4VHBtdW9WcGdsN0lEUmh5L3RSN3BsbVJCeFhxTzVjR21RbVhx" lendl
	"RlhmDWF6eC8zemo5YXdiNDVWdTdQMWNHQjVWQzJRaWNCMUhBSEdKWGdQL3BQYWlaYTVNaW94a1FC" lendl
	"ZnZlelRpNDRyc0UNS3Bnc1piVUR4R1FZWjJvVnMzYkV0RHZUa05idzlvMkJoU0xobUh6RWZxb0Rm" lendl
	"aGNDZzVFbGtNa01lempOQUFVUg10QjVVYUc5dFlYTWdWMkZuYm1WeUlEeG9iMjUxY3l0QVkyMTFM" lendl
	"bVZrZFQ0PQ09cFdyQQ0tLS0tLUVORCBQR1AgUFVCTElDIEtFWSBCTE9DSy0tLS0tDQ==" lendl
	lendl
	"--==========01106357==========--" lendl;
const char* cOpenSignature5 = 
	"Content-Type: application/pgp-signature; name=\"temp00000009.c\"" lendl
	"Content-Transfer-Encoding: 7bit" lendl
	"Content-Disposition: attachment; filename=\"temp00000009.c\"; size=233" lendl
	lendl
	"-----BEGIN PGP SIGNATURE-----" lendl
	"Version: Mulberry PGP Plugin v1.4b2" lendl
	"Comment: processed by Mulberry PGP Plugin" lendl
	lendl
	"iQA/AwUBN4JXuyK4BGrbmMvFEQKu7wCfR6QROxKTbr6zO/sAl8NKof9C55wAoLs6" lendl
	"xcP6nZWSyZNn6hVc5pWVeu5T" lendl
	"=OVtJ" lendl
	"-----END PGP SIGNATURE-----" lendl;

const char* cEncrypted =
	"-----BEGIN PGP MESSAGE-----" lendl
	"Version: PGP for Personal Privacy 5.5" lendl
	lendl
	"qANQR1DBwU4DK66LjGUXow4QB/4lbeEnioNpMj2gVkzC+uej3IZ3TVMEpSIJfy7I" lendl
	"16jZhKYDYP8nLFYnMpfB0C7HRY/iNpiWRdX+++bTvwXDEWL1rt4/+cdOjxZgQnAT" lendl
	"B/TXsNgTmP2JhsYf9137AjlPXzz3ZKADyI6wwvI2a9x+6dNS3kspfeKaPvA5/lWF" lendl
	"hmi5t0Eo83g+7NAIjDG09mC8H7oKW1O7j9LA9CRum4glkrtej9YuZzYSIFPIJX/L" lendl
	"7IG5ccHkcF4hc0EOSLEDN4GLl8wbthWb6qgvj8dCg4ygh/Am6FxIkLvGlGbKjH7Q" lendl
	"bE3tqXXC3ej0o3GH/wa6Mw7pjCKuTkFNDNbD+C4L2UItK2WUB/40Q8s5X06h4IZI" lendl
	"quDirvsUaBdAmPgcYCYDG66fSJI/3C7ezEfarUHRroGxMUOd3Jg1A/yYjSHLq1Kj" lendl
	"AzU5dnOI3TjFCGw5JOo0nrzFfY3hxh8gGRq+hieIITk9/SIRJvs8F2cKPS4Tg4mV" lendl
	"EryoOvk3jA2z2KQCa7ipZGjbZawH5UK7yEWPc8pPzIAOjJw/Am6SKZzGlNamJRQv" lendl
	"DSCP6Vm4SXkQudDJmNBZLI9bSy+tLL66buVG0KN+tdLTCL41UuBV9tYlI27I8drc" lendl
	"Gj3/i2YAeT+w68GFHjWKY1OTaXRxKoC1J/3aXH8fCLL+DN2gOE53086ruCYbtTds" lendl
	"oVM63TQCySNfMnnLSi4kgorUh6RQH4Wgf6H30XLjFkwSRXEDhbHKpbM4nw==" lendl
	"=k3Ig" lendl
	"-----END PGP MESSAGE-----" lendl;

void TestVerifyData(long refCon)
{
	cout << "GPGTest: Verify data" << endl;
	CSecurityPluginDLL::SDecryptVerifyData verify;
	char* out = NULL;
	unsigned long out_len = 0L;
	bool result = false;
	bool did_sig = false;
	verify.mInputData = cOpenData5;
	verify.mInputSignature = cOpenSignature5;
	verify.mOutputData = &out;
	verify.mOutputDataLength = &out_len;
	verify.mSuccess = &result;
	verify.mDidSig = &did_sig;

	cout << " DATA:" << endl << verify.mInputData << endl;
	if (verify.mInputSignature)
		cout << " SIGNATURE:" << endl << verify.mInputSignature << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDecryptVerifyData, &verify, refCon);

	if (result)
	{
		cout << " VERIFIED:" << endl;
		
		{
			char* p = out;
			long len = out_len;
			while(len--)
				if (*p == '\r')
					*p++ = '\n';
				else
					p++;
		}
		cout.write(out, out_len);
		cout << endl;
	}
	else
		cout << " VERIFY FAILED" << endl;

	cout << "GPGTest: Dispose Data" << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDisposeData, (void*) *verify.mOutputData, refCon);
}

void TestDecryptData(long refCon)
{
	cout << "GPGTest: Decrypt data" << endl;
	CSecurityPluginDLL::SDecryptVerifyData decrypt;
	char* out = NULL;
	unsigned long out_len = 0L;
	bool result = false;
	bool did_sig = false;
	decrypt.mInputData = cEncrypted;
	decrypt.mInputSignature = NULL;
	decrypt.mOutputData = &out;
	decrypt.mOutputDataLength = &out_len;
	decrypt.mSuccess = &result;
	decrypt.mDidSig = &did_sig;

	cout << " DATA:" << endl << decrypt.mInputData << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDecryptVerifyData, &decrypt, refCon);

	if (result)
	{
		cout << " DECRYPTED:" << endl;
		
		{
			char* p = out;
			long len = out_len;
			while(len--)
				if (*p == '\r')
					*p++ = '\n';
				else
					p++;
		}
		cout.write(out, out_len);
		cout << endl;
	}
	else
		cout << " DECRYPT FAILED" << endl;

	cout << "GPGTest: Dispose Data" << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityDisposeData, (void*) *decrypt.mOutputData, refCon);
}

void TestSignFile(long refCon)
{
	const char* in = in_file;
	const char* out = out_file;

#if __dest_os == __mac_os
	FSSpec _in_spec;
	FSSpec* in_spec = &_in_spec;
	OSErr err = ::FSpLocationFromFullPath(::strlen(in_file), in_file, in_spec);
	FSSpec _out_spec;
	FSSpec* out_spec = &_out_spec;
	err = ::FSpLocationFromFullPath(::strlen(out_file), out_file, out_spec);
#else
	fspec in_spec = in;
	fspec out_spec = out;
#endif

	cout << "GPGTest: Sign file" << endl;
	CSecurityPluginDLL::SSignFile sign;
	sign.mInputFile = in_spec;
	sign.mKey = "daboo@cyrusoft.com";
	sign.mOutputFile = out_spec;
	sign.mUseMIME = true;

	cout << " FILE: " << in_file << endl;
	cout << " KEY: " << sign.mKey << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecuritySignFile, &sign, refCon);

	cout << " SIGNED: " << out_file  << endl;
}

void TestEncryptFile(long refCon)
{
	const char* in = in_file;
	const char* out = out_file;

#if __dest_os == __mac_os
	FSSpec _in_spec;
	FSSpec* in_spec = &_in_spec;
	OSErr err = ::FSpLocationFromFullPath(::strlen(in_file), in_file, in_spec);
	FSSpec _out_spec;
	FSSpec* out_spec = &_out_spec;
	err = ::FSpLocationFromFullPath(::strlen(out_file), out_file, out_spec);
#else
	fspec in_spec = in;
	fspec out_spec = out;
#endif

	cout << "GPGTest: Encrypt file" << endl;
	const char* keys[10];
	keys[0] = "daboo@cyrusoft.com";
	keys[1] = NULL;
	CSecurityPluginDLL::SEncryptFile encrypt;
	encrypt.mInputFile = in_spec;
	encrypt.mKeys = keys;
	encrypt.mOutputFile = out_spec;
	encrypt.mUseMIME = true;

	cout << " DATA: " << in_file << endl;
	cout << " KEY: " << *encrypt.mKeys << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityEncryptFile, &encrypt, refCon);

	cout << " ENCRYPTED: " << out_file << endl;
}

void TestEncryptSignFile(long refCon)
{
	const char* in = in_file;
	const char* out = out_file;

#if __dest_os == __mac_os
	FSSpec _in_spec;
	FSSpec* in_spec = &_in_spec;
	OSErr err = ::FSpLocationFromFullPath(::strlen(in_file), in_file, in_spec);
	FSSpec _out_spec;
	FSSpec* out_spec = &_out_spec;
	err = ::FSpLocationFromFullPath(::strlen(out_file), out_file, out_spec);
#else
	fspec in_spec = in;
	fspec out_spec = out;
#endif

	cout << "GPGTest: Encrypt-sign file" << endl;
	const char* keys[10];
	keys[0] = "daboo@cyrusoft.com";
	keys[1] = NULL;
	CSecurityPluginDLL::SEncryptSignFile esign;
	esign.mInputFile = in_spec;
	esign.mKeys = keys;
	esign.mSignKey = "daboo@cyrusoft.com";
	esign.mOutputFile = out_spec;
	esign.mUseMIME = true;

	cout << " DATA: " << in_file << endl;
	cout << " KEY: " << *esign.mKeys << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecurityEncryptSignFile, &esign, refCon);

	cout << " ENCRYPTED-SIGNED: " << out_file << endl;
}

void TestDecryptFile(long refCon)
{
#if 0
#if __dest_os == __mac_os
	FSSpec in;
	FSSpec out;
	PrepareFiles(&in, &out);

	cout << "GPGTest: Decrypt file" << endl;
	CSecurityPluginDLL::SSignFile sign;
	sign.mInputFile = &in;
	sign.mKey = "daboo@cyrusoft.com";
	sign.mOutputFile = &out;
	sign.mUseMIME = true;

	cout << " FILE: " << in_file << endl;
	cout << " KEY: " << sign.mKey << endl;

	(*theFnPtr)(CSecurityPluginDLL::eSecuritySignFile, &sign, refCon);

	cout << " SIGNED: " << out_file  << endl;
#endif
#endif
}
