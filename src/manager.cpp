#include "../include/manager.hpp"

#include <iostream>

Manager::Manager() : tasks_{std::vector<Assignment>(0)}, last_id_{0} {}

// Todo: add task should be a bool to allow us to know whether the task was actually added successfully or not  
void Manager::AddTask(std::string name, std::string due_date) {
    Assignment new_assignment(last_id_ + 1, name, due_date);
    tasks_.push_back(new_assignment);
    last_id_++;
}

// temporary fromat for testing
void Manager::ShowTasks() const {
    std::cout << " TASK LIST\n#################";
    for (Assignment task : tasks_) {
        task.print();
    }
    std::cout << "\n###############\n";
}