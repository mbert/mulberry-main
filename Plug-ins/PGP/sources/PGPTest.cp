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

// PGPTest.cp

// Test application for PGP plugin testing

#include "CSecurityPluginDLL.h"

#include <iostream.h>

#if __dest_os == __mac_os
#include "FullPath.h"
#include <SIOUX.h>
#endif

extern "C"
{
long MulberryPluginEntry(long, void*, long);
}

void TestSignData(long refCon);
void TestEncryptData(long refCon);
void TestEncryptSignData(long refCon);
void TestVerifyData(long refCon);
void TestDecryptData(long refCon);
void TestSignFile(long refCon);
void TestEncryptFile(long refCon);
void TestEncryptSignFile(long refCon);

#if __dest_os == __mac_os
void PrepareFiles(FSSpec* in, FSSpec* out);
#endif

const char* in_file = ":Input File";
const char* out_file = ":Output File";

int main()
{
	SIOUXSettings.asktosaveonclose = 0;

	long code;
	long refCon = 0;

	cout << "PGPTest: Application Startup" << endl;
	cout << "PGPTest: Contsruct and Initialise" << endl;
	
	refCon = MulberryPluginEntry(CSecurityPluginDLL::ePluginConstruct, NULL, refCon);
	long err = MulberryPluginEntry(CSecurityPluginDLL::ePluginCanRun, NULL, refCon);
	err = MulberryPluginEntry(CSecurityPluginDLL::ePluginInitialise, NULL, refCon);

	TestSignData(refCon);
	//TestEncryptData(refCon);
	//TestEncryptSignData(refCon);
	//TestSignFile(refCon);
	//TestEncryptFile(refCon);
	//TestEncryptSignFile(refCon);
	//TestVerifyData(refCon);
	//TestDecryptData(refCon);

	cout << "PGPTest: Terminate and Destruct" << endl;

	err = MulberryPluginEntry(CSecurityPluginDLL::ePluginTerminate, NULL, refCon);
	err = MulberryPluginEntry(CSecurityPluginDLL::ePluginDestroy, NULL, refCon);

	cout << "PGPTest: Application Shutdown" << endl;
}

void TestSignData(long refCon)
{
	cout << "PGPTest: Sign data" << endl;
	CSecurityPluginDLL::SSignData sign;
	char* out = NULL;
	unsigned long out_len = 0L;
	sign.mInputData = "Content-Type: text/plain\n\ntest\n";
	sign.mKey = "daboo@cyrusoft.com";
	sign.mOutputData = &out;
	sign.mOutputDataLength = &out_len;
	sign.mUseMIME = true;

	cout << " DATA: " << sign.mInputData << endl;
	cout << " KEY: " << sign.mKey << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecuritySignData, &sign, refCon);

	cout << " SIGNED: " << endl;
	
	{
		char* p = out;
		long len = out_len;
		while(len--)
			if (*p == '\r')
				*p++ = '\r';
			else
				p++;
	}
	cout.write(out, out_len);
	cout << endl;

	cout << "PGPTest: Dispose Data" << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDisposeData, (void*) sign.mOutputData, refCon);
}

void TestEncryptData(long refCon)
{
	cout << "PGPTest: Encrypt data" << endl;
	char* keys[10];
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

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityEncryptData, &encrypt, refCon);

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

	cout << "PGPTest: Dispose Data" << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDisposeData, (void*) encrypt.mOutputData, refCon);
}

void TestEncryptSignData(long refCon)
{
	cout << "PGPTest: Encrypt-sign data" << endl;
	char* keys[10];
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

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityEncryptSignData, &esign, refCon);

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

	cout << "PGPTest: Dispose Data" << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDisposeData, (void*) esign.mOutputData, refCon);
}

