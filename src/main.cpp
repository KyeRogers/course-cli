#include <iostream>
#include "../include/manager.hpp"

int main() {
    Manager manager;
    std::cout << " [0] to exit, [1] to add task, [2] to add course, [3] to show task list\n";
    int option;
    while (std::cin >> option) {
        if (option == 0) {
            std::cout << "Thank you for using course-cli\n";
            break;
        } else if(option == 1) {
            std::cout << "Task name: ";
            std::string name{""}, due_date{""}, course{""};
            std::cin >> name;
            std::cout << "\nCourse: ";
            std::cin >> course;
            std::cout << "\nDue by: ";
            std::cin >> due_date;
            std::cout << "\nTask added successfully\n";
            manager.AddTask(name, 0, due_date);
        } else if (option == 2) {
            std::string name{""}, colour{""};
            std::cout << "Course: ";
            std::cin >> name;
            std::cout << "\nColour: ";
            std::cin >> colour;           
            std::cout << "\nCourse added successfully\n";
            manager.AddCourse(name, CourseColour::BrightBlue);
        } else {
            manager.ShowAssignments();
        }
         std::cout << " [0] to exit, [1] to add task, [2] to add course, [3] to show task list\n";
    }
    manager.SaveData("data/data.json");
    return 0;
}