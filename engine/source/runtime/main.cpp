#include "engine.hpp"

#include <iostream>
#include <stdexcept>
#include <cstdlib>

int main()
{
    Hello_Triangle_Application app{};

    try {
        app.run();
    } catch (const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
