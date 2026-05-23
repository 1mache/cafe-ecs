#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "config.hpp"
#include "foo.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

int main(int argc, char **argv)
{
    std::cout << "JSON: " << NLOHMANN_JSON_VERSION_MAJOR << "."
              << NLOHMANN_JSON_VERSION_MINOR << "."
              << NLOHMANN_JSON_VERSION_PATCH << '\n';
    std::cout << "CXXOPTS: " << CXXOPTS__VERSION_MAJOR << "."
              << CXXOPTS__VERSION_MINOR << "." << CXXOPTS__VERSION_PATCH
              << '\n';
    std::cout << "SPDLOG: " << SPDLOG_VER_MAJOR << "." << SPDLOG_VER_MINOR
              << "." << SPDLOG_VER_PATCH << '\n';
    std::cout << "\n\nUsage Example:\n";

    // Compiler Warning and clang tidy error
    // std::int32_t i = 0;

    // Adress Sanitizer should see this
    // char x[10];
    // x[11] = 1;

    const auto welcome_message = "Welcome to " + std::string{project_name} +
                                 " v" + std::string{project_version} + "\n";
    spdlog::info(welcome_message);

    cxxopts::Options options(project_name.data(), welcome_message);

    options.add_options("arguments")("h,help", "Print usage")(
        "f,filename",
        "File name",
        cxxopts::value<std::string>())(
        "v,verbose",
        "Verbose output",
        cxxopts::value<bool>()->default_value("false"));

    auto result = options.parse(argc, argv);

    if (argc == 1 || result.count("help"))
    {
        std::cout << options.help() << '\n';
        return 0;
    }

    auto filename = std::string{};
    auto verbose = false;

    if (result.count("filename"))
    {
        filename = result["filename"].as<std::string>();
    }
    else
    {
        return 1;
    }

    verbose = result["verbose"].as<bool>();

    if (verbose)
    {
        std::cout << "Opening file: " << filename << '\n';
    }

    auto ifs = std::ifstream{filename};

    if (!ifs.is_open())
    {
        return 1;
    }

    const auto parsed_data = json::parse(ifs);

    if (verbose)
    {
        const auto name = parsed_data["name"];
        std::cout << "Name: " << name << '\n';
    }

    return 0;
}
