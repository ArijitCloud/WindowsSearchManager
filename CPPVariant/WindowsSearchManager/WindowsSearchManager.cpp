// WindowsSearchManager.cpp
// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
// <IncludeHeader>
// include WebView2 header
#include "WebView2.h"
// </IncludeHeader>
#include <shlwapi.h>
#include "utils/SearchQueryBuilder.h"
#include "utils/SearchSelectorBuilder.h"
#include "utils/WindowsSearchQueryManager.h"

#pragma comment(lib, "Shlwapi.lib")

using namespace Microsoft::WRL;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Windows Desktop Search Application");

// Stored instance handle for use in Win32 API calls such as FindResource
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Pointer to WebViewController
static wil::com_ptr<ICoreWebView2Controller> webviewController;

// Pointer to WebView window
static wil::com_ptr<ICoreWebView2> webview;

//
// GetLocalHtmlPath - Computes the file URL of "index.html" relative to the executable.
// It retrieves the folder of the EXE, appends "index.html", replaces backslashes with forward slashes,
// and prepends "file:///".
//
std::wstring GetLocalHtmlPath()
{
	WCHAR exePath[MAX_PATH];
	GetModuleFileName(nullptr, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);  // Remove the executable name.

	std::wstring folder(exePath);
	std::wstring htmlPath = folder + L"\\index.html";

	// Convert backslashes to forward slashes.
	for (auto& ch : htmlPath)
	{
		if (ch == L'\\')
			ch = L'/';
	}

	return L"file:///" + htmlPath;
}

std::wstring ConvertVariantToString(const _variant_t& varValue)
{
	// If it's a date, convert from OLE Automation date to SYSTEMTIME,
	// then format it as a string.
	if (varValue.vt == VT_DATE)
	{
		SYSTEMTIME st;
		if (::VariantTimeToSystemTime(varValue.date, &st))
		{
			// Format it in your desired style, e.g. "YYYY-MM-DD HH:MM:SS".
			// We'll do it manually here.
			wchar_t buf[64];
			swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond);
			return buf;
		}
		// If conversion fails, fallback
		return L"[Invalid DATE]";
	}
	else if (varValue.vt == VT_BSTR && varValue.bstrVal != nullptr)
	{
		// For string values, just use the BSTR
		return varValue.bstrVal;
	}
	else
	{

		try {
			return static_cast<const wchar_t*>(_bstr_t(varValue));
		}
		catch (const _com_error& e) {
			//MessageBox(nullptr, e.ErrorMessage(), L"Conversion Error", MB_OK);
			return L"";
		}
		// Fallback: rely on _bstr_t to convert to a string if possible
		//return static_cast<const wchar_t*>(_bstr_t(varValue));
	}
}

void processMessage(wil::unique_cotaskmem_string* message) {
	// Process the message received from the web content
	// For example, you can log it or display it in a message box
	MessageBox(NULL, message->get(), L"Message from WebView", MB_OK);
}


