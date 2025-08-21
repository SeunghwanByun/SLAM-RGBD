#include "Viewer.hpp"
#include <iostream>

int main(){
    try {
        Youth::Viewer::Config config;
        config.width = 1280;
        config.height = 720;
        config.title = "Youth Viewer";
        config.vsync = true;

        Youth::Viewer viewer(config);
        if(!viewer.ok()) {
            std::cerr << "Failed to create viewer.\n";
            return 1;
        }
        viewer.run();
    }catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << '\n';
        return 2;
    }
    return 0;
}