const char* cWholeSignature =
	"-----BEGIN PGP SIGNED MESSAGE-----\n"
	"Hash: SHA1\n"
	"\n"
	"test\n"
	"\n"
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: PGP for Personal Privacy 5.5\n"
	"\n"
	"iQA/AwUBN4ItnCK4BGrbmMvFEQINhQCg51/6qAOfaQygGY1DYzXVFGm8ZK4AoOw/\n"
	"CM3QrMdHsHY0AGQ338JBUROI\n"
	"=wdX/\n"
	"-----END PGP SIGNATURE-----\n";

const char* cSeparateSignature =
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: PGP for Personal Privacy 5.5\n"
	"\n"
	"iQA/AwUBN4ItnCK4BGrbmMvFEQINhQCg51/6qAOfaQygGY1DYzXVFGm8ZK4AoOw/\n"
	"CM3QrMdHsHY0AGQ338JBUROI\n"
	"=wdX/\n"
	"-----END PGP SIGNATURE-----\n";

const char* cSeparateData =
	"test\n";

const char* cOpenWhole1 =
	"From daboo@cyrusoft.com Tue Jul 06 12:36:07 1999 -0400\n"
	"Date: Tue, 06 Jul 1999 12:36:07 -0400\n"
	"From: Cyrus Daboo <daboo@cyrusoft.com>\n"
	"To: Cyrus Daboo <daboo@cyrusoft.com>\n"
	"Subject: test\n"
	"Message-ID: <497881.3140253366@sardis.cyrusoft.com>\n"
	"Originator-Info: login-id=daboo; server=imap.cyrusoft.com:431\n"
	"X-Mailer: Mulberry (MacOS) [2.0.0a3, s/n S1-000001]\n"
	"MIME-Version: 1.0\n"
	"Content-Type: Multipart/signed; micalg=pgp-sha1;\n"
	" protocol=\"application/pgp-signature\";\n"
	" Boundary=\"==========00503649==========\"\n"
	"\n"
	"--==========00503649==========\n"
	"Content-Type: text/plain; charset=us-ascii\n"
	"Content-Transfer-Encoding: 7bit\n"
	"Content-Disposition: inline\n"
	"\n"
	"This test \n"
	" \n"
	"--==========00503649==========\n"
	"Content-Type: application/pgp-signature\n"
	"Content-Transfer-Encoding: 7bit\n"
	"\n"
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: Mulberry PGP Plugin v1.4b2\n"
	"Comment: processed by Mulberry PGP Plugin\n"
	"\n"
	"iQA/AwUBN4IwdyK4BGrbmMvFEQJooQCfTCX+nxc2IqhWp/uPhRo7G7i0jhoAnAmT\n"
	"vcsUkQVu1SZEoulB7c8h9f/R\n"
	"=dEzG\n"
	"-----END PGP SIGNATURE-----\n"
	"\n"
	"--==========00503649==========--\n";

const char* cOpenSignature1 =
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: Mulberry PGP Plugin v1.4b2\n"
	"Comment: processed by Mulberry PGP Plugin\n"
	"\n"
	"iQA/AwUBN4IwdyK4BGrbmMvFEQJooQCfTCX+nxc2IqhWp/uPhRo7G7i0jhoAnAmT\n"
	"vcsUkQVu1SZEoulB7c8h9f/R\n"
	"=dEzG\n"
	"-----END PGP SIGNATURE-----\n";

const char* cOpenData1 =
	"Content-Type: text/plain; charset=us-ascii\n"
	"Content-Transfer-Encoding: 7bit\n"
	"Content-Disposition: inline\n"
	"\n"
	"This test\n";

