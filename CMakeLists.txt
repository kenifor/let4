cmake_minimum_required(VERSION 3.5)

project(let_4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(ENABLE_UNIT_TESTS ON)

set(HEADERS_DIR Headers)
set(SOURCES_DIR Sources)

set(FILES
    ${HEADERS_DIR}/trigrammdictionary.h
    ${SOURCES_DIR}/trigrammdictionary.cpp
)

find_package(Threads REQUIRED)

include_directories(${HEADERS_DIR})

add_executable(${PROJECT_NAME} ${FILES} ${SOURCES_DIR}/main.cpp)

target_link_libraries(${PROJECT_NAME}
    Threads::Threads
)

if (ENABLE_UNIT_TESTS)

    include(FetchContent)
    include(GoogleTest)
    enable_testing()

    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)

    set(TEST_SOURCES
        Tests/test.cpp
        ${FILES}
    )

    set(TEST_PROJECT_NAME let4-test)

    find_package(Threads REQUIRED)

    add_executable(${TEST_PROJECT_NAME} ${TEST_SOURCES})

    target_link_libraries(${TEST_PROJECT_NAME}
    PRIVATE gtest_main
    Threads::Threads
    )

endif()
