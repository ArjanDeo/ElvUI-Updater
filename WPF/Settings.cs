using System.IO;

namespace WPF
{
    public class Settings
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
                string[] lines = File.ReadAllLines("settings.txt");
                if (lines.Length > 0)
                {
                    AddonsFolderPath = lines[0];
                    return true;
                }
            }
            return false;
        }
    }
}
