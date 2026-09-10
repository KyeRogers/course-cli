#include <iostream>
#include <string>

#include "../include/manager.hpp"

namespace {

void PrintUsage() {
  std::cout << "Usage:\n"
            << "  course-cli                 Run the interactive menu\n"
            << "  course-cli today           Show today's assignments\n"
            << "  course-cli tomorrow        Show tomorrow's assignments\n"
            << "  course-cli week            Show the next seven days\n"
            << "  course-cli course NAME     Show assignments for a course\n"
            << "  course-cli add_task NAME COURSE DATE [TIME]\n"
            << "  course-cli add_course NAME COLOUR\n"
            << "  course-cli done ID         Toggle an assignment's completion\n";
}

bool ParseId(const std::string& value, int& id) {
  try {
    std::size_t parsed_characters = 0;
    id = std::stoi(value, &parsed_characters);
    return parsed_characters == value.size();
  } catch (const std::exception&) {
    return false;
  }
}

int RunCommand(Manager& manager, const int argc, const char* argv[]) {
  const std::string command{argv[1]};
  if (command == "today") {
    manager.ShowAssignmentsForDate("today");
    return 0;
  }
  if (command == "tomorrow") {
    manager.ShowAssignmentsForDate("tomorrow");
    return 0;
  }
  if (command == "week") {
    manager.ShowAssignmentsInRange("today", "week");
    return 0;
  }
  if (command == "course" && argc == 3) {
    manager.ShowAssignmentsForCourse(argv[2]);
    return 0;
  }
  if (command == "add_task" && (argc == 5 || argc == 6)) {
    const std::string name{argv[2]};
    const std::string course{argv[3]};
    std::string date{argv[4]};
    const std::string time = argc == 6 ? argv[5] : "";
    return manager.AddTask(name, course, date, time) ? 0 : 1;
  }
  if (command == "add_course" && argc == 4) {
    const std::string name{argv[2]};
    const std::string colour{argv[3]};
    manager.AddCourse(name, Colors::from_string(colour));
    return 0;
  }
  if (command == "done" && argc == 3) {
    int id = 0;
    if (ParseId(argv[2], id)) {
      manager.CompleteAssignmentById(id);
      return 0;
    }
  }

  std::cerr << "Invalid arguments.\n\n";
  PrintUsage();
  return 1;
}

void RunInteractiveMenu(Manager& manager) {
  std::cout << " [0] to exit, [1] to add task, [2] to add course, [3] to show "
               "task list, [4] to show course list, [5] to toggle task "
               "completion status\n";
  int option;
  while (std::cin >> option) {
    if (option == 0) {
      std::cout << "Thank you for using course-cli\n";
      break;
    } else if (option == 1) {
      std::cout << "Task name: ";
      std::string name, due_date, course;
      std::cin >> name;
      std::cout << "\nCourse: ";
      std::cin >> course;
      std::cout << "\nDue by: ";
      std::cin >> due_date;
      std::cout << "\nDue time: ";
      std::string due_time;
      std::cin >> due_time;
      if (!manager.AddTask(name, course, due_date, due_time)) {
        std::cout << "Failed to add assignment\n";
      } else {
        std::cout << "\nTask added successfully\n";
      }
    } else if (option == 2) {
      std::string name, colour;
      std::cout << "Course: ";
      std::cin >> name;
      std::cout << "\nColour: ";
      std::cin >> colour;
      manager.AddCourse(name, Colors::from_string(colour));
      std::cout << "\nCourse added successfully\n";
    } else if (option == 3) {
      manager.ShowAssignments();
    } else if (option == 4) {
      manager.ShowCourses();
    } else if (option == 5) {
      std::cout << "Input task ID: ";
      int id;
      std::cin >> id;
      std::cout << "\n";
      manager.CompleteAssignmentById(id);
    }
    std::cout << " [0] to exit, [1] to add task, [2] to add course, [3] to "
                 "show task list, [4] to show course list, [5] to toggle task "
                 "completion status\n";
  }
}

}  // namespace

int main(const int argc, const char* argv[]) {
  Manager manager;
  const int result = argc > 1 ? RunCommand(manager, argc, argv) : 0;
  if (argc == 1) {
    RunInteractiveMenu(manager);
  }
  manager.SaveData("data/data.json");
  return result;
}
