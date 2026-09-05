#include "../include/manager.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <system_error>
Manager::Manager()
    : assignments_{std::vector<Assignment>(0)},
      courses_{std::vector<Course>(0)},
      last_assignment_id_{0},
      last_course_id_{0} {
  if (!LoadData("data/data.json")) {
    std::exit(0);
  }
}

// Todo: add task should be a bool to allow us to know whether the task was
// actually added successfully or not
void Manager::AddTask(const std::string& name, const int course_id,
                      const std::string& due_date) {
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
    std::perror("Error loading file to save data");
    return false;
  }

  nlohmann::json document;
  document["courses"] = nlohmann::json::array();
  for (const Course& course : courses_) {
    document["courses"].push_back(
        {{"id", course.GetId()},
         {"name", course.GetName()},
         {"colour", Colors::to_string(course.GetColour())}});
  }
  document["assignments"] = nlohmann::json::array();
  for (const Assignment& assignment : assignments_) {
    document["assignments"].push_back(
        {{"id", assignment.GetId()},
         {"course_id", assignment.GetCourseId()},
         {"name", assignment.GetName()},
         {"due_date", assignment.GetDueDate()},
         {"completed", assignment.GetCompleted()}});
  }

  output_file << document.dump(2) << '\n';

  return output_file.good();
}

// temporary fromat for testing
void Manager::ShowAssignments() const {
  std::cout << " TASK LIST\n#################";
  for (const Assignment assignment : assignments_) {
    assignment.print();
  }
  std::cout << "\n###############\n";
}

void Manager::ShowCourses() const {
  std::cout << " ### Courses ###\n";
  for (const Course course : courses_) {
    course.Print();
  }
  std::cout << "\n###############\n";
}

bool Manager::LoadData(const std::string& filename) {
  std::ifstream input_file(filename);
  if (!input_file) {
    std::perror("Error opening data file");
    return false;
  }

  input_file >> std::ws;

  if (input_file.peek() == std::ifstream::traits_type::eof()) {
    return true;
  }
  nlohmann::json document;
  input_file >> document;

  if (!document.contains("courses") || !document.contains("assignments")) {
    std::cout << "File: " << filename
              << " is empty or corrupted, overwrite?[y/n] ";
    char response;
    std::cin >> response;
    if (response == 'y') {
      return true;
    } else {
      std::perror("Cannot continue with corrupt file");
      return false;
    }
  }

  // load courses
  for (const auto& course_data : document.at("courses")) {
    Course course(
        course_data.at("id").get<int>(),
        course_data.at("name").get<std::string>(),
        Colors::from_string(course_data.at("colour").get<std::string>()));

    courses_.push_back(course);
  }
  // load assignments
  for (const auto& assignment_data : document.at("assignments")) {
    Assignment assignment(assignment_data.at("id").get<int>(),
                          assignment_data.at("course_id").get<int>(),
                          assignment_data.at("name").get<std::string>(),
                          assignment_data.at("due_date").get<std::string>(),
                          assignment_data.at("completed").get<bool>());
    assignments_.push_back(assignment);
  }
  if (courses_.size() >= 1) {
    last_course_id_ = courses_[courses_.size() - 1].GetId();
  }
  if (assignments_.size() >= 1) {
    last_assignment_id_ = assignments_[assignments_.size() - 1].GetId();
  }
  return true;
}