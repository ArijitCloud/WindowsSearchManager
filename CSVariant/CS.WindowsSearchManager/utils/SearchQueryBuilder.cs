using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CS.WindowsSearchManager
{
    public class SearchQueryOptions
    {
        // The search term to match in the item name.
        public required string SearchTerm { get; set; }

        // Flag to disable email items.
        public bool DisableEmail { get; set; }

        // You can add more flags/properties as needed.
        // For example, a flag to filter by file type, a date range, etc.
        public string? FileExtension { get; set; } // e.g., ".txt", ".docx" (optional)
    }


    public class SearchQueryBuilder
    {
        public string BuildQuery(SearchQueryOptions queryOptions, SearchSelectorOptions selectorOptions, int topCount = 100)
        {
            StringBuilder queryBuilder = new StringBuilder();
            queryBuilder.Append($"SELECT TOP {topCount} ");

            // Build the column list dynamically.
            if (selectorOptions.Columns.Count > 0)
            {
                queryBuilder.Append(string.Join(", ", selectorOptions.Columns));
            }
            else
            {
                // Fallback default columns
                queryBuilder.Append("System.ItemName, System.ItemUrl");
            }

            queryBuilder.Append(" FROM SYSTEMINDEX");

            bool firstCondition = true;
            void AppendCondition(string condition)
            {
                if (firstCondition)
                {
                    queryBuilder.Append(" WHERE ");
                    firstCondition = false;
                }
                else
                {
                    queryBuilder.Append(" AND ");
                }
                queryBuilder.Append(condition);
            }

            // Add conditions from queryOptions as before.
            if (!string.IsNullOrWhiteSpace(queryOptions.SearchTerm))
            {
                string safeTerm = queryOptions.SearchTerm.Replace("'", "''");
                AppendCondition($"System.ItemName LIKE '%{safeTerm}%'");
            }
            if (queryOptions.DisableEmail)
            {
                AppendCondition("System.ItemUrl NOT LIKE 'mapi%'");
            }
            if (!string.IsNullOrWhiteSpace(queryOptions.FileExtension))
            {
                string ext = queryOptions.FileExtension.StartsWith(".") ? queryOptions.FileExtension : "." + queryOptions.FileExtension;
                AppendCondition($"System.FileExtension = '{ext}'");
            }
            else
            {
                AppendCondition("System.FileExtension IS NOT NULL");
            }

            return queryBuilder.ToString();
        }
    }

}
