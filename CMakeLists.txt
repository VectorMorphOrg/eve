cmake_minimum_required(VERSION 3.25)

project(eve VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(cmake/compiler_flags.cmake)

add_library(eve_core
    src/eve/core/identifiers.cpp
    src/eve/core/platform_request.cpp
    src/eve/core/platform_response.cpp
    src/eve/validation/validation_result.cpp
    src/eve/validation/validators.cpp
    src/eve/validation/parameter_validator.cpp
    src/eve/validation/policy_validator.cpp
    src/eve/validation/validation_engine.cpp
    src/eve/dispatcher/command_dispatcher.cpp
    src/eve/capability/capability_engine.cpp
    src/eve/capability/handlers/core_handlers.cpp
    src/eve/capability/handlers/documentation_handlers.cpp
    src/eve/services/configuration_service.cpp
    src/eve/services/logging_service.cpp
    src/eve/services/documentation_service.cpp
    src/eve/services/search_service.cpp
    src/eve/services/status_service.cpp
    src/eve/services/diagnostics_service.cpp
    src/eve/knowledge/knowledge_object.cpp
    src/eve/knowledge/in_memory_knowledge_store.cpp
    src/eve/knowledge/document_indexer.cpp
    src/eve/knowledge/repository_discovery.cpp
    src/eve/knowledge/markdown_parser.cpp
    src/eve/knowledge/metadata_extractor.cpp
    src/eve/knowledge/knowledge_object_builder.cpp
    src/eve/knowledge/relationship_builder.cpp
    src/eve/context/context_package.cpp
    src/eve/context/context_builder.cpp
    src/eve/ai/provider_manager.cpp
    src/eve/ai/null_provider.cpp
    src/eve/core/core_platform.cpp
    src/eve/core/platform_bootstrap.cpp
)

target_include_directories(eve_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(eve_core PUBLIC cxx_std_23)

if(MSVC)
    target_compile_options(eve_core PRIVATE /W4 /utf-8)
else()
    target_compile_options(eve_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_subdirectory(tests)
