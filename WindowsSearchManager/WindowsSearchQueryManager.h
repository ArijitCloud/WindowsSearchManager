#pragma once
#include <string>
#include <vector>

// A simple struct to represent one row of the search results.
struct SearchResult
{
    std::wstring Name;
    std::wstring Url;
    // Add more fields if needed
};

// Manager that executes queries against Windows Search
class WindowsSearchQueryManager
{
public:
    // Takes a fully built SQL query, executes it, and returns the results.
    static std::vector<SearchResult> ExecuteQuery(const std::wstring& sqlQuery);
};
