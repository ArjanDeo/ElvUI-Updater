using Newtonsoft.Json;
using ElvUI_Updater.Main.Models;

namespace ElvUI_Updater.Main
{
    public class Program
    {
        private static readonly HttpClient client = new();

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

            await UpdateElvUIAsync(settings);
        }

        public static async Task UpdateElvUIAsync(Settings settings)
        {
            try
            {
                HttpResponseMessage response = await client.GetAsync("https://api.tukui.org/v1/addon/elvui");
                response.EnsureSuccessStatusCode();

                string responseBody = await response.Content.ReadAsStringAsync();
                var responseData = JsonConvert.DeserializeObject<ElvUIModel>(responseBody);

                string tocFilePath = $"{settings.AddonsFolderPath}/ElvUI/ElvUI_Mainline.toc";

                if (File.Exists(tocFilePath))
                {
                    string vLine = File.ReadLines(tocFilePath).Skip(3).Take(1).FirstOrDefault();

                    if (vLine != null)
                    {
                        string installedVersion = vLine.Split(':')[1]?.Trim();

                        if (installedVersion == responseData.Version)
                        {
                            Console.WriteLine($"You already have the latest version of ElvUI ({responseData.Version}).");
                            Console.ReadKey();
                            return;
                        }
                    }
                }
                else
                {
                    Console.WriteLine("Couldn't find ElvUI. (Check if your settings have the correct path)");
                    Console.ReadKey();
                    return;
                }

                await DownloadAndExtractElvUIAsync(settings.AddonsFolderPath, responseData.Url, responseData.Version);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"An exception occurred: {ex.Message}");
            }
        }

        public static async Task DownloadAndExtractElvUIAsync(string addonsPath, string url, string v)
        {
            try
            {
                HttpResponseMessage response = await client.GetAsync(url);
                response.EnsureSuccessStatusCode();

                string elvuiZip = $"./elvui-{v}.zip";
                byte[] fileBytes = await response.Content.ReadAsByteArrayAsync();

                File.WriteAllBytes(elvuiZip, fileBytes);
                System.IO.Compression.ZipFile.ExtractToDirectory(elvuiZip, addonsPath, true);
                File.Delete(elvuiZip);
                Console.WriteLine($"ElvUI successfully updated to version {v}.\nPress any key to exit.");
                Console.ReadKey();
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to download or extract ElvUI. {ex.Message}");
            }
        }
    }
}