const char* cOpenWhole2 =
	"From daboo@cyrusoft.com Tue Jul 06 12:36:15 1999 -0400\n"
	"Date: Tue, 06 Jul 1999 12:36:15 -0400\n"
	"From: Cyrus Daboo <daboo@cyrusoft.com>\n"
	"To: Cyrus Daboo <daboo@cyrusoft.com>\n"
	"Subject: test\n"
	"Message-ID: <498381.3140253375@sardis.cyrusoft.com>\n"
	"Originator-Info: login-id=daboo; server=imap.cyrusoft.com:431\n"
	"X-Mailer: Mulberry (MacOS) [2.0.0a3, s/n S1-000001]\n"
	"MIME-Version: 1.0\n"
	"Content-Type: multipart/signed; micalg=pgp-sha1;\n"
	" protocol=\"application/pgp-signature\";\n"
	" boundary=\"==========00508501==========\"\n"
	"\n"
	"--==========00508501==========\n"
	"Content-Type: text/plain; charset=us-ascii\n"
	"Content-Transfer-Encoding: 7bit\n"
	"Content-Disposition: inline\n"
	"\n"
	"This test \n"
	"\n"
	"--==========00508501==========\n"
	"Content-Type: application/pgp-signature\n"
	"Content-Transfer-Encoding: 7bit\n"
	"\n"
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: Mulberry PGP Plugin v1.4b2\n"
	"Comment: processed by Mulberry PGP Plugin\n"
	"\n"
	"iQA/AwUBN4IwfyK4BGrbmMvFEQIboACg5yZtGb7LOkgwS8+l/tgNkVq/28IAnR/g\n"
	"0a4z5sRexybzwCFOhDvd94lr\n"
	"=+JO5\n"
	"-----END PGP SIGNATURE-----\n"
	"\n"
	"--==========00508501==========--\n";

const char* cOpenSignature2 =
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: Mulberry PGP Plugin v1.4b2\n"
	"Comment: processed by Mulberry PGP Plugin\n"
	"\n"
	"iQA/AwUBN4IwfyK4BGrbmMvFEQIboACg5yZtGb7LOkgwS8+l/tgNkVq/28IAnR/g\n"
	"0a4z5sRexybzwCFOhDvd94lr\n"
	"=+JO5\n"
	"-----END PGP SIGNATURE-----\n";

const char* cOpenData2 =
	"Content-Type: text/plain; charset=us-ascii\n"
	"Content-Transfer-Encoding: 7bit\n"
	"Content-Disposition: inline\n"
	"\n"
	"This test \n";

const char* cOpenWhole3 =
	"From steve@execmail.com Tue Mar 02 22:58:55 1999 -0500\n"
	"Return-Path: <steve@execmail.com>\n"
	"Received: from demo.esys.ca (demo.esys.ca [207.167.22.130])\n"
	"	by darius.cyrusoft.com (8.8.5/8.8.5) with ESMTP id WAA08886\n"
	"	for <daboo@cyrusoft.com>; Tue, 2 Mar 1999 22:58:53 -0500 (EST)\n"
	"Received: from galileo.esys.ca (dhcp198-58.esys.ca [198.161.92.58])\n"
	"	by demo.esys.ca (2.0.4/SMS 2.0.4-beta-5) with ESMTP id MAA11654\n"
	"	for <daboo@cyrusoft.com>; Wed, 3 Mar 1999 12:08:20 -0700\n"
	"From: Steve Hole <steve@execmail.com>\n"
	"Date: Tue, 2 Mar 1999 21:01:23 -0700\n"
	"To: Cyrus Daboo <daboo@cyrusoft.com>\n"
	"Subject: Signed only message\n"
	"Message-ID: <EXECMAIL.990302210123.P@galileo.execmail.com>\n"
	"Priority: NORMAL\n"
	"X-Mailer: Execmail for Win32 Version 5.0 pc5 Build (37)\n"
	"MIME-Version: 1.0\n"
	"Content-Type: Multipart/signed; boundary=\"Part9903022101.Q\"; protocol=\"application/pgp-signature\"; micalg=\"pgp-sha1\"\n"
	"\n"
	"\n"
	"--Part9903022101.Q\n"
	"Content-Type: Text/Plain; charset=\"us-ascii\"; name=\"ipm.txt\"\n"
	"Content-Disposition: inline; filename=\"ipm.txt\"\n"
	"\n"
	"Here is a singed only message.\n"
	"\n"
	"---  \n"
	"Steve Hole                           \n"
	"Execmail Inc.\n"
	"Mailto:Steve.Hole@execmail.com \n"
	"Phone: 780-424-4922\n"
	"\n"
	"\n"
	"--Part9903022101.Q\n"
	"Content-Type: Application/pgp-signature\n"
	"\n"
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: PGPsdk version 1.5.2 (C) 1997-1998 Network Associates, Inc. and its affiliated companies.\n"
	"\n"
	"iQA/AwUBNty0GNi5Jj9Fn5KMEQI8XACgnzgWI71xJRCYDj0UkLHjgGTUuHcAnAmj\n"
	"rcRM6c9uFQmxTobe6rHUN8nE\n"
	"=9fS1\n"
	"-----END PGP SIGNATURE-----\n"
	"\n"
	"--Part9903022101.Q--\n";

