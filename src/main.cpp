#include <iostream>
#include "../include/manager.hpp"

int main() {
    Manager manager;
    std::cout << " [0] to exit, [1] to add task, [2] to show task list\n";
    int option;
    while (std::cin >> option) {
        if (option == 0) {
            std::cout << "Thank you for using course-cli\n";
            break;
        } else if( option == 1) {
            std::cout << "Task name: ";
            std::string name{""}, due_date{""};
            std::cin >> name;
            std::cout << "\nDue by: ";
            std::cin >> due_date;
            std::cout << "\nTask added successfully\n";
            manager.AddTask(name, due_date);
        } else {
            manager.ShowTasks();
        }
        std::cout << " [0] to exit, [1] to add task, [2] to show task list\n";

    }
    return 0;
}