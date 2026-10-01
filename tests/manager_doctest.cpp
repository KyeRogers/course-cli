#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "../include/manager.hpp"

namespace {

std::string TodayString() {
  const auto today = std::chrono::floor<std::chrono::days>(
      std::chrono::system_clock::now());
  const auto date = std::chrono::year_month_day{today};

  std::ostringstream output;
  output << std::setfill('0') << std::setw(4)
         << static_cast<int>(date.year()) << '-'
         << std::setw(2) << static_cast<unsigned>(date.month()) << '-'
         << std::setw(2) << static_cast<unsigned>(date.day());
  return output.str();
}

std::string OffsetDate(const int offset_days) {
  const auto today = std::chrono::sys_days{
      std::chrono::year_month_day{std::chrono::floor<std::chrono::days>(
          std::chrono::system_clock::now())}};
  const auto target = today + std::chrono::days{offset_days};
  const auto date = std::chrono::year_month_day{target};

  std::ostringstream output;
  output << std::setfill('0') << std::setw(4)
         << static_cast<int>(date.year()) << '-'
         << std::setw(2) << static_cast<unsigned>(date.month()) << '-'
         << std::setw(2) << static_cast<unsigned>(date.day());
  return output.str();
}

void WriteJson(const std::string& json_text) {
  std::filesystem::create_directories("data");
  std::ofstream output_file("data/data.json");
  output_file << json_text;
  output_file.close();
}

std::string BuildJsonFixture() {
  return R"({
    "courses": [
      {"id": 1, "name": "maths", "colour": "Blue"},
      {"id": 2, "name": "physics", "colour": "Red"}
    ],
    "assignments": [
      {"id": 1, "course_id": 1, "name": "today task", "due_date": "today", "due_time": "09:00", "completed": false},
      {"id": 2, "course_id": 1, "name": "three days later", "due_date": "three", "due_time": "09:00", "completed": false},
      {"id": 3, "course_id": 2, "name": "later in week", "due_date": "later", "due_time": "16:00", "completed": false}
    ]
  })";
}

}  // namespace

TEST_CASE("manager can add a task to an existing course") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                       std::filesystem::path("coursecli-manager-tests");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);
  WriteJson(R"({
    "courses": [{"id": 1, "name": "maths", "colour": "Blue"}],
    "assignments": []
  })");

  Manager manager;

  std::string due_date = "2026-09-12";
  CHECK(manager.AddTask("essay", "maths", due_date, "18:00"));

  const Assignment* assignment = manager.GetAssignmentById(1);
  REQUIRE(assignment != nullptr);
  CHECK(assignment->GetName() == "essay");
  CHECK(assignment->GetDueDate() == "2026-09-12");
  CHECK(assignment->GetDueTime() == "18:00");

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}

TEST_CASE("manager filters a supplied assignment list for today") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                       std::filesystem::path("coursecli-manager-tests-today");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);

  const std::string today = TodayString();
  const std::string later = OffsetDate(3);
  std::ostringstream json;
  json << "{\n"
       << "  \"courses\": [{\"id\": 1, \"name\": \"maths\", \"colour\": \"Blue\"}],\n"
       << "  \"assignments\": [\n"
       << "    {\"id\": 1, \"course_id\": 1, \"name\": \"today task\", \"due_date\": \"" << today
       << "\", \"due_time\": \"09:00\", \"completed\": false},\n"
       << "    {\"id\": 2, \"course_id\": 1, \"name\": \"later task\", \"due_date\": \"" << later
       << "\", \"due_time\": \"18:00\", \"completed\": false}\n"
       << "  ]\n"
       << "}\n";
  WriteJson(json.str());

  Manager manager;

  const auto today_tasks = manager.FilterAssignmentsForDate(
      manager.GetAssignments(), "today");
  REQUIRE(today_tasks.size() == 1);
  CHECK(today_tasks.front().GetName() == "today task");

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}

TEST_CASE("manager filters a supplied assignment list in a week range") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                       std::filesystem::path("coursecli-manager-tests-week");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);

  const std::string today = TodayString();
  const std::string two_days = OffsetDate(2);
  const std::string nine_days = OffsetDate(9);
  std::ostringstream json;
  json << "{\n"
       << "  \"courses\": [{\"id\": 1, \"name\": \"maths\", \"colour\": \"Blue\"}],\n"
       << "  \"assignments\": [\n"
       << "    {\"id\": 1, \"course_id\": 1, \"name\": \"today task\", \"due_date\": \"" << today
       << "\", \"due_time\": \"09:00\", \"completed\": false},\n"
       << "    {\"id\": 2, \"course_id\": 1, \"name\": \"nearby task\", \"due_date\": \"" << two_days
       << "\", \"due_time\": \"18:00\", \"completed\": false},\n"
       << "    {\"id\": 3, \"course_id\": 1, \"name\": \"too late task\", \"due_date\": \"" << nine_days
       << "\", \"due_time\": \"10:00\", \"completed\": false}\n"
       << "  ]\n"
       << "}\n";
  WriteJson(json.str());

  Manager manager;

  const auto week_tasks = manager.FilterAssignmentsInRange(
      manager.GetAssignments(), "today", "week");
  REQUIRE(week_tasks.size() == 2);
  CHECK(week_tasks[0].GetName() == "today task");
  CHECK(week_tasks[1].GetName() == "nearby task");

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}

