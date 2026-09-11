#include "../include/manager.hpp"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <system_error>
#include <format>

namespace {

std::chrono::year_month_day Today() {
  const auto now =
      std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
  return std::chrono::year_month_day{now};
}

std::string FormatDate(const std::chrono::year_month_day date) {
  const auto year = static_cast<int>(date.year());
  const auto month = static_cast<unsigned>(date.month());
  const auto day = static_cast<unsigned>(date.day());
  return std::format("{:04}-{:02}-{:02}", year, month, day);
}

std::string ResolveDate(const std::string& date) {
  const auto today = std::chrono::sys_days{Today()};
  if (date == "today") {
    return FormatDate(Today());
  }
  if (date == "tomorrow") {
    return FormatDate(
        std::chrono::year_month_day{today + std::chrono::days{1}});
  }
  return date;
}

}  // namespace

Manager::Manager()
    : assignments_{std::vector<Assignment>(0)},
      courses_{std::vector<Course>(0)},
      last_assignment_id_{0},
      last_course_id_{0} {
  if (!LoadData("data/data.json")) {
    std::exit(0);
  }
}

bool Manager::AddTask(const std::string& name, const std::string& course,
                      std::string& due_date, const std::string& due_time) {
  int course_id = GetCourseIdByName(course);
  if (course_id == 0) {
    std::cerr << name << " doesnt exist\n";
    return false;
  } else {
    if (due_date == "today" || due_date == "Today") {
      due_date = FormatDate(Today());
    } else if (due_date == "tomorrow" || due_date == "Tomorrow") {
      const auto tomorrow = std::chrono::sys_days{Today()} + std::chrono::days{1};
      due_date = FormatDate(std::chrono::year_month_day{tomorrow});
    }

    Assignment new_assignment(last_assignment_id_ + 1, course_id, name,
                              due_date, due_time);
    assignments_.push_back(new_assignment);
    last_assignment_id_++;
    return true;
  }
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
         {"due_time", assignment.GetDueTime()},
         {"completed", assignment.GetCompleted()}});
  }

  output_file << document.dump(2) << '\n';

  return output_file.good();
}

// temporary fromat for testing
void Manager::ShowAssignments() const {
  std::cout << "### TASK LIST ###\n";
  for (const Assignment assignment : assignments_) {
    const Course* temp = GetCourseById(assignment.GetCourseId());
    if (temp == nullptr) {
      std::cout << "Course with id: " << assignment.GetCourseId()
                << " doesnt exist\n";
    } else {
      std::cout << Colors::to_ansi(temp->GetColour());
      std::cout << temp->GetName() << "\t";
      assignment.print();
      std::cout << Colors::to_ansi(CourseColour::Default);
    }
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

// return id that matches course name. Return 0 when course doesnt exist
int Manager::GetCourseIdByName(const std::string& name) const {
  for (const Course& course : courses_) {
    if (name == course.GetName()) {
      return course.GetId();
    }
  }

  return 0;
}

const Course* Manager::GetCourseById(const int id) const {
  for (const Course& course : courses_) {
    if (course.GetId() == id) {
      return &course;
    }
  }
  return nullptr;
}

Assignment* Manager::GetAssignmentById(const int id) {
  for (Assignment& assignment : assignments_) {
    if (assignment.GetId() == id) {
      return &assignment;
    }
  }
  return nullptr;
}

const Assignment* Manager::GetAssignmentById(const int id) const {
  for (const Assignment& assignment : assignments_) {
    if (assignment.GetId() == id) {
      return &assignment;
    }
  }
  return nullptr;
}

bool Manager::CompleteAssignmentById(const int id) {
  Assignment* assignment = GetAssignmentById(id);
  if (assignment == nullptr) {
    std::cerr << "Assignment with id: " << id << " doesnt exist\n";
    return false;
  } else {
    assignment->ToggleCompleted();
    return true;
  }
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
                          assignment_data.at("due_time").get<std::string>(),
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

void Manager::ShowAssignmentsForDate(const std::string& date) const {
  const std::string resolved_date = ResolveDate(date);
  std::cout << "### TASK LIST FOR " << resolved_date << " ###\n";
  for (const Assignment assignment : assignments_) {
    if (assignment.GetDueDate() == resolved_date) {
      const Course* temp = GetCourseById(assignment.GetCourseId());
      if (temp == nullptr) {
        std::cout << "Course with id: " << assignment.GetCourseId()
                  << " doesnt exist\n";
      } else {
        std::cout << Colors::to_ansi(temp->GetColour());
        std::cout << temp->GetName() << "\t";
        assignment.print();
        std::cout << Colors::to_ansi(CourseColour::Default);
      }
    }
  }
  std::cout << "\n###############\n";
}

void Manager::ShowAssignmentsForCourse(const std::string& course) const {
  std::cout << "### TASK LIST FOR COURSE: " << course << " ###\n";
  int course_id = GetCourseIdByName(course);
  if (course_id == 0) {
    std::cerr << course << " doesnt exist\n";
    return;
  }
  for (const Assignment assignment : assignments_) {
    if (assignment.GetCourseId() == course_id) {
      const Course* temp = GetCourseById(assignment.GetCourseId());
      if (temp == nullptr) {
        std::cout << "Course with id: " << assignment.GetCourseId()
                  << " doesnt exist\n";
      } else {
        std::cout << Colors::to_ansi(temp->GetColour());
        std::cout << temp->GetName() << "\t";
        assignment.print();
        std::cout << Colors::to_ansi(CourseColour::Default);
      }
    }
  }
  std::cout << "\n###############\n";
}

void Manager::ShowAssignmentsInRange(const std::string& start_date,
                                     const std::string& end_date) const {
  const std::string resolved_start = ResolveDate(start_date);
  std::string resolved_end = ResolveDate(end_date);
  if (end_date == "week") {
    const auto week_end = std::chrono::sys_days{Today()} + std::chrono::days{7};
    resolved_end = FormatDate(std::chrono::year_month_day{week_end});
  }

  std::cout << "### TASK LIST FOR RANGE: " << resolved_start << " to "
            << resolved_end << " ###\n";
  for (const Assignment assignment : assignments_) {
    if (assignment.GetDueDate() >= resolved_start &&
        assignment.GetDueDate() <= resolved_end) {
      const Course* temp = GetCourseById(assignment.GetCourseId());
      if (temp == nullptr) {
        std::cout << "Course with id: " << assignment.GetCourseId()
                  << " doesnt exist\n";
      } else {
        std::cout << Colors::to_ansi(temp->GetColour());
        std::cout << temp->GetName() << "\t";
        assignment.print();
        std::cout << Colors::to_ansi(CourseColour::Default);
      }
    }
  }
  std::cout << "\n###############\n";
}

// TODO: add a helper to group tasks by day/week using std::chrono date keys.
// TODO: add a function that returns assignments within [start, end] for: today,
// tomorrow, next 7 days.
// TODO: add date-range rendering such as ShowCalendarForDays(7) and
// ShowTasksForDay(date).
// TODO: expose a quick CLI path without the menu, e.g. "course-cli today" or
// "course-cli week".