const char* cOpenSignature3 =
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: PGPsdk version 1.5.2 (C) 1997-1998 Network Associates, Inc. and its affiliated companies.\n"
	"\n"
	"iQA/AwUBNty0GNi5Jj9Fn5KMEQI8XACgnzgWI71xJRCYDj0UkLHjgGTUuHcAnAmj\n"
	"rcRM6c9uFQmxTobe6rHUN8nE\n"
	"=9fS1\n"
	"-----END PGP SIGNATURE-----\n";

const char* cOpenData3 =
	"Content-Type: Text/Plain; charset=\"us-ascii\"; name=\"ipm.txt\"\n"
	"Content-Disposition: inline; filename=\"ipm.txt\"\n"
	"\n"
	"Here is a singed only message.\n"
	"\n"
	"---  \n"
	"Steve Hole                           \n"
	"Execmail Inc.\n"
	"Mailto:Steve.Hole@execmail.com \n"
	"Phone: 780-424-4922\n"
	"\n";

const char* cOpenData4 = "Content-Type: text/plain; charset=us-ascii\rContent-Transfer-Encoding: 7bit\rContent-Disposition: inline\r\rThis test\r";
const char* cOpenSignature4 = "-----BEGIN PGP SIGNATURE-----\rVersion: Mulberry PGP Plugin v1.4b2\rComment: processed by Mulberry PGP Plugin\r\riQA/AwUBN4IwdyK4BGrbmMvFEQJooQCfTCX+nxc2IqhWp/uPhRo7G7i0jhoAnAmT\rvcsUkQVu1SZEoulB7c8h9f/R\r=dEzG\r-----END PGP SIGNATURE-----\r";

const char* cOpenData5 = 
	"Content-Type: multipart/mixed; boundary=\"==========01106357==========\"\n"
	"\n"
	"--==========01106357==========\n"
	"Content-Type: text/plain; charset=us-ascii\n"
	"Content-Transfer-Encoding: 7bit\n"
	"Content-Disposition: inline\n"
	"\n"
	"\n"
	"\n"
	"--\n"
	"Cyrus\n"
	"--==========01106357==========\n"
	"Content-Type: application/octet-stream; name=Unnamed part #1_2\n"
	"Content-Transfer-Encoding: base64\n"
	"Content-Disposition: attachment; filename=Unnamed part #1_2; size=334\n"
	"\n"
	"LS0tLS1CRUdJTiBQR1AgUFVCTElDIEtFWSBCTE9DSy0tLS0tDVZlcnNpb246IDIuNi4yDQ1tUUNO\n"
	"QXkvR1N1MEFBQUVFQU1QQUlTZkY4VHBtdW9WcGdsN0lEUmh5L3RSN3BsbVJCeFhxTzVjR21RbVhx\n"
	"RlhmDWF6eC8zemo5YXdiNDVWdTdQMWNHQjVWQzJRaWNCMUhBSEdKWGdQL3BQYWlaYTVNaW94a1FC\n"
	"ZnZlelRpNDRyc0UNS3Bnc1piVUR4R1FZWjJvVnMzYkV0RHZUa05idzlvMkJoU0xobUh6RWZxb0Rm\n"
	"aGNDZzVFbGtNa01lempOQUFVUg10QjVVYUc5dFlYTWdWMkZuYm1WeUlEeG9iMjUxY3l0QVkyMTFM\n"
	"bVZrZFQ0PQ09cFdyQQ0tLS0tLUVORCBQR1AgUFVCTElDIEtFWSBCTE9DSy0tLS0tDQ==\n"
	"\n"
	"--==========01106357==========--\n";
