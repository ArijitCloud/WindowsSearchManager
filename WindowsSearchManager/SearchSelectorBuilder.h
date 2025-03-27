#pragma once
#include <string>
#include <vector>

// Builder that helps select which columns to retrieve in the SELECT clause.
class SearchSelectorBuilder
{
public:
    // Add one or more columns to the selection.
    SearchSelectorBuilder& Select(const std::wstring& column);

    // Builds a comma-separated string of selected columns.
    // If no columns were selected, fallback to default (System.ItemName, System.ItemUrl).
    std::wstring BuildColumns() const;

private:
    std::vector<std::wstring> m_columns;
};
#pragma once
