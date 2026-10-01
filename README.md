# CourseCLI

CourseCLI is a local terminal task manager for students. It combines quick command-line operations with an interactive FTXUI interface for managing courses, assignments, deadlines, completion state, and priorities.

## Current Features

* JSON persistence through `data/data.json`.
* Interactive screens for Dashboard, Tasks, Courses, Calendar, and Help.
* Dashboard sections for overdue, today, upcoming, and completed work.
* Pending/completed task toggling from the Dashboard, Tasks, Calendar, and course task views.
* Shared task form for adding and editing task name, course, date, time, and priority.
* Three task priorities displayed as symbols: `!` high, `-` normal, and `.` low.
* Weekly horizontal Calendar with selectable empty days and quick-add using `a`.
* Course-colored task output and course task previews.
* Cleanup of completed tasks older than a selected number of days or weeks.
* Human-readable dates in the TUI and CLI output.
* CLI listings show pending tasks by default; `-a` includes completed tasks.

## Interactive Menu

The application opens the TUI when run without arguments:

```sh
./build/course-cli
```

Main screens:

* **Dashboard:** See overdue, today, upcoming, and completed tasks with progress totals.
* **Tasks:** Review pending/completed tasks, add tasks, and clean up old completed tasks.
* **Courses:** Add/delete courses and open a course to see its tasks.
* **Calendar:** Move across days and weeks, inspect the selected day, and add tasks directly to it.
* **Help:** View the main keyboard controls and app capabilities.

Common controls include `Up/Down` for items, `Left/Right` for sections or days, `Enter` for opening or completing, `a` for adding, `Space` for confirming selectors, and `q` or `Esc` for going back.

## CLI Commands

Read-only listings show pending tasks unless `-a` is supplied:

```sh
./build/course-cli today
./build/course-cli today -a
./build/course-cli tomorrow
./build/course-cli week
./build/course-cli course "Programming" -a
```

Task and course operations:

```sh
./build/course-cli add_task "CLI project" "Programming" 2026-10-02 23:59
./build/course-cli add_course "Physics" Red
./build/course-cli done 12
./build/course-cli -h
```

## Data Model

Assignments contain a name, course, due date, optional due time, completion state, and priority. Existing data without a `priority` field remains compatible and is treated as normal priority.

## Source Layout

* `src/main.cpp` handles CLI arguments, command dispatch, and application startup.
* `include/ui.hpp` exposes the interactive UI entry point.
* `src/ui.cpp` owns screen rendering, keyboard interaction, formatting, and shared UI components such as task rows and the add/edit form.
* `src/manager.cpp` owns task queries and mutations, persistence, and composable vector-in/vector-out filters, including the Monday–Sunday calendar week for any supplied date.
* `src/assignment.cpp` and `src/course.cpp` own model data and behavior, independent of FTXUI presentation.

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

## Roadmap

Status markers: `[x]` implemented, `[~]` partially implemented, `[ ]` not yet implemented.

### Phase 1: Consistent UX

* [x] Use one keyboard convention and a context-sensitive footer on every screen.
* [x] Improve visual hierarchy with fewer boxes, clearer selected-row highlighting, whitespace, and course-color indicators.
* [x] Add contextual `?` help popups. The app includes a full Help screen and in-context help access.
* [x] Keep date formatting readable and consistent.

### Phase 2: Task Management

* [x] Add task filters: All, Today, Upcoming, Overdue, and Completed.
* [x] Add sorting by due date, priority, and course.
* [x] Add task editing for name, course, date, time, and priority.
* [x] Add task deletion and `/` search across task names and courses.
* [x] Expand CLI commands with `overdue`, `upcoming`, `search`, `edit`, `delete`, and `courses`.
* [x] Clarify the distinction between the current calendar week and the next seven days.

### Phase 3: Dashboard and Courses

* [x] Show Dashboard categories for today, overdue, upcoming, and completed tasks, with overall and date-range progress.

### Phase 4: Calendar

* [x] Use a horizontal weekly layout with a selectable day and that day's tasks.
* [x] Quick-add tasks with the selected date prefilled and edit task dates from the shared task form.
