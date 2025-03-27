using Microsoft.Web.WebView2.WinForms;
using System.Text.Json;

namespace CS.WindowsSearchManager
{
    public partial class WindowsSearchManager : Form
    {
        public WebView2 webView21;
        public WindowsSearchManager()
        {
            InitializeComponent();
            InitializeAsync();
        }

        private async void InitializeAsync()
        {
            webView21 = new WebView2
            {
                Dock = DockStyle.Fill
            };

            this.Controls.Add(webView21);

            await webView21.EnsureCoreWebView2Async();
            webView21.CoreWebView2.WebMessageReceived += CoreWebView2_WebMessageReceived;
            webView21.Source = new Uri(Path.Combine(Application.StartupPath, "index.html"));
        }

        private void CoreWebView2_WebMessageReceived(object? sender, Microsoft.Web.WebView2.Core.CoreWebView2WebMessageReceivedEventArgs e)
        {
            var queryText = JsonDocument.Parse(e.WebMessageAsJson).RootElement.GetProperty("query").GetString();

            SearchQueryOptions queryOptions = new SearchQueryOptions
            {
                SearchTerm = queryText??"",
                DisableEmail = true
                // Additional options...
            };

            // Build the selector.
            SearchSelectorOptions selectorOptions = new SearchSelectorBuilder()
                .Select("System.ItemName", "System.ItemUrl", "System.FileExtension", "System.ItemDate", "System.Kind")
                .Build();

            // Build the final SQL query.
            SearchQueryBuilder queryBuilder = new SearchQueryBuilder();
            string sqlQuery = queryBuilder.BuildQuery(queryOptions, selectorOptions);

            var searchQueryManager = new WindowsSearchQueryManager();
            var results = searchQueryManager.QueryWindowsSearch(sqlQuery, selectorOptions);

            //Show the result json
            var jsonResults = JsonSerializer.Serialize(results);

            webView21.CoreWebView2.PostWebMessageAsJson(jsonResults);
        }

        private void WindowsSearchManager_Load(object sender, EventArgs e)
        {

        }
    }
}
