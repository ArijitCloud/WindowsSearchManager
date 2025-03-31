#include "SearchQueryBuilder.h"
#include <sstream>
#include <algorithm>

std::wstring SearchQueryBuilder::BuildQuery(const SearchQueryOptions& queryOpts,
    const SearchSelectorOptions& selectorOptions,
    int topCount)
{
    std::wstringstream query;

    // Build the SELECT clause using dynamic selector options.
    query << L"SELECT TOP " << topCount << L" ";
    if (!selectorOptions.Columns.empty()) {
        // Join the columns with a comma.
        for (size_t i = 0; i < selectorOptions.Columns.size(); i++) {
            if (i > 0) {
                query << L", ";
            }
            query << selectorOptions.Columns[i];
        }
    }
    else {
        // Default columns if none provided.
        query << L"System.ItemName, System.ItemUrl";
    }

    query << L" FROM SYSTEMINDEX";

    bool firstCondition = true;
    auto appendCondition = [&](const std::wstring& condition)
        {
            if (firstCondition)
            {
                query << L" WHERE " << condition;
                firstCondition = false;
            }
            else
            {
                query << L" AND " << condition;
            }
        };

    // If SearchTerm is specified, add a condition to match System.ItemName.
    if (!queryOpts.SearchTerm.empty())
    {
        // Basic sanitization: escape single quotes.
        std::wstring safeTerm = queryOpts.SearchTerm;
        size_t pos = 0;
        while ((pos = safeTerm.find(L"'", pos)) != std::wstring::npos) {
            safeTerm.replace(pos, 1, L"''");
            pos += 2;
        }
        appendCondition(L"System.ItemName LIKE '%" + safeTerm + L"%'");
    }

    // Exclude email items if requested.
    if (queryOpts.DisableEmail)
    {
        appendCondition(L"System.ItemUrl NOT LIKE 'mapi%'");
    }

    // If FileExtension is specified, filter by it.
    if (!queryOpts.FileExtension.empty())
    {
        std::wstring ext = (queryOpts.FileExtension[0] == L'.')
            ? queryOpts.FileExtension
            : L"." + queryOpts.FileExtension;
        appendCondition(L"System.FileExtension = '" + ext + L"'");
    }
    else
    {
        // Otherwise, require that the file extension is not null.
        appendCondition(L"System.FileExtension IS NOT NULL");
    }

    return query.str();
}
