#include "../include/manager.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

Manager::Manager() : assignments_{std::vector<Assignment>(0)}, courses_{std::vector<Course>(0)}, last_assignment_id_{0}, last_course_id_{0} {}

// Todo: add task should be a bool to allow us to know whether the task was actually added successfully or not  
void Manager::AddTask(const std::string& name, const int course_id, const std::string& due_date) {
    Assignment new_assignment(last_assignment_id_ + 1, course_id, name, due_date);
    assignments_.push_back(new_assignment);
    last_assignment_id_++;
}

void Manager::AddCourse(const std::string& name, const CourseColour colour) {
    Course new_course(last_course_id_ + 1, name, colour);
    courses_.push_back(new_course);
    last_course_id_++;
}

bool Manager::SaveData(const std::string& filename) const {
    std::ofstream output_file(filename);
    if (!output_file) {
        return false;
    }

    nlohmann::json document;
    document["courses"] = nlohmann::json::array();
    for (const Course& course : courses_) {
        document["courses"].push_back({
            {"id", course.GetId()},
            {"name", course.GetName()},
            {"colour", Colors::to_string(course.GetColour())}
        });
    }
    document["assignments"] = nlohmann::json::array();
    for (const Assignment& assignment : assignments_) {
        document["assignments"].push_back({
            {"id", assignment.GetId()},
            {"course_id", assignment.GetCourseId()},
            {"name", assignment.GetName()},
            {"due_date", assignment.GetDueDate()},
            {"completed", assignment.GetCompleted()}
        });
    }

    output_file << document.dump(2) << '\n';

    return output_file.good();
}

// temporary fromat for testing
void Manager::ShowAssignments() const {
    std::cout << " TASK LIST\n#################";
    for (Assignment assignment : assignments_) {
        assignment.print();
    }
    std::cout << "\n###############\n";
}