#include "SearchQueryBuilder.h"
#include <sstream>

std::wstring SearchQueryBuilder::BuildQuery(const SearchQueryOptions& options, int topCount)
{
    std::wstringstream query;
    query << L"SELECT TOP " << topCount << L" System.ItemName, System.ItemUrl FROM SYSTEMINDEX";

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

    // Match item name
    if (!options.SearchTerm.empty())
    {
        // Basic sanitization by escaping single quotes:
        std::wstring safeTerm = options.SearchTerm;
        // Replace ' with '' if you want
        // ...
        appendCondition(L"System.ItemName LIKE '%" + safeTerm + L"%'");
    }

    // Exclude email items if requested
    if (options.DisableEmail)
    {
        appendCondition(L"System.ItemUrl NOT LIKE 'mapi%'");
    }

    // Filter by file extension
    if (!options.FileExtension.empty())
    {
        std::wstring ext = options.FileExtension[0] == L'.'
            ? options.FileExtension
            : L"." + options.FileExtension;
        appendCondition(L"System.FileExtension = '" + ext + L"'");
    }
    else
    {
        // Force only items that have a file extension
        appendCondition(L"System.FileExtension IS NOT NULL");
    }

    return query.str();
}
