using Newtonsoft.Json;
using ElvUI_Updater.Main.Models;

namespace ElvUI_Updater.Main
{
	internal class Settings
	{
		public string AddonsFolderPath { get; set; }

		public void SaveSettings()
		{
			File.WriteAllText("settings.txt", AddonsFolderPath);
		}

		public bool LoadSettings()
		{
			if (File.Exists("settings.txt"))
			{
				AddonsFolderPath = File.ReadAllText("settings.txt");
				return true;
			}
			return false;
		}
	}
	internal class Program
	{
		static readonly HttpClient client = new HttpClient();

		static async Task Main()
		{
			Settings settings = new Settings();

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
				await DownloadAndExtractAddonAsync(addonsPath, x.Url, x.Version);
			}
			catch (HttpRequestException ex)
			{
				Console.WriteLine("HTTP request exception: " + ex.Message);
			}
			catch (Exception ex)
			{
				Console.WriteLine("An exception occurred: " + ex.Message);
			}
		}
		public static async Task DownloadAndExtractAddonAsync(string addonsPath, string url, string v)
		{
			HttpResponseMessage response = await client.GetAsync(url);
			if (response.IsSuccessStatusCode)
			{

				byte[] fileBytes = await response.Content.ReadAsByteArrayAsync();

				string line = File.ReadLines($"{addonsPath}/ElvUI/ElvUI_Mainline.toc").Skip(3).Take(1).First();
				bool isSameVersion = line.Contains(v);
				if (isSameVersion)
				{
					await Console.Out.WriteLineAsync($"ElvUI is up to date. (version {v})");
					Console.ReadKey();
				}
				else
				{
					File.WriteAllBytes($"./elvui-{v}.zip", fileBytes);
					System.IO.Compression.ZipFile.ExtractToDirectory($"./elvui-{v}.zip", addonsPath, System.Text.Encoding.UTF8, true);
					Console.WriteLine("File downloaded successfully.");
				}
			}
			else
			{
				Console.WriteLine($"Failed to download file. Status code: {response.StatusCode}");
			}
		}
	}

}