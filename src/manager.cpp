#include "../include/manager.hpp"

#include <algorithm>
#include <cctype>
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

std::chrono::year_month_day ParseDate(const std::string& input) {
  const std::string date = ResolveDate(input);
  if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
    throw std::invalid_argument("Expected date in YYYY-MM-DD format");
  }
  const std::chrono::year_month_day parsed{
      std::chrono::year{std::stoi(date.substr(0, 4))},
      std::chrono::month{static_cast<unsigned>(std::stoi(date.substr(5, 2)))},
      std::chrono::day{static_cast<unsigned>(std::stoi(date.substr(8, 2)))} };
  if (!parsed.ok()) {
    throw std::invalid_argument("Invalid calendar date");
  }
  return parsed;
}

}  // namespace

Manager::Manager()
    : assignments_{std::vector<Assignment>(0)},
      courses_{std::vector<Course>(0)},
      last_assignment_id_{0},
      last_course_id_{0} {
  if (!LoadData("../data/data.json")) {
    std::exit(0);
  }
}

void Manager::CaptureState() {
  undo_stack_.push_back({assignments_, courses_, last_assignment_id_, last_course_id_});
}

bool Manager::UndoLastAction() {
  if (undo_stack_.empty()) {
    return false;
  }

  const Snapshot snapshot = undo_stack_.back();
  undo_stack_.pop_back();
  assignments_ = snapshot.assignments;
  courses_ = snapshot.courses;
  last_assignment_id_ = snapshot.last_assignment_id;
  last_course_id_ = snapshot.last_course_id;
  return true;
}

bool Manager::AddTask(const std::string& name, const std::string& course,
                      std::string& due_date, const std::string& due_time,
                      const AssignmentPriority priority,
                      const Recurrence recurrence) {
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

    CaptureState();
    Assignment new_assignment(last_assignment_id_ + 1, course_id, name,
                              due_date, due_time);
    new_assignment.SetDetails(name, course_id, due_date, due_time, priority, recurrence);
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
    std::perror("Error loading file to s  data");
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
           {"priority", assignment.GetPriority() == AssignmentPriority::High
                ? "high"
                : assignment.GetPriority() == AssignmentPriority::Low
                  ? "low"
                  : "normal"},
         {"completed", assignment.GetCompleted()}});
  }

  output_file << document.dump(2) << '\n';

  return output_file.good();
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

const std::vector<Assignment>& Manager::GetAssignments() const {
  return assignments_;
}

std::vector<Assignment> Manager::FilterAssignmentsByCompletion(
    const std::vector<Assignment>& source,
    const CompletionFilter completion) const {
  std::vector<Assignment> filtered;
  for (const Assignment& assignment : source) {
    if (completion == CompletionFilter::All ||
        (completion == CompletionFilter::Pending && !assignment.GetCompleted()) ||
        (completion == CompletionFilter::Completed && assignment.GetCompleted())) {
      filtered.push_back(assignment);
    }
  }
  return filtered;
}

std::vector<Assignment> Manager::FilterAssignmentsForDate(
    const std::vector<Assignment>& source, const std::string& date) const {
  const std::string resolved_date = ResolveDate(date);
  std::vector<Assignment> filtered;
  for (const Assignment& assignment : source) {
    if (assignment.GetDueDate() == resolved_date) filtered.push_back(assignment);
  }
  return filtered;
}

std::vector<Assignment> Manager::FilterAssignmentsInRange(
    const std::vector<Assignment>& source, const std::string& start_date,
    const std::string& end_date) const {
  const std::string resolved_start = ResolveDate(start_date);
  std::string resolved_end = ResolveDate(end_date);
  if (end_date == "week") {
    const auto end = std::chrono::sys_days{Today()} + std::chrono::days{7};
    resolved_end = FormatDate(std::chrono::year_month_day{end});
  }

  std::vector<Assignment> filtered;
  for (const Assignment& assignment : source) {
    if (assignment.GetDueDate() >= resolved_start &&
        assignment.GetDueDate() <= resolved_end) {
      filtered.push_back(assignment);
    }
  }
  return filtered;
}

