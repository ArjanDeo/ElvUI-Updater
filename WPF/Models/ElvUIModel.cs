using Newtonsoft.Json;

namespace WPF.Models
{
    public class ElvUIModel
    {
        [JsonProperty("id")]
        public int Id { get; set; }

        [JsonProperty("slug")]
        public string Slug { get; set; }

        [JsonProperty("author")]
        public string Author { get; set; }

        [JsonProperty("name")]
        public string Name { get; set; }

        [JsonProperty("url")]
        public string Url { get; set; }

        [JsonProperty("version")]
        public string Version { get; set; }

        [JsonProperty("changelog_url")]
        public string ChangelogUrl { get; set; }

        [JsonProperty("ticket_url")]
        public string TicketUrl { get; set; }

        [JsonProperty("git_url")]
        public string GitUrl { get; set; }

        [JsonProperty("patch")]
        public List<string> Patch { get; set; }

        [JsonProperty("last_update")]
        public string LastUpdate { get; set; }

        [JsonProperty("web_url")]
        public string WebUrl { get; set; }

        [JsonProperty("donate_url")]
        public string DonateUrl { get; set; }

        [JsonProperty("small_desc")]
        public string SmallDesc { get; set; }

        [JsonProperty("screenshot_url")]
        public string ScreenshotUrl { get; set; }

        [JsonProperty("directories")]
        public List<string> Directories { get; set; }
    }
}