#pragma once
#include <string>

// Simple struct to hold query-building options:
struct SearchQueryOptions
{
    std::wstring SearchTerm;
    bool DisableEmail = false;
    std::wstring FileExtension; // e.g., ".jpg" or ".pdf"
};

class SearchQueryBuilder
{
public:
    // Builds a SQL query using the given options.
    // e.g., SELECT TOP 100 System.ItemName, System.ItemUrl FROM SYSTEMINDEX WHERE ...
    static std::wstring BuildQuery(const SearchQueryOptions& options, int topCount = 100);
};
