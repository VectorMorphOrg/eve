cmake_minimum_required(VERSION 3.25)

find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(eve_tests
    platform_request_test.cpp
    platform_response_test.cpp
    knowledge_store_test.cpp
    context_package_test.cpp
    capability_engine_test.cpp
    integration_test.cpp
)

target_link_libraries(eve_tests
    PRIVATE
        eve_core
        GTest::gtest_main
)

target_compile_definitions(eve_tests PRIVATE EVE_PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

include(GoogleTest)
gtest_discover_tests(eve_tests)
