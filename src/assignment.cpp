#include "../include/assignment.hpp"

#include <chrono>
#include <iostream>
#include <stdexcept>

namespace {

std::chrono::year_month_day Today() {
  const auto now = std::chrono::floor<std::chrono::days>(
      std::chrono::system_clock::now());
  return std::chrono::year_month_day{now};
}

}  // namespace

Assignment::Assignment()
    : id_{9999999},
      course_id_{9999999},
      name_{"NULL"},
      due_date_{"NULL"},
      due_time_{},
      completed_{false} {}

Assignment::Assignment(const int id, const int course_id,
                       const std::string& name, const std::string& due_date)
  : Assignment{id, course_id, name, due_date, std::string{}} {}

Assignment::Assignment(const int id, const int course_id,
                       const std::string& name, const std::string& due_date,
                       const std::string& due_time)
    : id_{id},
      course_id_{course_id},
      name_{name},
      due_date_{due_date},
      due_time_{due_time},
      completed_{false} {}

    Assignment::Assignment(const int id, const int course_id,
               const std::string& name, const std::string& due_date,
               const bool completed)
      : Assignment{id, course_id, name, due_date, std::string{}, completed} {}

Assignment::Assignment(const int id, const int course_id,
                       const std::string& name, const std::string& due_date,
                       const std::string& due_time, const bool completed)
    : id_{id},
      course_id_{course_id},
      name_{name},
      due_date_{due_date},
      due_time_{due_time},
      completed_{completed} {}

// getters
int Assignment::GetId() const { return id_; }
int Assignment::GetCourseId() const { return course_id_; }
const std::string& Assignment::GetName() const { return name_; }
const std::string& Assignment::GetDueDate() const { return due_date_; }
const std::string& Assignment::GetDueTime() const { return due_time_; }
const bool Assignment::GetCompleted() const { return completed_; }

void Assignment::print() const {
  std::cout << "[" << id_ << "]: " << name_ << " due by: " << due_date_
            << (completed_ ? " COMPLETED" : " PENDING") << std::endl;
}

void Assignment::ToggleCompleted() { completed_ = !completed_; }

std::chrono::year_month_day Assignment::GetDateKey() const {
  if (due_date_.size() != 10 || due_date_[4] != '-' || due_date_[7] != '-') {
    throw std::runtime_error("Invalid date format, expected YYYY-MM-DD");
  }

  int year = std::stoi(due_date_.substr(0, 4));
  unsigned month = static_cast<unsigned>(std::stoul(due_date_.substr(5, 2)));
  unsigned day = static_cast<unsigned>(std::stoul(due_date_.substr(8, 2)));

  std::chrono::year_month_day year_month_day{std::chrono::year{year}, std::chrono::month{month},
                                  std::chrono::day{day}};

  if (!year_month_day.ok()) {
    throw std::runtime_error("Invalid calendar date");
  }

  return year_month_day;
}

bool Assignment::IsDueToday() const { return GetDateKey() == Today(); }

bool Assignment::IsDueWithinDays(const int days) const {
  if (days < 0) {
    throw std::invalid_argument("The number of days cannot be negative");
  }

  const auto start = std::chrono::sys_days{Today()};
  const auto end = start + std::chrono::days{days};
  const auto due = std::chrono::sys_days{GetDateKey()};
  return due >= start && due <= end;
}