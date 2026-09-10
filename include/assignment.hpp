#ifndef ASSIGNMENT_HPP
#define ASSIGNMENT_HPP

#include <string>
#include <chrono>

class Assignment {
 public:
  Assignment();
  Assignment(const int id, const int course_id, const std::string& name,
             const std::string& due_date);
  Assignment(const int id, const int course_id, const std::string& name,
             const std::string& due_date, const std::string& due_time);
  Assignment(const int id, const int course_id, const std::string& name,
             const std::string& due_date, const bool completed);
  Assignment(const int id, const int course_id, const std::string& name,
             const std::string& due_date, const std::string& due_time,
             const bool completed);

  int GetId() const;
  int GetCourseId() const;
  const std::string& GetName() const;
  const std::string& GetDueDate() const;
  const std::string& GetDueTime() const;
  const bool GetCompleted() const;

  std::chrono::year_month_day GetDateKey() const;
  bool IsDueToday() const;
  bool IsDueWithinDays(const int days) const;

  void ToggleCompleted();

  void print() const;

 private:
  int id_, course_id_;
  std::string name_;
  std::string due_date_; // (YYYYY-MM-DD) 
  std::string due_time_;  // optional, can be empty if not provided
  bool completed_;
};

#endif