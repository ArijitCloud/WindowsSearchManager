// WebViewHelper.cpp

#include "WebViewHelper.h"

#include <windows.h>
#include <unknwn.h>
#include <shlwapi.h>
#include <wrl.h>
#include <WebView2.h>
#include <string>

#include "SearchQueryBuilder.h"
#include "SearchSelectorBuilder.h"
#include "WindowsSearchQueryManager.h"

#pragma comment(lib, "Shlwapi.lib")

using namespace Microsoft::WRL;

// Global WebView2 pointers
ICoreWebView2Controller* g_webViewController = nullptr;
ICoreWebView2* g_webViewWindow = nullptr;

// Get path to local index.html file
static std::wstring GetLocalHtmlPath()
{
    WCHAR exePath[MAX_PATH];
    GetModuleFileName(nullptr, exePath, MAX_PATH);
    PathRemoveFileSpec(exePath);

    std::wstring htmlPath = std::wstring(exePath) + L"\\index.html";

    // Replace backslashes for file:// URL
    for (auto& ch : htmlPath) if (ch == L'\\') ch = L'/';
    return L"file:///" + htmlPath;
}

HRESULT InitializeWebView2(HWND hWnd)
{
    return CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [hWnd](HRESULT result, ICoreWebView2Environment* env) -> HRESULT
            {
                if (!env) return E_FAIL;

                return env->CreateCoreWebView2Controller(
                    hWnd,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                        [hWnd](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT
                        {
                            if (!controller) return E_FAIL;

                            g_webViewController = controller;
                            g_webViewController->get_CoreWebView2(&g_webViewWindow);

                            // Set parent window and visibility 
                            g_webViewController->put_ParentWindow(hWnd);   
                            g_webViewController->put_IsVisible(TRUE);

                            // Resize to fill window
                            RECT bounds;
                            GetClientRect(hWnd, &bounds);
                            g_webViewController->put_Bounds(bounds);

                            if (!g_webViewWindow) return E_FAIL;

                            // Load about:blank and test content
                            g_webViewWindow->Navigate(L"about:blank");

                            g_webViewWindow->add_NavigationCompleted(
                                Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                    [](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT
                                    {
                                        MessageBox(nullptr, L"Navigation Completed", L"Debug", MB_OK);
                                        sender->ExecuteScript(L"document.body.innerHTML = '<h1>Hello from WebView2!</h1>';", nullptr);
                                        return S_OK;
                                    }
                                ).Get(), nullptr);

                            // Add WebMessageReceived handler
                            g_webViewWindow->add_WebMessageReceived(
                                Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                    [&](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT
                                    {
                                        // Extract raw JSON string
                                        LPWSTR rawMessage = nullptr;
                                        args->TryGetWebMessageAsString(&rawMessage);
                                        std::wstring message = rawMessage ? rawMessage : L"";
                                        CoTaskMemFree(rawMessage);

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

                                        SearchSelectorBuilder selector;
                                        selector.Select(L"System.ItemName")
                                            .Select(L"System.ItemUrl")
                                            .Select(L"System.FileExtension");

                                        std::wstring sqlQuery = SearchQueryBuilder::BuildQuery(queryOpts, 100);
                                        auto results = WindowsSearchQueryManager::ExecuteQuery(sqlQuery);

                                        // Convert to JSON (basic)
                                        std::wstring json = L"[";
                                        for (size_t i = 0; i < results.size(); ++i)
                                        {
                                            if (i > 0) json += L",";
                                            json += L"{\"Name\":\"" + results[i].Name + L"\",\"Url\":\"" + results[i].Url + L"\"}";
                                        }
                                        json += L"]";

                                        sender->PostWebMessageAsString(json.c_str());
                                        return S_OK;
                                    }
                                ).Get(), nullptr);

                            return S_OK;
                        }
                    ).Get());
            }
        ).Get());
}
