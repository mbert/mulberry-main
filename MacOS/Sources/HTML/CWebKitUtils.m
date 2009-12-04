//#define AVAILABLE_MAC_OS_X_VERSION_10_3_AND_LATER
#include <Cocoa/Cocoa.h>
#include <WebKit/WebKit.h>
#include <WebKit/HIWebView.h>
#include <WebKit/CarbonUtils.h>

#include "CWebKitUtils.h"

void InitWebKit()
{
	// Init webkit
	WebInitForCarbon();
}

OSStatus CreateHIWebView(HIViewRef *viewref)
{
	return HIWebViewCreate(viewref);	
}

void URLToWebView(HIViewRef inView, CFURLRef inURL)
{
	WebView*		nativeView;
    NSURLRequest*	request;
    WebFrame* 		mainFrame;
	
	nativeView = HIWebViewGetWebView( inView );
	request = [NSURLRequest requestWithURL:(NSURL*)inURL];
	mainFrame = [nativeView mainFrame];
	[mainFrame loadRequest:request];
}

void DataToWebView(HIViewRef inView, CFStringRef inData)
{
	WebView*		nativeView;
    WebFrame* 		mainFrame;
	
	nativeView = HIWebViewGetWebView( inView );
	mainFrame = [nativeView mainFrame];
	[mainFrame loadHTMLString:(NSString*)inData baseURL:nil];
}