std::vector<Assignment> Manager::FilterAssignmentsBeforeDate(
    const std::vector<Assignment>& source, const std::string& date) const {
  const std::string resolved_date = ResolveDate(date);
  std::vector<Assignment> filtered;
  for (const Assignment& assignment : source) {
    if (assignment.GetDueDate() < resolved_date) filtered.push_back(assignment);
  }
  return filtered;
}

std::vector<Assignment> Manager::FilterAssignmentsAfterDate(
    const std::vector<Assignment>& source, const std::string& date) const {
  const std::string resolved_date = ResolveDate(date);
  std::vector<Assignment> filtered;
  for (const Assignment& assignment : source) {
    if (assignment.GetDueDate() > resolved_date) filtered.push_back(assignment);
  }
  return filtered;
}

std::vector<Assignment> Manager::FilterAssignmentsForCourse(
    const std::vector<Assignment>& source, const std::string& course) const {
  const int course_id = GetCourseIdByName(course);
  std::vector<Assignment> filtered;
  if (course_id == 0) return filtered;
  for (const Assignment& assignment : source) {
    if (assignment.GetCourseId() == course_id) filtered.push_back(assignment);
  }
  return filtered;
}

std::vector<Assignment> Manager::SearchAssignments(
    const std::vector<Assignment>& source, const std::string& query) const {
  if (query.empty()) return source;
  std::string needle = query;
  std::transform(needle.begin(), needle.end(), needle.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });

  std::vector<Assignment> matches;
  for (const Assignment& assignment : source) {
    const Course* course = GetCourseById(assignment.GetCourseId());
    std::string task_name = assignment.GetName();
    std::string course_name = course == nullptr ? "" : course->GetName();
    std::transform(task_name.begin(), task_name.end(), task_name.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    std::transform(course_name.begin(), course_name.end(), course_name.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    if (task_name.find(needle) != std::string::npos ||
        course_name.find(needle) != std::string::npos) {
      matches.push_back(assignment);
    }
  }
  return matches;
}

std::vector<Assignment> Manager::SortAssignments(
    const std::vector<Assignment>& source,
    const AssignmentSort sort) const {
  std::vector<Assignment> sorted = source;
  std::sort(sorted.begin(), sorted.end(), [&](const Assignment& left,
                                             const Assignment& right) {
    switch (sort) {
      case AssignmentSort::DueDate:
        return left.GetDueDate() < right.GetDueDate();
      case AssignmentSort::Priority: {
        const auto left_pri = static_cast<int>(left.GetPriority());
        const auto right_pri = static_cast<int>(right.GetPriority());
        if (left_pri != right_pri) return left_pri > right_pri;
        return left.GetDueDate() < right.GetDueDate();
      }
      case AssignmentSort::Course: {
        const Course* left_course = GetCourseById(left.GetCourseId());
        const Course* right_course = GetCourseById(right.GetCourseId());
        const std::string left_name = left_course == nullptr ? "" : left_course->GetName();
        const std::string right_name = right_course == nullptr ? "" : right_course->GetName();
        if (left_name != right_name) return left_name < right_name;
        return left.GetDueDate() < right.GetDueDate();
      }
      case AssignmentSort::Name:
      default:
        return left.GetName() < right.GetName();
    }
  });
  return sorted;
}

CalendarWeek Manager::GetCalendarWeek(
    const std::vector<Assignment>& source, const std::string& date) const {
  const auto selected_day = std::chrono::sys_days{ParseDate(date)};
  const auto weekday = std::chrono::weekday{selected_day};
  const auto monday = selected_day -
              std::chrono::days{
                static_cast<int>(weekday.iso_encoding()) - 1};
  const auto sunday = monday + std::chrono::days{6};

  CalendarWeek week;
  week.monday = FormatDate(std::chrono::year_month_day{monday});
  week.sunday = FormatDate(std::chrono::year_month_day{sunday});
  week.previous_monday =
      FormatDate(std::chrono::year_month_day{monday - std::chrono::days{7}});
  week.next_monday =
      FormatDate(std::chrono::year_month_day{monday + std::chrono::days{7}});

  for (std::size_t day = 0; day < week.days.size(); ++day) {
    const auto date_point =
        monday + std::chrono::days{static_cast<int>(day)};
    week.days[day].date =
        FormatDate(std::chrono::year_month_day{date_point});
  }

  for (const Assignment& assignment : source) {
    try {
      const auto due = std::chrono::sys_days{assignment.GetDateKey()};
      if (due < monday || due > sunday) continue;
      const auto day_index = static_cast<std::size_t>((due - monday).count());
      week.days[day_index].assignments.push_back(assignment);
    } catch (const std::exception&) {
      // Ignore malformed stored dates rather than failing the entire week view.
    }
  }
  return week;
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

bool Manager::DeleteAssignmentById(const int id) {
  const auto assignment = std::remove_if(assignments_.begin(), assignments_.end(),
                                         [id](const Assignment& task) {
                                           return task.GetId() == id;
                                         });
  if (assignment == assignments_.end()) {
    return false;
  }
  assignments_.erase(assignment, assignments_.end());
  return true;
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

bool Manager::UpdateAssignment(const int id, const std::string& name,
                               const std::string& course,
                               const std::string& due_date,
                               const std::string& due_time,
                               const AssignmentPriority priority,
                               const Recurrence recurrence) {
  Assignment* assignment = GetAssignmentById(id);
  const int course_id = GetCourseIdByName(course);
  if (assignment == nullptr || course_id == 0 || name.empty() || due_date.empty()) {
    return false;
  }

  CaptureState();
  assignment->SetDetails(name, course_id, due_date, due_time, priority, recurrence);
  return true;
}

int Manager::RemoveCompletedOlderThanDays(const int days) {
  if (days < 0) {
    return 0;
  }

  const auto today = std::chrono::floor<std::chrono::days>(
      std::chrono::system_clock::now());
  const auto cutoff = std::chrono::sys_days{today} - std::chrono::days{days};
  const auto previous_size = assignments_.size();

  assignments_.erase(
      std::remove_if(assignments_.begin(), assignments_.end(),
                     [&](const Assignment& assignment) {
                       if (!assignment.GetCompleted()) {
                         return false;
                       }

                       try {
                         return std::chrono::sys_days{assignment.GetDateKey()} <=
                                cutoff;
                       } catch (const std::exception&) {
                         return false;
                       }
                     }),
      assignments_.end());

  return static_cast<int>(previous_size - assignments_.size());
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
    if (assignment_data.contains("priority")) {
      const std::string priority = assignment_data.at("priority").get<std::string>();
      assignment.SetDetails(assignment.GetName(), assignment.GetCourseId(),
                            assignment.GetDueDate(), assignment.GetDueTime(),
                            priority == "high" ? AssignmentPriority::High
                            : priority == "low" ? AssignmentPriority::Low
                                                 : AssignmentPriority::Normal);
    }
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

const std::vector<Course>& Manager::GetCourses() const {
  return courses_;
}

void Manager::DeleteCourse(const int course_id) {
  const auto course = std::find_if(
        courses_.begin(), courses_.end(), [course_id](const Course& course) {
        return course.GetId() == course_id;
      });
  if (course == courses_.end()) {
    return;
  }

  courses_.erase(course);
  assignments_.erase(
      std::remove_if(assignments_.begin(), assignments_.end(),
                     [course_id](const Assignment& assignment) {
                       return assignment.GetCourseId() == course_id;
                     }),
      assignments_.end());
}