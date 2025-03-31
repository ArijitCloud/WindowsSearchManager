#pragma once
#include <string>
#include <vector>
#include "SearchSelectorBuilder.h"

// Options for building the WHERE clause.
struct SearchQueryOptions {
    std::wstring SearchTerm;
    bool DisableEmail = false;
    std::wstring FileExtension; // empty means “any file extension”
};


class SearchQueryBuilder {
public:
    // Builds a SQL query given the query options, selector options, and max number of results.
    static std::wstring BuildQuery(const SearchQueryOptions& queryOpts,
        const SearchSelectorOptions& selectorOptions,
        int topCount);
};
