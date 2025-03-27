using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CS.WindowsSearchManager
{
    public class SearchSelectorOptions
    {
        public List<string> Columns { get; } = new List<string>();
    }

    public class SearchSelectorBuilder
    {
        private readonly SearchSelectorOptions _options = new SearchSelectorOptions();

       
        public SearchSelectorBuilder Select(params string[] columns)
        {
            _options.Columns.AddRange(columns);
            return this;
        }

        public SearchSelectorOptions Build() => _options;
    }

}
