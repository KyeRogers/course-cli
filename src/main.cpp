#include <iostream>

#include "../include/manager.hpp"

int main() {
  Manager manager;
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
      std::string name{""}, due_date{""}, course{""};
      std::cin >> name;
      std::cout << "\nCourse: ";
      std::cin >> course;
      std::cout << "\nDue by: ";
      std::cin >> due_date;
      if (!manager.AddTask(name, course, due_date)) {
        std::cout << "Failed to add assignment\n";
      } else {
        std::cout << "\nTask added successfully\n";
      }
    } else if (option == 2) {
      std::string name{""}, colour{""};
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
  manager.SaveData("data/data.json");
  return 0;
}