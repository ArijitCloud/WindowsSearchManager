#include "SearchSelectorBuilder.h"
#include <sstream>

SearchSelectorBuilder& SearchSelectorBuilder::Select(const std::wstring& column)
{
    m_columns.push_back(column);
    return *this; // allow chaining
}

std::wstring SearchSelectorBuilder::BuildColumns() const
{
    if (m_columns.empty())
    {
        // Fallback if no columns selected
        return L"System.ItemName, System.ItemUrl";
    }

    // Join the selected columns with commas
    std::wstringstream ss;
    for (size_t i = 0; i < m_columns.size(); ++i)
    {
        if (i > 0) ss << L", ";
        ss << m_columns[i];
    }
    return ss.str();
}