int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	// The parameters to CreateWindowEx explained:
	// WS_EX_OVERLAPPEDWINDOW : An optional extended window style.
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		800, 600,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);


	// <-- WebView2 code starts here -->
	// Step 3 - Create a single WebView within the parent window
	// Locate the browser and set up the environment for WebView
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window hWnd
				env->CreateCoreWebView2Controller(hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							webviewController = controller;
							webviewController->get_CoreWebView2(&webview);
						}

						// Add a few settings for the webview - can be removed
						// The demo step is redundant since the values are the default settings but we want to keep this as placeholder for now
						wil::com_ptr<ICoreWebView2Settings> settings;
						webview->get_Settings(&settings);
						settings->put_IsScriptEnabled(TRUE);
						settings->put_AreDefaultScriptDialogsEnabled(TRUE);
						settings->put_IsWebMessageEnabled(TRUE);

						// Resize WebView to fit the bounds of the parent window
						RECT bounds;
						GetClientRect(hWnd, &bounds);
						webviewController->put_Bounds(bounds);



						std::wstring url = GetLocalHtmlPath();
						LPCWSTR localPathCStr = url.c_str();

						webview->Navigate(localPathCStr);

						// <NavigationEvents>
						// Step 4 - Navigation events
						// register an ICoreWebView2NavigationStartingEventHandler to cancel any non-https navigation
						EventRegistrationToken token;
						//webview->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
						//	[](ICoreWebView2* webview, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
						//		wil::unique_cotaskmem_string uri;
						//		args->get_Uri(&uri);
						//		std::wstring source(uri.get());
						//		if (source.substr(0, 5) != L"https") {
						//			// Cancel the navigation
						//			//args->put_Cancel(true);
						//		}
						//		return S_OK;
						//	}).Get(), &token);
						// </NavigationEvents>

						// <Scripting>
						// Step 5 - Scripting
						// Schedule an async task to add initialization script that freezes the Object object
						//webview->AddScriptToExecuteOnDocumentCreated(L"Object.freeze(Object);", nullptr);
						// Schedule an async task to get the document URL
						//webview->ExecuteScript(L"window.document.URL;", Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
						//	[](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
						//		LPCWSTR URL = resultObjectAsJson;
						//		//doSomethingWithURL(URL);
						//		return S_OK;
						//	}).Get());
						// </Scripting>

						// <CommunicationHostWeb>
						// Step 6 - Communication between host and web content
						// Set an event handler for the host to return received message back to the web content
						webview->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							[](ICoreWebView2* webview, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
								// Extract raw JSON string
								wil::unique_cotaskmem_string rawMessage;
								HRESULT hr=	args->get_WebMessageAsJson(&rawMessage);
								if (FAILED(hr)) {
									MessageBox(nullptr, L"Failed to get web view message", L"Error", MB_OK);									
								}
								std::wstring message = rawMessage.get() ? rawMessage.get() : L"";


								// Very simple JSON parsing for: { "query": "value" }
								std::wstring searchTerm;
								size_t pos = message.find(L"\"query\"");
								if (pos != std::wstring::npos)
								{
									size_t colon = message.find(L":", pos);
									size_t firstQuote = message.find(L"\"", colon + 1);
									size_t secondQuote = message.find(L"\"", firstQuote + 1);
									if (colon != std::wstring::npos && firstQuote != std::wstring::npos && secondQuote != std::wstring::npos)
									{
										searchTerm = message.substr(firstQuote + 1, secondQuote - (firstQuote + 1));
									}
								}

								// Build & run query
								SearchQueryOptions queryOpts;
								queryOpts.SearchTerm = searchTerm;
								queryOpts.DisableEmail = true;

								SearchSelectorOptions selectorOptions = SearchSelectorBuilder()
									.Select({ L"System.ItemName", L"System.ItemUrl", L"System.FileExtension", L"System.ItemDate" })
									.Build();


								std::wstring sqlQuery = SearchQueryBuilder::BuildQuery(queryOpts, selectorOptions, 100);
								WindowsSearchQueryManager queryManager;
								auto results = queryManager.ExecuteQuery(sqlQuery, selectorOptions);

								// Convert to JSON (basic manual conversion)
								std::wstring json = L"[";
								for (size_t i = 0; i < results.size(); ++i)
								{
									if (i > 0)
										json += L",";
									json += L"{";

									bool firstCol = true;
									for (const auto& pair : results[i])
									{
										if (!firstCol)
											json += L",";
										firstCol = false;
										// Assuming pair.first is the column name and pair.second is a _variant_t holding a BSTR.
										std::wstring key = pair.first;
										std::wstring value = ConvertVariantToString(pair.second);

										// Build the JSON property.
										json += std::wstring(L"\"") + key + L"\":\"" + value + L"\"";
									}
									json += L"}";
								}
								json += L"]";


								webview->PostWebMessageAsString(json.c_str());

								return S_OK;
							}).Get(), &token);

						// Schedule an async task to add initialization script that
						// 1) Add an listener to print message from the host
						// 2) Post document URL to the host
					/*	webview->AddScriptToExecuteOnDocumentCreated(
							L"window.chrome.webview.addEventListener(\'message\', event => alert(event.data));" \
							L"window.chrome.webview.postMessage(window.document.URL);",
							nullptr);*/
							// </CommunicationHostWeb>

						return S_OK;
					}).Get());
				return S_OK;
			}).Get());



	// <-- WebView2 sample code ends here -->


	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	TCHAR greeting[] = _T("Hello, Loading the webview page...");

	switch (message)
	{
	case WM_SIZE:
		if (webviewController != nullptr) {
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			webviewController->put_Bounds(bounds);
		};
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		// Here your application is laid out.
		// For this introduction, we just print out "Hello, Windows desktop!"
		// in the top left corner.
		TextOut(hdc,
			5, 5,
			greeting, _tcslen(greeting));
		// End application-specific layout section.

		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}