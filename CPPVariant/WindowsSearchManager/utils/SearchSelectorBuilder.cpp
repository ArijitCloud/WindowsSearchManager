#include "SearchSelectorBuilder.h"

SearchSelectorBuilder& SearchSelectorBuilder::Select(std::initializer_list<std::wstring> columns)
{
    for (const auto& col : columns)
    {
        options.Columns.push_back(col);
    }
    return *this;
}

SearchSelectorOptions SearchSelectorBuilder::Build() const
{
    return options;
}
