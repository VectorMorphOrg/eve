#include "eve/services/service_implementations.hpp"

#include <format>
#include <iostream>

namespace eve::services {

void LoggingService::info(std::string_view message) {
    std::cout << std::format("[INFO] {}\n", message);
}

void LoggingService::warn(std::string_view message) {
    std::cout << std::format("[WARN] {}\n", message);
}

void LoggingService::error(std::string_view message) {
    std::cerr << std::format("[ERROR] {}\n", message);
}

}  // namespace eve::services
