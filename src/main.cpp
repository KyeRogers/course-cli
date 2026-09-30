#include <iostream>
#include <exception>
#include <string>
#include <vector>

#include "../include/manager.hpp"
#include "../include/ui.hpp"

namespace {

void PrintUsage() {
  std::cout << "Usage:\n"
            << "  course-cli                  Run the interactive menu\n"
            << "  course-cli today [-a]       Show today's tasks\n"
            << "  course-cli tomorrow [-a]    Show tomorrow's tasks\n"
            << "  course-cli week [-a]        Show the next seven days\n"
            << "  course-cli overdue [-a]     Show overdue tasks\n"
            << "  course-cli course NAME [-a] Show a course's tasks\n"
            << "  course-cli add_task NAME COURSE DATE [TIME]\n"
            << "  course-cli add_course NAME COLOUR\n"
            << "  course-cli done ID          Toggle task completion\n"
            << "  course-cli -h               Show this help\n"
            << "\nListings show pending tasks by default; -a includes "
               "completed tasks.\n"
            << "Stack listing selectors in any order, for example:\n"
            << "  course-cli today course Programming -a\n";
}

void PrintAssignments(const Manager& manager,
                      const std::vector<Assignment>& assignments) {
  if (assignments.empty()) {
    std::cout << "No matching assignments.\n";
    return;
  }
  for (const Assignment& assignment : assignments) {
    const Course* course = manager.GetCourseById(assignment.GetCourseId());
    if (course != nullptr) {
      std::cout << Colors::to_ansi(course->GetColour()) << course->GetName()
                << "\t";
    }
    assignment.print();
    std::cout << Colors::to_ansi(CourseColour::Default);
  }
}

bool IsListingSelector(const std::string& value) {
  return value == "today" || value == "tomorrow" || value == "week" ||
         value == "course";
}

int RunListings(Manager& manager, const int argc, const char* argv[]) {
  std::vector<Assignment> assignments = manager.GetAssignments();
  bool has_selector = false;
  bool include_completed = false;

  for (int index = 1; index < argc; ++index) {
    const std::string option{argv[index]};
    if (option == "-a") {
      include_completed = true;
    } else if (option == "today" || option == "tomorrow") {
      assignments = manager.FilterAssignmentsForDate(assignments, option);
      has_selector = true;
    } else if (option == "overdue" ) {
      assignments =
          manager.FilterAssignmentsInRange(assignments, "today", "week");
      has_selector = true;
    } else if (option == "course") {
      if (index + 1 >= argc || IsListingSelector(argv[index + 1]) ||
          std::string{argv[index + 1]} == "-a") {
        return -1;
      }
      assignments =
          manager.FilterAssignmentsForCourse(assignments, argv[++index]);
      has_selector = true;
    } else if (option == "overdue") {
        assignments = manager.FilterAssignmentsBeforeDate(assignments, "today");
        has_selector = true;
    } else {
      return -1;
    }
  }

  if (!has_selector) return -1;
  if (!include_completed) {
    assignments = manager.FilterAssignmentsByCompletion(
        assignments, CompletionFilter::Pending);
  }
  PrintAssignments(manager, assignments);
  return 0;
}

int RunCommand(Manager& manager, const int argc, const char* argv[]) {
  const std::string command{argv[1]};
  if (command == "-h" || command == "--help" || command == "help") {
    PrintUsage();
    return 0;
  }

  if (command == "add_task" && (argc == 5 || argc == 6)) {
    std::string date{argv[4]};
    const std::string time = argc == 6 ? argv[5] : "";
    return manager.AddTask(argv[2], argv[3], date, time) ? 0 : 1;
  }
  if (command == "add_course" && argc == 4) {
    manager.AddCourse(argv[2], Colors::from_string(argv[3]));
    return 0;
  }
  if (command == "done" && argc == 3) {
    try {
      std::size_t parsed = 0;
      const int id = std::stoi(argv[2], &parsed);
      if (parsed == std::string{argv[2]}.size()) {
        return manager.CompleteAssignmentById(id) ? 0 : 1;
      }
    } catch (const std::exception&) {
    }
  }

  const int listing_result = RunListings(manager, argc, argv);
  if (listing_result >= 0) return listing_result;

  std::cerr << "Invalid arguments.\n\n";
  PrintUsage();
  return 1;
}

}  // namespace

int main(const int argc, const char* argv[]) {
  Manager manager;
  const int result = argc > 1 ? RunCommand(manager, argc, argv) : 0;
  if (argc == 1) RunInteractiveMenu(manager);
  manager.SaveData("../data/data.json");
  return result;
}
