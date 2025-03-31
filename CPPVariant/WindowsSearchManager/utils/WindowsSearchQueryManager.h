#pragma once
#include <string>
#include <vector>
#include <map>
#include <comdef.h> // For _variant_t
#include "SearchSelectorBuilder.h"

// A search result: mapping from column name to its value (stored as a _variant_t).
typedef std::map<std::wstring, _variant_t> SearchResult;

class WindowsSearchQueryManager {
public:    
    std::vector<SearchResult> ExecuteQuery(const std::wstring& sqlQuery, const SearchSelectorOptions& selectorOptions);
};
