#include "CLI.h"


CLI::CLI(Database& db) : db(db), window(sf::VideoMode(800, 600), "Simple Database CLI") {
    // Initialization
}

void CLI::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Handle input for save and load commands
            if (event.type == sf::Event::TextEntered) {
                std::string inputCommand; // Assume this is filled with user input
                std::istringstream iss(inputCommand);
                std::string command;
                iss >> command;
                if (command == "save") {
                    std::string filename;
                    iss >> filename;
                    try {
                        FileOps::saveDatabase(db, filename);
                        std::cout << "Database saved to " << filename << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "Error saving database: " << e.what() << std::endl;
                    }
                } else if (command == "load") {
                    std::string filename;
                    iss >>
                        filename;
                    try {
                        db = FileOps::loadDatabase(filename);
                        std::cout << "Database loaded from " << filename << std::endl;
                    } catch (const std::exception& e) {
                        std::cerr << "Error loading database: " << e.what() << std::endl;
                    }
                }
// Handle other commands...
            }
        }
        // ... [Rendering and other event handling] ...
    }
}