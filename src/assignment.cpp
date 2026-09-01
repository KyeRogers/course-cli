#include "../include/assignment.hpp"

#include <iostream>

Assignment::Assignment() : id_{9999999}, name_{"NULL"}, due_date_{"NULL"}, completed_{false} {}

Assignment::Assignment(int id, std::string name, std::string due_date) : id_{id}, name_{name}, due_date_{due_date}, completed_{false} {}

void Assignment::print() const {
    std::cout << name_ << std::endl;
    std::cout << "ID: " << id_ << "\n";
    std::cout << "DUE BY: " << due_date_ << std::endl; 
    std::cout << (completed_ ? "Complete" : "Incomplete") << std::endl;
}