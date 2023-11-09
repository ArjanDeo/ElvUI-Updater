using Newtonsoft.Json;
using ElvUI_Updater.Main.Models;

namespace ElvUI_Updater.Main
{
    public class Program
    {
        private static readonly HttpClient client;
        static Program()
        {
            client = new();
        }

        static async Task Main()
        {
            Settings settings = new();

            if (!settings.LoadSettings())
            {
                Console.WriteLine("Please enter the path to your addons folder:");
                string addonsPath = Console.ReadLine();

                settings.AddonsFolderPath = addonsPath;
                settings.SaveSettings();
            }
          
            try
            {
                await GetElvUIAPIAsync(settings.AddonsFolderPath);
            }
            catch (Exception ex)
            {
                Console.WriteLine("An exception occurred: " + ex.Message);
            }
        }
        
        public static async Task GetElvUIAPIAsync(string addonsPath)
        {
            try
            {
                HttpResponseMessage response = await client.GetAsync("https://api.tukui.org/v1/addon/elvui");
                response.EnsureSuccessStatusCode();

                string responseBody = await response.Content.ReadAsStringAsync();
                var x = JsonConvert.DeserializeObject<ElvUIModel>(responseBody);

                await DownloadAndExtractElvUIAsync(addonsPath, x.Url, x.Version);
            }
            catch (HttpRequestException ex)
            {
                Console.WriteLine($"HTTP request exception: {ex.Message}");
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An exception occurred: {ex.Message}");
            }
        }
      
        public static async Task DownloadAndExtractElvUIAsync(string addonsPath, string url, string v)
        {
            HttpResponseMessage response = await client.GetAsync(url);
            if (response.IsSuccessStatusCode)
            {
                string elvuiZip = $"./elvui-{v}.zip";

                byte[] fileBytes = await response.Content.ReadAsByteArrayAsync();
                string vLine = "";
                try
                {
                    vLine = File.ReadLines($"{addonsPath}/ElvUI/ElvUI_Mainline.toc").Skip(3).Take(1).First();
                }
                catch
                {
                    Console.WriteLine("Couldn't find ElvUI. (Check if your settings has the correct path)");
                    Console.ReadKey();
                    Environment.Exit(0);
                }

                bool isSameVersion = vLine.Contains(v);
                if (isSameVersion)
                {
                    Console.WriteLine($"ElvUI is up to date. (version {v})");
                    Console.ReadKey();
                }
                else
                {
                    File.WriteAllBytes(elvuiZip, fileBytes);
                    System.IO.Compression.ZipFile.ExtractToDirectory(elvuiZip, addonsPath, System.Text.Encoding.UTF8, true);
                    File.Delete(elvuiZip);
                    Console.WriteLine($"ElvUI successfully updated to version {v}.\nPress any key to exit.");
                    Console.ReadKey();
                }
            }
            else
            {
                Console.WriteLine($"Failed to download ElvUI {v}. Status code: {response.StatusCode}");
            }
        }      
    }
}