#include "application.hpp"

int main(int argc, char ** argv) {
    Application app;
    if (argc < 2) {
        std::cout << "No arguments. Pass path to gltf model." << std::endl;
        return 1;
    }

    return app.run(argv[1]);
}

