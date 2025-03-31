#pragma once

#include <vector>
#include <string>
#include <initializer_list>

// Structure to hold the selector options.
struct SearchSelectorOptions {
    std::vector<std::wstring> Columns;
};

// Builder class declaration.
class SearchSelectorBuilder {
public:
    // Adds multiple columns at once.
    SearchSelectorBuilder& Select(std::initializer_list<std::wstring> columns);

    // Returns the built selector options.
    SearchSelectorOptions Build() const;

private:
    SearchSelectorOptions options;
};
