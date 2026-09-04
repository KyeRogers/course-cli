#include "../include/course.hpp"

Course::Course() : id_{9999999}, name_{"NULL"}, colour_{CourseColour::Default} {}

Course::Course(const int id, const std::string& name, const CourseColour colour) : id_{id}, name_{name}, colour_{colour} {}

int Course::GetId() const {
	return id_;
}

const std::string& Course::GetName() const {
	return name_;
}

CourseColour Course::GetColour() const {
	return colour_;
}