#include "WindowsSearchQueryManager.h"
#include <windows.h>
#import "C:\\Program Files\\Common Files\\System\\ado\\msado15.dll" no_namespace rename("EOF", "EndOfFile")
#include <vector>
#include <iostream>

// Implements ExecuteQuery which mimics the C# method.
std::vector<SearchResult> WindowsSearchQueryManager::ExecuteQuery(const std::wstring& sqlQuery, const SearchSelectorOptions& selectorOptions)
{
    std::vector<SearchResult> results;

    // OLE DB connection string for Windows Search.
    _bstr_t connectionString(L"Provider=Search.CollatorDSO;Extended Properties='Application=Windows'");

    // Create an ADO Connection pointer.
    _ConnectionPtr pConn;
    HRESULT hr = pConn.CreateInstance(__uuidof(Connection));
    if (FAILED(hr))
    {
        std::wcerr << L"Failed to create ADO Connection. HRESULT: 0x" << std::hex << hr << std::endl;
        return results;
    }

    try
    {
        // Open the connection.
        pConn->Open(connectionString, "", "", adConnectUnspecified);
    }
    catch (_com_error& e)
    {
        std::wcerr << L"Connection Open failed: " << e.ErrorMessage() << std::endl;
        return results;
    }

    // Execute the SQL query.
    _RecordsetPtr pRst;
    try
    {
        pRst = pConn->Execute(_bstr_t(sqlQuery.c_str()), nullptr, adCmdText);
    }
    catch (_com_error& e)
    {
        std::wcerr << L"Execute failed: " << e.ErrorMessage() << std::endl;
        pConn->Close();
        return results;
    }

    // Loop through the recordset.
    while (!pRst->EndOfFile)
    {
        SearchResult row;
        // For each column specified in the selector options, extract its value.
        for (const auto& col : selectorOptions.Columns)
        {
            _variant_t vtValue;
            try {
                vtValue = pRst->Fields->Item[col.c_str()]->Value;
            }
            catch (_com_error& e) {
                // Assign a default empty value if the column doesn't exist.
                vtValue = L"";
            }
            row[col] = vtValue;

        }
        results.push_back(row);
        pRst->MoveNext();
    }

    // Clean up.
    pRst->Close();
    pConn->Close();

    return results;
}