const char* cOpenSignature5 = 
	"Content-Type: application/pgp-signature; name=\"temp00000009.c\"\n"
	"Content-Transfer-Encoding: 7bit\n"
	"Content-Disposition: attachment; filename=\"temp00000009.c\"; size=233\n"
	"\n"
	"-----BEGIN PGP SIGNATURE-----\n"
	"Version: Mulberry PGP Plugin v1.4b2\n"
	"Comment: processed by Mulberry PGP Plugin\n"
	"\n"
	"iQA/AwUBN4JXuyK4BGrbmMvFEQKu7wCfR6QROxKTbr6zO/sAl8NKof9C55wAoLs6\n"
	"xcP6nZWSyZNn6hVc5pWVeu5T\n"
	"=OVtJ\n"
	"-----END PGP SIGNATURE-----\n";

const char* cEncrypted =
	"-----BEGIN PGP MESSAGE-----\n"
	"Version: PGP for Personal Privacy 5.5\n"
	"\n"
	"qANQR1DBwU4DK66LjGUXow4QB/4lbeEnioNpMj2gVkzC+uej3IZ3TVMEpSIJfy7I\n"
	"16jZhKYDYP8nLFYnMpfB0C7HRY/iNpiWRdX+++bTvwXDEWL1rt4/+cdOjxZgQnAT\n"
	"B/TXsNgTmP2JhsYf9137AjlPXzz3ZKADyI6wwvI2a9x+6dNS3kspfeKaPvA5/lWF\n"
	"hmi5t0Eo83g+7NAIjDG09mC8H7oKW1O7j9LA9CRum4glkrtej9YuZzYSIFPIJX/L\n"
	"7IG5ccHkcF4hc0EOSLEDN4GLl8wbthWb6qgvj8dCg4ygh/Am6FxIkLvGlGbKjH7Q\n"
	"bE3tqXXC3ej0o3GH/wa6Mw7pjCKuTkFNDNbD+C4L2UItK2WUB/40Q8s5X06h4IZI\n"
	"quDirvsUaBdAmPgcYCYDG66fSJI/3C7ezEfarUHRroGxMUOd3Jg1A/yYjSHLq1Kj\n"
	"AzU5dnOI3TjFCGw5JOo0nrzFfY3hxh8gGRq+hieIITk9/SIRJvs8F2cKPS4Tg4mV\n"
	"EryoOvk3jA2z2KQCa7ipZGjbZawH5UK7yEWPc8pPzIAOjJw/Am6SKZzGlNamJRQv\n"
	"DSCP6Vm4SXkQudDJmNBZLI9bSy+tLL66buVG0KN+tdLTCL41UuBV9tYlI27I8drc\n"
	"Gj3/i2YAeT+w68GFHjWKY1OTaXRxKoC1J/3aXH8fCLL+DN2gOE53086ruCYbtTds\n"
	"oVM63TQCySNfMnnLSi4kgorUh6RQH4Wgf6H30XLjFkwSRXEDhbHKpbM4nw==\n"
	"=k3Ig\n"
	"-----END PGP MESSAGE-----\n";

