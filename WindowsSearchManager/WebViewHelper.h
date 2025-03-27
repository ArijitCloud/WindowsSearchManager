#pragma once

#include <windows.h>
#include <unknwn.h> 
#include <wrl.h>
#include <WebView2.h>

using namespace Microsoft::WRL;

extern ICoreWebView2Controller* g_webViewController;
extern ICoreWebView2* g_webViewWindow;

// Initialize WebView2 in the specified window.
// This function creates the environment, controller, and navigates to a URL.
HRESULT InitializeWebView2(HWND hWnd);
