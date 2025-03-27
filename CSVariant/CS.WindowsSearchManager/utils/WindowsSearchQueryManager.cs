using System;
using System.Collections.Generic;
using System.Data.OleDb;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CS.WindowsSearchManager
{
    public class WindowsSearchQueryManager
    {
        public List<Dictionary<string, object>> QueryWindowsSearch(string sqlQuery, SearchSelectorOptions selectorOptions)
        {
            var results = new List<Dictionary<string, object>>();
            using (var conn = new OleDbConnection("Provider=Search.CollatorDSO;Extended Properties='Application=Windows'"))
            using (var cmd = new OleDbCommand(sqlQuery, conn))
            {
                conn.Open();
                using (var reader = cmd.ExecuteReader())
                {
                    while (reader.Read())
                    {
                        var row = new Dictionary<string, object>();
                        // Use the selector options to determine which columns to map.
                        foreach (var col in selectorOptions.Columns)
                        {
                            row[col] = reader[col];
                        }
                        results.Add(row);
                    }
                }
            }
            return results;
        }

    }
}
