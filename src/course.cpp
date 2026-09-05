#include "../include/course.hpp"

#include <iostream>


Course::Course()
    : id_{9999999}, name_{"NULL"}, colour_{CourseColour::Default} {}

Course::Course(const int id, const std::string& name, const CourseColour colour)
    : id_{id}, name_{name}, colour_{colour} {}

int Course::GetId() const { return id_; }

const std::string& Course::GetName() const { return name_; }

CourseColour Course::GetColour() const { return colour_; }

void Course::Print() const {
  std::cout << Colors::to_ansi(colour_) << "[" << id_ << "]: " << name_
            << Colors::to_ansi(CourseColour::Default) << std::endl;
}