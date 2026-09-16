# CourseCLI

CourseCLI is a command-line application designed to help students track assignments, calculate urgency, and manage academic workloads.

## Goal

The goal of this project is to build a fast, terminal-based tool that keeps track of coursework and helps prioritize tasks based on deadlines, effort, and assignment weight.

## Core Requirements

* **Command-Line Entry:** Allow quick addition, listing, and completion of assignments via terminal commands.
* **Terminal User Interface (TUI):** Provide an interactive visual dashboard in the terminal to view and manage tasks.
* **Local Data Storage:** Save and load task data locally on the user's machine (using JSON).

## Build and Test

Configure and build the project with CMake:

```sh
cmake -S . -B build
cmake --build build
```

Run the complete test suite with CTest:

```sh
ctest --test-dir build --output-on-failure
```

CMake finds installed `nlohmann_json` and Doctest packages when available. Otherwise,
it downloads the pinned versions during configuration.

## Planned Features

* Shorthand syntax for adding tasks.
* Visual crunch-week indicators for high-workload windows.
* Custom filtering by course.