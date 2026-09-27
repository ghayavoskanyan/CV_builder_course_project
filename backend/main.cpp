#include "CVEngine.h"
#include "ApiController.h"
#include <iostream>
#include <string>

namespace {

void printUsage() {
    std::cout << "CV Builder\n"
              << "Usage:\n"
              << "  cv_builder              Run interactive CLI mode\n"
              << "  cv_builder --server     Start JSON API server on port 8080\n"
              << "  cv_builder --server PORT  Start JSON API server on custom port\n";
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc == 1) {
        CVEngine engine;
        engine.run();
        return 0;
    }

    std::string arg1 = argv[1];
    if (arg1 == "--help" || arg1 == "-h") {
        printUsage();
        return 0;
    }

    if (arg1 == "--server" || arg1 == "--api") {
        int port = 8080;
        if (argc >= 3) {
            try {
                port = std::stoi(argv[2]);
            } catch (const std::exception&) {
                std::cerr << "Invalid port number.\n";
                return 1;
            }
        }

        ApiController controller;
        controller.startServer(port);
        return 0;
    }

    if (arg1 == "--cli") {
        CVEngine engine;
        engine.run();
        return 0;
    }

    std::cerr << "Unknown option: " << arg1 << "\n";
    printUsage();
    return 1;
}
