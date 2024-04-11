using System.Windows;
using Pathoschild.Http.Client;
using System.IO;
using WPF.Models;
using System.Windows.Forms;
namespace WPF
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private readonly FluentClient client;
        private readonly Settings settings;
        public MainWindow()
        {
            client = new();
            settings = new();
            InitializeComponent();
        }
        private async void BtnClick(object sender, RoutedEventArgs e)
        {
            if (!settings.LoadSettings())
            {
                displayText.Text = "Please enter the path to your addons folder:";

                using (var dialog = new FolderBrowserDialog())
                {
                    DialogResult result = dialog.ShowDialog();
                    if (result == System.Windows.Forms.DialogResult.OK && !string.IsNullOrWhiteSpace(dialog.SelectedPath))
                    {
                        SelectedDirectoryLabel.Content = "Selected Directory: " + dialog.SelectedPath;
                        settings.AddonsFolderPath = dialog.SelectedPath;
                    }
                }
                settings.SaveSettings();
            }

            await Update_ElvUI(settings);
        }
        private async Task Update_ElvUI(Settings settings)
        {
            try
            {
                IResponse response = await client.GetAsync("https://api.tukui.org/v1/addon/elvui");

                ElvUIModel responseData = await response.As<ElvUIModel>();

                string tocFilePath = $"{settings.AddonsFolderPath}/ElvUI/ElvUI_Mainline.toc";

                if (File.Exists(tocFilePath))
                {
                    string vLine = File.ReadLines(tocFilePath).Skip(3).Take(1).FirstOrDefault();

                    if (vLine != null)
                    {
                        string installedVersion = vLine.Split(':')[1]?.Trim();

                        if (installedVersion == responseData.Version)
                        {
                            displayText.Text = $"You already have the latest version of ElvUI ({responseData.Version}).";
                            elvUIBtn.Content = "Up To Date!";
                            elvUIBtn.IsEnabled = false;
                            return;
                        }
                    }
                }
                else
                {
                    displayText.Text = "Couldn't find ElvUI. (Check if your settings have the correct path)";
                    return;
                }

                await DownloadAndExtractElvUIAsync(settings.AddonsFolderPath, responseData.Url, responseData.Version);
            }
            catch (Exception ex)
            {
                displayText.Text = $"An exception occurred: {ex.Message}";
            }
        }
        private async Task DownloadAndExtractElvUIAsync(string addonsPath, string url, string v)
        {
            try
            {
                IResponse response = await client.GetAsync(url);

                string elvuiZip = $"./elvui-{v}.zip";
                byte[] fileBytes = await response.AsByteArray();

                File.WriteAllBytes(elvuiZip, fileBytes);
                System.IO.Compression.ZipFile.ExtractToDirectory(elvuiZip, addonsPath, true);
                File.Delete(elvuiZip);
                displayText.Text = $"ElvUI successfully updated to version {v}.\nPress any key to exit.";
                elvUIBtn.Content = "Up To Date!";
                elvUIBtn.IsEnabled = false;
            }
            catch (Exception ex)
            {
                displayText.Text = $"Failed to download or extract ElvUI. {ex.Message}";
            }
        }
    }
}
