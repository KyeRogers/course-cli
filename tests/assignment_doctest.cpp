#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "../include/assignment.hpp"

TEST_CASE("assignment stores its basic data") {
  Assignment assignment{7, 2, "essay", "2026-09-12", std::string{"18:30"}};

  CHECK(assignment.GetId() == 7);
  CHECK(assignment.GetCourseId() == 2);
  CHECK(assignment.GetName() == "essay");
  CHECK(assignment.GetDueDate() == "2026-09-12");
  CHECK(assignment.GetDueTime() == "18:30");
  CHECK_FALSE(assignment.GetCompleted());
}

TEST_CASE("assignment parses a valid date") {
  Assignment assignment{1, 1, "task", "2026-09-12"};
  const auto date = assignment.GetDateKey();

  CHECK(static_cast<int>(date.year()) == 2026);
  CHECK(static_cast<unsigned>(date.month()) == 9);
  CHECK(static_cast<unsigned>(date.day()) == 12);
}

TEST_CASE("invalid dates throw an exception") {
  Assignment assignment{1, 1, "task", "2026-02-30"};

  CHECK_THROWS_AS(assignment.GetDateKey(), std::runtime_error);
}

TEST_CASE("completion status can be toggled") {
  Assignment assignment{1, 1, "task", "2026-09-10"};

  CHECK_FALSE(assignment.GetCompleted());
  assignment.ToggleCompleted();
  CHECK(assignment.GetCompleted());
}
