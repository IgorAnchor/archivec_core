#include <iostream>

#include "src/filemanager.hpp"

int main() {

    auto bytes = FileManager::open_file("1.txt");

    FileManager::write_file(bytes, "copy.txt");

    return EXIT_SUCCESS;
}