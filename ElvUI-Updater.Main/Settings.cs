namespace ElvUI_Updater.Main
{
    public class Settings
    {
        public string AddonsFolderPath { get; set; }
    
        public void SaveSettings()
        {
            string content = AddonsFolderPath;
            File.WriteAllText("settings.txt", content);
        }


        public bool LoadSettings()
        {
            if (File.Exists("settings.txt"))
            {
                string[] lines = File.ReadAllLines("settings.txt");
                AddonsFolderPath = lines[0];                  
               
            }
            else
            {
                Console.WriteLine("Error: Settings file not found.");
                return false;
            }


            return false;
        }
    }
}