TEST_CASE("manager can complete an assignment") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                       std::filesystem::path("coursecli-manager-tests-done");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);
  WriteJson(R"({
    "courses": [{"id": 1, "name": "maths", "colour": "Blue"}],
    "assignments": [{"id": 1, "course_id": 1, "name": "essay", "due_date": "2026-09-12", "due_time": "09:00", "completed": false}]
  })");

  Manager manager;

  CHECK(manager.GetAssignmentById(1) != nullptr);
  CHECK_FALSE(manager.GetAssignmentById(1)->GetCompleted());

  manager.CompleteAssignmentById(1);
  CHECK(manager.GetAssignmentById(1)->GetCompleted());

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}

TEST_CASE("deleting a course deletes its assignments") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                        std::filesystem::path("coursecli-manager-tests-delete-course");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);
  WriteJson(R"({
    "courses": [
      {"id": 1, "name": "maths", "colour": "Blue"},
      {"id": 2, "name": "physics", "colour": "Red"}
    ],
    "assignments": [
      {"id": 1, "course_id": 1, "name": "maths task", "due_date": "2026-09-12", "due_time": "09:00", "completed": false},
      {"id": 2, "course_id": 2, "name": "physics task", "due_date": "2026-09-12", "due_time": "10:00", "completed": false}
    ]
  })");

  Manager manager;
  manager.DeleteCourse(1);

  CHECK(manager.GetCourseById(1) == nullptr);
  CHECK(manager.GetAssignmentById(1) == nullptr);
  CHECK(manager.GetCourseById(2) != nullptr);
  CHECK(manager.GetAssignmentById(2) != nullptr);

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}

TEST_CASE("assignment filters can be composed over a supplied vector") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                        std::filesystem::path("coursecli-manager-tests-filters");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);

  std::ostringstream json;
  json << R"({
    "courses": [
      {"id": 1, "name": "maths", "colour": "Blue"},
      {"id": 2, "name": "physics", "colour": "Red"}
    ],
    "assignments": [
      {"id": 1, "course_id": 1, "name": "late maths", "due_date": ")"
       << OffsetDate(-1)
       << R"(", "due_time": "09:00", "completed": false},
      {"id": 2, "course_id": 1, "name": "done maths", "due_date": ")"
       << OffsetDate(-2)
       << R"(", "due_time": "10:00", "completed": true},
      {"id": 3, "course_id": 2, "name": "late physics", "due_date": ")"
       << OffsetDate(-1)
       << R"(", "due_time": "11:00", "completed": false}
    ]
  })";
  WriteJson(json.str());

  Manager manager;
  const auto pending = manager.FilterAssignmentsByCompletion(
      manager.GetAssignments(), CompletionFilter::Pending);
  const auto maths = manager.FilterAssignmentsForCourse(pending, "maths");
  const auto overdue = manager.FilterAssignmentsBeforeDate(maths, "today");

  REQUIRE(overdue.size() == 1);
  CHECK(overdue.front().GetId() == 1);
  CHECK(overdue.front().GetName() == "late maths");

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}

TEST_CASE("calendar week is calculated from the supplied weekday") {
  const auto previous = std::filesystem::current_path();
  const auto temp_dir = std::filesystem::temp_directory_path() /
                        std::filesystem::path("coursecli-manager-calendar-week");

  std::filesystem::remove_all(temp_dir);
  std::filesystem::create_directories(temp_dir / "data");
  std::filesystem::current_path(temp_dir);
  WriteJson(R"({
    "courses": [{"id": 1, "name": "maths", "colour": "Blue"}],
    "assignments": [
      {"id": 1, "course_id": 1, "name": "Monday", "due_date": "2026-09-28", "due_time": "", "completed": false},
      {"id": 2, "course_id": 1, "name": "Wednesday", "due_date": "2026-09-30", "due_time": "", "completed": false},
      {"id": 3, "course_id": 1, "name": "Sunday", "due_date": "2026-10-04", "due_time": "", "completed": false},
      {"id": 4, "course_id": 1, "name": "Next Monday", "due_date": "2026-10-05", "due_time": "", "completed": false}
    ]
  })");

  Manager manager;
  const CalendarWeek week =
      manager.GetCalendarWeek(manager.GetAssignments(), "2026-09-30");

  CHECK(week.monday == "2026-09-28");
  CHECK(week.sunday == "2026-10-04");
  CHECK(week.days[0].assignments.size() == 1);
  CHECK(week.days[2].assignments.size() == 1);
  CHECK(week.days[6].assignments.size() == 1);
  CHECK(week.next_monday == "2026-10-05");

  std::filesystem::current_path(previous);
  std::filesystem::remove_all(temp_dir);
}
