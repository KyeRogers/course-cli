#include "../include/assignment.hpp"

#include <iostream>

Assignment::Assignment() : id_{9999999}, name_{"NULL"}, due_date_{"NULL"}, completed_{false} {}

Assignment::Assignment(const int id, const int course_id, const std::string& name, const std::string& due_date) : id_{id}, course_id_{course_id}, name_{name}, due_date_{due_date}, completed_{false} {}

// getters
int Assignment::GetId() const {
    return id_;
}

int Assignment::GetCourseId() const {
    return course_id_;
}

const std::string& Assignment::GetName() const {
    return name_;
}

const std::string& Assignment::GetDueDate() const {
    return due_date_;
}

const bool Assignment::GetCompleted() const {
    return completed_;
}

void Assignment::print() const {
    std::cout << name_ << std::endl;
    std::cout << "ID: " << id_ << "\n";
    std::cout << "DUE BY: " << due_date_ << std::endl; 
    std::cout << (completed_ ? "Complete" : "Incomplete") << std::endl;
}