void TestVerifyData(long refCon)
{
	cout << "PGPTest: Verify data" << endl;
	CSecurityPluginDLL::SDecryptVerifyData verify;
	char* out = NULL;
	unsigned long out_len = 0L;
	bool result = false;
	verify.mInputData = cOpenData5;
	verify.mInputSignature = cOpenSignature5;
	verify.mOutputData = &out;
	verify.mOutputDataLength = &out_len;
	verify.mSuccess = &result;

	cout << " DATA:" << endl << verify.mInputData << endl;
	if (verify.mInputSignature)
		cout << " SIGNATURE:" << endl << verify.mInputSignature << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDecryptVerifyData, &verify, refCon);

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

	cout << "PGPTest: Dispose Data" << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDisposeData, (void*) verify.mOutputData, refCon);
}

void TestDecryptData(long refCon)
{
	cout << "PGPTest: Decrypt data" << endl;
	CSecurityPluginDLL::SDecryptVerifyData decrypt;
	char* out = NULL;
	unsigned long out_len = 0L;
	bool result = false;
	decrypt.mInputData = cEncrypted;
	decrypt.mInputSignature = NULL;
	decrypt.mOutputData = &out;
	decrypt.mOutputDataLength = &out_len;
	decrypt.mSuccess = &result;

	cout << " DATA:" << endl << decrypt.mInputData << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDecryptVerifyData, &decrypt, refCon);

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

	cout << "PGPTest: Dispose Data" << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityDisposeData, (void*) decrypt.mOutputData, refCon);
}

void TestSignFile(long refCon)
{
#if __dest_os == __mac_os
	FSSpec in;
	FSSpec out;
	PrepareFiles(&in, &out);

	cout << "PGPTest: Sign file" << endl;
	CSecurityPluginDLL::SSignFile sign;
	sign.mInputFile = &in;
	sign.mKey = "daboo@cyrusoft.com";
	sign.mOutputFile = &out;
	sign.mUseMIME = true;

	cout << " FILE: " << in_file << endl;
	cout << " KEY: " << sign.mKey << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecuritySignFile, &sign, refCon);

	cout << " SIGNED: " << out_file  << endl;
#endif
}

void TestEncryptFile(long refCon)
{
#if __dest_os == __mac_os
	FSSpec in;
	FSSpec out;
	PrepareFiles(&in, &out);

	cout << "PGPTest: Encrypt file" << endl;
	char* keys[10];
	keys[0] = "daboo@cyrusoft.com";
	keys[1] = NULL;
	CSecurityPluginDLL::SEncryptFile encrypt;
	encrypt.mInputFile = &in;
	encrypt.mKeys = keys;
	encrypt.mOutputFile = &out;
	encrypt.mUseMIME = true;

	cout << " DATA: " << in_file << endl;
	cout << " KEY: " << *encrypt.mKeys << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityEncryptFile, &encrypt, refCon);

	cout << " ENCRYPTED: " << out_file << endl;
#endif
}

void TestEncryptSignFile(long refCon)
{
#if __dest_os == __mac_os
	FSSpec in;
	FSSpec out;
	PrepareFiles(&in, &out);

	cout << "PGPTest: Encrypt-sign file" << endl;
	char* keys[10];
	keys[0] = "daboo@cyrusoft.com";
	keys[1] = NULL;
	CSecurityPluginDLL::SEncryptSignFile esign;
	esign.mInputFile = &in;
	esign.mKeys = keys;
	esign.mSignKey = "daboo@cyrusoft.com";
	esign.mOutputFile = &out;
	esign.mUseMIME = true;

	cout << " DATA: " << in_file << endl;
	cout << " KEY: " << *esign.mKeys << endl;

	MulberryPluginEntry(CSecurityPluginDLL::eSecurityEncryptSignFile, &esign, refCon);

	cout << " ENCRYPTED-SIGNED: " << out_file << endl;
#endif
}

#if __dest_os == __mac_os
void PrepareFiles(FSSpec* in, FSSpec* out)
{
	// Convert input file to FSSpec
	::FSpLocationFromFullPath(::strlen(in_file), in_file, in);
	
	// Delete any existing output file
	::fclose(::fopen(out_file, "w"));
	
	// Convert output file to FSSpec
	::FSpLocationFromFullPath(::strlen(out_file), out_file, out);
}
#endif
