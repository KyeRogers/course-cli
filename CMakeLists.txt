cmake_minimum_required(VERSION 3.20)

project(course-cli VERSION 1.0 LANGUAGES CXX)

include(CTest)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# nlohmann_json
find_package(nlohmann_json 3.11 QUIET)
if(NOT TARGET nlohmann_json::nlohmann_json)
  FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

# FTXUI
find_package(ftxui QUIET)
if(NOT TARGET ftxui::component)
  FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v7.0.3
  )
  FetchContent_MakeAvailable(ftxui)
endif()

add_executable(course-cli
  src/main.cpp
  src/assignment.cpp
  src/course.cpp
  src/manager.cpp
)

target_include_directories(course-cli PRIVATE include)
target_link_libraries(course-cli
  PRIVATE
    nlohmann_json::nlohmann_json
    ftxui::screen
    ftxui::dom
    ftxui::component
)

add_executable(ftxui-demo src/ftxui_demo.cpp)
target_link_libraries(ftxui-demo
  PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
)

if(BUILD_TESTING)
  find_package(doctest 2.4 QUIET)
  if(NOT TARGET doctest::doctest)
    FetchContent_Declare(
      doctest
      URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz
    )
    FetchContent_MakeAvailable(doctest)
  endif()

  add_executable(assignment-tests
    tests/assignment_doctest.cpp
    src/assignment.cpp
  )
  target_include_directories(assignment-tests PRIVATE include)
  target_link_libraries(assignment-tests PRIVATE doctest::doctest)

  add_executable(manager-tests
    tests/manager_doctest.cpp
    src/manager.cpp
    src/assignment.cpp
    src/course.cpp
  )
  target_include_directories(manager-tests PRIVATE include)
  target_link_libraries(manager-tests PRIVATE nlohmann_json::nlohmann_json doctest::doctest)

  add_test(NAME assignment-tests COMMAND assignment-tests)
  add_test(NAME manager-tests COMMAND manager-tests)

  add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS assignment-tests manager-tests
    USES_TERMINAL
  )
endif()