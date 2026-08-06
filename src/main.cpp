#include "Settings.hpp"
#include <iostream>

int main()
{
    Settings settings;
    settings.load();

    if (!Settings::isValidWowDirectory(settings.getWowDirectory()))
    {
        std::string input;

        while (true)
        {
            std::cout << "WoW installation directory not found or invalid.\n";
            std::cout << "Enter the path to your World of Warcraft folder: ";
            std::getline(std::cin, input);

            if (Settings::isValidWowDirectory(input))
            {
                settings.setWowDirectory(input);
                settings.save();
                break;
            }

            std::cout << "That doesn't look like a valid WoW directory "
                          "(missing Interface/AddOns). Try again.\n\n";
        }
    }

    std::cout << "Using WoW directory: " << settings.getWowDirectory() << "\n";
    

    return 0;
}