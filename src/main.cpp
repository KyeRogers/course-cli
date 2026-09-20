#include <array>
#include <chrono>
#include <ctime>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "../include/manager.hpp"

namespace {

ftxui::Color ToUiColor(const CourseColour colour) {
  switch (colour) {
    case CourseColour::Red:
      return ftxui::Color::Red;
    case CourseColour::Green:
      return ftxui::Color::Green;
    case CourseColour::Yellow:
      return ftxui::Color::Yellow;
    case CourseColour::Blue:
      return ftxui::Color::Blue;
    case CourseColour::Magenta:
      return ftxui::Color::Magenta;
    case CourseColour::Cyan:
      return ftxui::Color::Cyan;
    case CourseColour::White:
      return ftxui::Color::White;
    case CourseColour::BrightBlack:
      return ftxui::Color::GrayDark;
    case CourseColour::BrightBlue:
      return ftxui::Color::BlueLight;
    case CourseColour::Default:
    default:
      return ftxui::Color::Default;
  }
}

void PrintUsage() {
  std::cout
      << "Usage:\n"
      << "  course-cli                 Run the interactive menu\n"
      << "  course-cli today [-a]      Show today's pending assignments\n"
      << "  course-cli tomorrow [-a]   Show tomorrow's pending assignments\n"
      << "  course-cli week [-a]       Show the next seven days\n"
      << "  course-cli course NAME [-a] Show assignments for a course\n"
      << "  course-cli add_task NAME COURSE DATE [TIME]\n"
      << "  course-cli add_course NAME COLOUR\n"
      << "  course-cli done ID         Toggle an assignment's completion\n"
      << "  course-cli -h              Show this help\n"
      << "\nBy default listings show pending tasks only. Use -a to include completed tasks.\n"
      << "\nInteractive features:\n"
      << "  Dashboard  Overview of pending and completed tasks.\n"
      << "  Tasks      Add, complete, review, and clean up old tasks.\n"
      << "  Courses    Add/delete courses and view their tasks.\n"
      << "  Calendar   Browse the weekly agenda and add tasks by day.\n"
      << "\nKeyboard: Left/Right changes sections, Up/Down changes tasks,\n"
      << "Enter completes or selects, Space confirms choices, q goes back.\n";
}

bool ParseId(const std::string& value, int& id) {
  try {
    std::size_t parsed_characters = 0;
    id = std::stoi(value, &parsed_characters);
    return parsed_characters == value.size();
  } catch (const std::exception&) {
    return false;
  }
}

bool HasFlag(const int argc, const char* argv[], const std::string& flag) {
  for (int index = 1; index < argc; ++index) {
    if (std::string{argv[index]} == flag) {
      return true;
    }
  }
  return false;
}

int RunCommand(Manager& manager, const int argc, const char* argv[]) {
  const std::string command{argv[1]};
  if (command == "-h" || command == "--help" || command == "help") {
    PrintUsage();
    return 0;
  }
  const bool show_completed = HasFlag(argc, argv, "-a");
  if (command == "today" && (argc == 2 || (argc == 3 && show_completed))) {
    manager.ShowAssignmentsForDate("today", show_completed);
    return 0;
  }
  if (command == "tomorrow" &&
      (argc == 2 || (argc == 3 && show_completed))) {
    manager.ShowAssignmentsForDate("tomorrow", show_completed);
    return 0;
  }
  if (command == "week" && (argc == 2 || (argc == 3 && show_completed))) {
    manager.ShowAssignmentsInRange("today", "week", show_completed);
    return 0;
  }
  if (command == "course" &&
      (argc == 3 || (argc == 4 && show_completed &&
                     std::string{argv[3]} == "-a"))) {
    manager.ShowAssignmentsForCourse(argv[2], show_completed);
    return 0;
  }
  if (command == "add_task" && (argc == 5 || argc == 6)) {
    const std::string name{argv[2]};
    const std::string course{argv[3]};
    std::string date{argv[4]};
    const std::string time = argc == 6 ? argv[5] : "";
    return manager.AddTask(name, course, date, time) ? 0 : 1;
  }
  if (command == "add_course" && argc == 4) {
    const std::string name{argv[2]};
    const std::string colour{argv[3]};
    manager.AddCourse(name, Colors::from_string(colour));
    return 0;
  }
  if (command == "done" && argc == 3) {
    int id = 0;
    if (ParseId(argv[2], id)) {
      manager.CompleteAssignmentById(id);
      return 0;
    }
  }

  std::cerr << "Invalid arguments.\n\n";
  PrintUsage();
  return 1;
}

struct DateValue {
  int year;
  int month;
  int day;
};

struct AddTaskPanelState {
  std::string task_name;
  std::string task_date;
  std::string task_time;
  std::vector<std::string> course_options;
  int selected_course = 0;
  int chosen_course = 0;
  int selected_priority = 1;
  int chosen_priority = 1;
  std::vector<std::string> priority_options{"!", "-", "."};
  std::string status =
      "Select a course with Space, then enter the task details.";
};

    DateValue ParseDateValue(const std::string& date);

std::string ReadableDate(const std::string& value) {
  try {
    const DateValue date = ParseDateValue(value);
    std::tm time{};
    time.tm_year = date.year - 1900;
    time.tm_mon = date.month - 1;
    time.tm_mday = date.day;
    std::mktime(&time);
    char weekday[32]{};
    char month[32]{};
    std::strftime(weekday, sizeof(weekday), "%A", &time);
    std::strftime(month, sizeof(month), "%B", &time);
    return std::string(weekday) + " " + std::to_string(date.day) + " " +
           month;
  } catch (const std::exception&) {
    return value;
  }
}

std::string PriorityLabel(const AssignmentPriority priority) {
  switch (priority) {
    case AssignmentPriority::High:
      return "!";
    case AssignmentPriority::Low:
      return ".";
    case AssignmentPriority::Normal:
    default:
      return "-";
  }
}

std::string ShortDate(const DateValue& date) {
  static const std::array<std::string, 12> months{
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return std::to_string(date.day) + " " + months[date.month - 1];
}

int DaysFromCivil(int year, int month, int day) {
  year -= month <= 2;
  const int era = (year >= 0 ? year : year - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(year - era * 400);
  const unsigned doy =
      (153 * (month + (month > 2 ? -2 : 9)) + 2) / 5 + day - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + static_cast<int>(doe) - 719468;
}

DateValue ParseDateValue(const std::string& date) {
  std::tm parsed{};
  std::istringstream stream(date);
  stream >> std::get_time(&parsed, "%Y-%m-%d");
  if (!stream || stream.peek() != std::char_traits<char>::eof()) {
    throw std::runtime_error("Invalid date: " + date);
  }
  return {parsed.tm_year + 1900, parsed.tm_mon + 1, parsed.tm_mday};
}

DateValue TodayDateValue() {
  const std::time_t now = std::time(nullptr);
  const std::tm* local = std::localtime(&now);
  return {local->tm_year + 1900, local->tm_mon + 1, local->tm_mday};
}

DateValue DateFromTodayOffset(const int offset) {
  const DateValue today = TodayDateValue();
  std::tm date{};
  date.tm_year = today.year - 1900;
  date.tm_mon = today.month - 1;
  date.tm_mday = today.day + offset;
  date.tm_hour = 12;
  const std::time_t timestamp = std::mktime(&date);
  const std::tm* normalized = std::localtime(&timestamp);
  return {normalized->tm_year + 1900, normalized->tm_mon + 1,
          normalized->tm_mday};
}

std::string FormatDateValue(const DateValue& date) {
  std::ostringstream output;
  output << std::setfill('0') << std::setw(4) << date.year << '-'
         << std::setw(2) << date.month << '-' << std::setw(2) << date.day;
  return output.str();
}

std::string WeekdayName(const DateValue& date) {
  std::tm value{};
  value.tm_year = date.year - 1900;
  value.tm_mon = date.month - 1;
  value.tm_mday = date.day;
  value.tm_hour = 12;
  std::mktime(&value);
  char name[32]{};
  std::strftime(name, sizeof(name), "%A", &value);
  return name;
}

int DaysFromToday(const std::string& date) {
  const DateValue today = TodayDateValue();
  const DateValue target = ParseDateValue(date);
  return DaysFromCivil(target.year, target.month, target.day) -
         DaysFromCivil(today.year, today.month, today.day);
}

void RunHelp() {
  using namespace ftxui;
  auto screen = ScreenInteractive::TerminalOutput();
  auto content = vbox({text(" COURSE MANAGER HELP ") | bold | color(Color::Cyan) |
                           center,
                       separator(),
                       text("Dashboard: Left/Right sections, Up/Down tasks, Enter toggles"),
                       text("Tasks: add, complete, edit, cleanup old completed tasks"),
                       text("Courses: Enter opens tasks, q deselects, d deletes"),
                       text("Calendar: Left/Right days, Up/Down tasks, a adds"),
                       separator(),
                       text("Global: Space confirms choices, ? opens Help, q/Esc goes back")}) |
                border;
  auto app = CatchEvent(Renderer([&] { return content; }),
                        [&](const Event& event) {
                          if (event == Event::Character('q') ||
                              event == Event::Escape) {
                            screen.Exit();
                            return true;
                          }
                          return false;
                        });
  screen.Loop(app);
}

void RunDashboard(Manager& manager) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
  const std::vector<std::string> section_names{"Overdue", "Today", "Upcoming",
                                                "Completed"};
  int selected_section = 0;
  int selected_task = 0;
  std::vector<int> task_ids;

  auto renderer = Renderer([&] {
    task_ids.clear();
    std::array<Elements, 4> rows;
    int today_total = 0;
    int today_completed = 0;
    int week_total = 0;
    int week_completed = 0;
    int pending_count = 0;
    int completed_count = 0;

    for (const Assignment& assignment : manager.GetAssignments()) {
      const bool completed = assignment.GetCompleted();
      completed ? ++completed_count : ++pending_count;
      const int days = DaysFromToday(assignment.GetDueDate());
      if (days == 0) {
        ++today_total;
        if (completed) ++today_completed;
      }
      if (days >= 0 && days <= 7) {
        ++week_total;
        if (completed) ++week_completed;
      }

      int section = 3;
      if (!completed && days < 0) section = 0;
      else if (!completed && days == 0) section = 1;
      else if (!completed && days > 0) section = 2;

      const Course* course = manager.GetCourseById(assignment.GetCourseId());
      const std::string label =
          std::string(completed ? "[x] " : "[ ] ") +
          PriorityLabel(assignment.GetPriority()) + " " + assignment.GetName() +
          "  " + ReadableDate(assignment.GetDueDate()) +
          (course != nullptr ? "  (" + course->GetName() + ")" : "");
      Element row = text(label) |
                    color(course == nullptr ? Color::Default
                                            : ToUiColor(course->GetColour()));
      if (completed) row = row | dim;
      rows[section].push_back(row);
      if (section == selected_section) task_ids.push_back(assignment.GetId());
    }

    if (selected_task >= static_cast<int>(task_ids.size())) {
      selected_task = task_ids.empty() ? 0 : static_cast<int>(task_ids.size()) - 1;
    }
    if (selected_task < static_cast<int>(rows[selected_section].size())) {
      rows[selected_section][selected_task] =
          rows[selected_section][selected_task] | inverted;
    }

    if (rows[2].size() > 5) {
      rows[2].resize(5);
    }
    Elements active_rows = rows[selected_section];
    return vbox({
               text("Dashboard") | bold | color(Color::Cyan),
               text(ReadableDate(FormatDateValue(TodayDateValue()))) | dim,
               separator(),
               hbox({text("Pending: ") | bold, text(std::to_string(pending_count)),
                     text("    Completed: ") | bold,
                     text(std::to_string(completed_count))}),
               text("Today: " + std::to_string(today_completed) + "/" +
                    std::to_string(today_total)),
               text("Week: " + std::to_string(week_completed) + "/" +
                    std::to_string(week_total)),
               separator(),
               text((selected_section == 0 ? "> " : "  ") + section_names[0] +
                    "  (" + std::to_string(rows[0].size()) + ")") | bold,
               text((selected_section == 1 ? "> " : "  ") + section_names[1] +
                    "  (" + std::to_string(rows[1].size()) + ")") | bold,
               text((selected_section == 2 ? "> " : "  ") + section_names[2] +
                    "  (next 5)") | bold,
               text((selected_section == 3 ? "> " : "  ") + section_names[3] +
                    "  (" + std::to_string(rows[3].size()) + ")") | bold,
               active_rows.empty() ? text("  No tasks") : vbox(active_rows),
               separator(),
               text("Left/Right = section  Up/Down = task  Enter = toggle  q/Esc = back") |
                   dim,
           }) |
           border;
  });

  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.Exit();
      return true;
    }
    if (event == Event::ArrowLeft) {
      if (selected_section > 0) --selected_section;
      selected_task = 0;
      return true;
    }
    if (event == Event::ArrowRight) {
      if (selected_section < 3) ++selected_section;
      selected_task = 0;
      return true;
    }
    if (event == Event::ArrowUp) {
      if (selected_task > 0) --selected_task;
      return true;
    }
    if (event == Event::ArrowDown) {
      if (selected_task + 1 < static_cast<int>(task_ids.size())) ++selected_task;
      return true;
    }
    if (event == Event::Return && !task_ids.empty()) {
      manager.CompleteAssignmentById(task_ids[selected_task]);
      selected_task = 0;
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

void RunDashboardLegacy(Manager& manager) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
  int selected_section = 0;
  int selected_task = 0;
  std::vector<int> task_ids;
  auto renderer = Renderer([&] {
    task_ids.clear();
    Elements today_pending;
    Elements today_completed;
    Elements week_pending;
    Elements week_completed;
    std::vector<int> today_pending_ids;
    std::vector<int> today_completed_ids;
    std::vector<int> week_pending_ids;
    std::vector<int> week_completed_ids;
    int completed_count = 0;
    int pending_count = 0;

    for (const Assignment& assignment : manager.GetAssignments()) {
      assignment.GetCompleted() ? ++completed_count : ++pending_count;

      const int days = DaysFromToday(assignment.GetDueDate());
      if (days < 0 || days > 7) {
        continue;
      }

      const Course* course = manager.GetCourseById(assignment.GetCourseId());
      const bool completed = assignment.GetCompleted();
      const std::string label =
          std::string(completed ? "[x] " : "[ ] ") +
          PriorityLabel(assignment.GetPriority()) + " " + assignment.GetName() +
          "  " + ReadableDate(assignment.GetDueDate()) +
          (course != nullptr ? "  (" + course->GetName() + ")" : "");
      Element row = text(label) |
                    color(course == nullptr ? Color::Default
                                            : ToUiColor(course->GetColour()));

      if (completed) {
        const int section = days == 0 ? 0 : 1;
        auto& completed_ids = section == 0 ? today_completed_ids
                                           : week_completed_ids;
        if (section == selected_section) {
          const int position = static_cast<int>(
              section == 0 ? today_pending_ids.size() + completed_ids.size()
                           : week_pending_ids.size() + completed_ids.size());
          completed_ids.push_back(assignment.GetId());
          if (position == selected_task) {
            row = row | inverted;
          }
        } else {
          completed_ids.push_back(assignment.GetId());
        }
        row = row | dim;
        (days == 0 ? today_completed : week_completed).push_back(row);
      } else {
        const int section = days == 0 ? 0 : 1;
        if (section == selected_section) {
          task_ids.push_back(assignment.GetId());
        }
        auto& pending_ids = section == 0 ? today_pending_ids : week_pending_ids;
        const int position = static_cast<int>(pending_ids.size());
        pending_ids.push_back(assignment.GetId());
        if (section == selected_section &&
            position == selected_task) {
          row = row | inverted;
        }
        (section == 0 ? today_pending : week_pending).push_back(row);
      }
    }

    if (selected_section == 0) {
      task_ids.clear();
      task_ids.insert(task_ids.end(), today_pending_ids.begin(),
                      today_pending_ids.end());
      task_ids.insert(task_ids.end(), today_completed_ids.begin(),
                      today_completed_ids.end());
    } else {
      task_ids.clear();
      task_ids.insert(task_ids.end(), week_pending_ids.begin(),
                      week_pending_ids.end());
      task_ids.insert(task_ids.end(), week_completed_ids.begin(),
                      week_completed_ids.end());
    }

    if (selected_task >= static_cast<int>(task_ids.size())) {
      selected_task = task_ids.empty() ? 0 : static_cast<int>(task_ids.size()) - 1;
    }

    return vbox({
               text("Dashboard") | bold,
               separator(),
               hbox({text("Pending: ") | bold, text(std::to_string(pending_count)),
                     text("    Completed: ") | bold,
                     text(std::to_string(completed_count))}),
               separator(),
                 text((selected_section == 0 ? "> " : "  ") +
                  std::string("Today")) |
                   bold,
               today_pending.empty() ? text("No pending tasks due today.")
                                     : vbox(today_pending),
               today_completed.empty()
                   ? text("  No completed tasks") | dim
                   : vbox(today_completed),
               separator(),
                 text((selected_section == 1 ? "> " : "  ") +
                  std::string("Next 7 days")) |
                   bold,
               week_pending.empty() ? text("No pending tasks due this week.")
                                    : vbox(week_pending),
               week_completed.empty()
                   ? text("  No completed tasks") | dim
                   : vbox(week_completed),
               separator(),
                 text("Left/Right = section, Up/Down = task, Enter = complete, q = back") |
                   dim,
           }) |
           border;
  });

  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.Exit();
      return true;
    }
    if (event == Event::ArrowUp) {
      if (selected_task > 0) {
        --selected_task;
      }
      return true;
    }
    if (event == Event::ArrowLeft) {
      if (selected_section > 0) {
        --selected_section;
        selected_task = 0;
      }
      return true;
    }
    if (event == Event::ArrowRight) {
      if (selected_section < 1) {
        ++selected_section;
        selected_task = 0;
      }
      return true;
    }
    if (event == Event::ArrowDown) {
      if (!task_ids.empty() &&
          selected_task < static_cast<int>(task_ids.size()) - 1) {
        ++selected_task;
      }
      return true;
    }
    if (event == Event::Return && !task_ids.empty()) {
      manager.CompleteAssignmentById(task_ids[selected_task]);
      selected_task = 0;
      return true;
    }
    return false;
  });

  screen.Loop(app);
}

ftxui::Component MakeAddTaskPanel(Manager& manager,
                                  const std::optional<std::string>& fixed_date,
                                  const std::function<void()>& on_added = {}) {
  using namespace ftxui;

  const std::optional<std::string> panel_date = fixed_date;
  const std::function<void()> added_callback = on_added;
  auto state = std::make_shared<AddTaskPanelState>();

  for (const Course& course : manager.GetCourses()) {
    state->course_options.push_back("[ ] " + course.GetName());
  }

  auto name_input = Input(&state->task_name, "Task name");
  MenuOption course_option;
  course_option.entries_option.transform = [state,
                                            &manager](const EntryState& entry) {
    const bool chosen = entry.index == state->chosen_course;
    Element row =
        text(std::string(chosen ? "[x] " : "[ ] ") +
             manager.GetCourses()[entry.index].GetName()) |
        color(ToUiColor(manager.GetCourses()[entry.index].GetColour()));
    if (entry.focused) {
      row = row | inverted;
    }
    if (entry.active) {
      row = row | bold;
    }
    return row;
  };
  auto course_menu =
      Menu(&state->course_options, &state->selected_course, course_option);
  auto course_selector = CatchEvent(course_menu, [state](const Event& event) {
    if (event == Event::Character(' ') &&
        state->selected_course <
            static_cast<int>(state->course_options.size())) {
      state->chosen_course = state->selected_course;
      return true;
    }
    return false;
  });
  MenuOption priority_option;
  priority_option.entries_option.transform = [state](
                                                  const EntryState& entry) {
    Element row = text(std::string(entry.index == state->chosen_priority
                                        ? "[x] "
                                        : "[ ] ") +
                       state->priority_options[entry.index]);
    if (entry.focused) row = row | inverted;
    return row;
  };
  auto priority_menu =
      Menu(&state->priority_options, &state->selected_priority, priority_option);
  auto priority_selector = CatchEvent(
      priority_menu, [state](const Event& event) {
        if (event == Event::Character(' ')) {
          state->chosen_priority = state->selected_priority;
          return true;
        }
        return false;
      });
  auto time_input = Input(&state->task_time, "Due time (optional)");
  auto date_input = Input(&state->task_date, "Due date (YYYY-MM-DD)");
  auto add_button = Button("Add task", [&, state, panel_date, added_callback] {
    const std::string date =
        panel_date.has_value() ? *panel_date : state->task_date;
    if (state->task_name.empty() || state->course_options.empty() ||
        date.empty()) {
      state->status =
          panel_date.has_value()
              ? "Name and course are required. Select the course with Space."
              : "Name, course, and date are required.";
      return;
    }

    std::string submitted_date = date;
    if (state->chosen_course < 0 ||
        state->chosen_course >= static_cast<int>(manager.GetCourses().size())) {
      state->status = "Select a course with Space.";
      return;
    }

    if (manager.AddTask(state->task_name,
                        manager.GetCourses()[state->chosen_course].GetName(),
                        submitted_date, state->task_time)) {
      Assignment* added =
          manager.GetAssignmentById(manager.GetAssignments().back().GetId());
      if (added != nullptr) {
        added->SetDetails(
            added->GetName(), added->GetCourseId(), added->GetDueDate(),
            added->GetDueTime(),
            state->chosen_priority == 0
                ? AssignmentPriority::High
                : state->chosen_priority == 2 ? AssignmentPriority::Low
                                               : AssignmentPriority::Normal);
      }
      state->task_name.clear();
      state->task_date.clear();
      state->task_time.clear();
      state->status = "Task added.";
      if (added_callback) {
        added_callback();
      }
    } else {
      state->status = "That course does not exist.";
    }
  });

  std::vector<Component> form_components{
      name_input,
      course_selector,
      priority_selector,
  };
  if (!panel_date.has_value()) {
    form_components.push_back(date_input);
  }
  form_components.push_back(time_input);
  form_components.push_back(add_button);
  auto form = Container::Vertical(std::move(form_components));

  return Renderer(form, [state, panel_date, name_input, course_selector,
                         priority_selector, date_input, time_input, add_button] {
    return vbox({
               text("Add task") | bold,
               separator(),
               name_input->Render(),
               text("Course"),
               course_selector->Render(),
               text("Priority (Space to select)"),
               priority_selector->Render(),
               panel_date.has_value() ? text("Due date: " + *panel_date) | dim
                                      : date_input->Render(),
               time_input->Render(),
               add_button->Render(),
               separator(),
               text(state->status) | dim,
           }) |
           border;
  });
}

void RunAgendaAddTask(Manager& manager, const std::string& date) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
  auto panel = MakeAddTaskPanel(manager, date, [&] { screen.Exit(); });

  auto app = CatchEvent(panel, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.Exit();
      return true;
    }
    return false;
  });

  screen.Loop(app);
}
void RunCalendar(Manager& manager) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();

  int week_offset = 0;
  int selected_day = 0;
  int selected_task = -1;

  // Tasks belonging to each day of the currently displayed week.
  std::array<std::vector<int>, 7> day_task_ids;

  auto get_week_start_offset = [&]() {
    const DateValue today = TodayDateValue();

    std::tm today_tm{};
    today_tm.tm_year = today.year - 1900;
    today_tm.tm_mon = today.month - 1;
    today_tm.tm_mday = today.day;
    today_tm.tm_hour = 12;

    std::mktime(&today_tm);

    // tm_wday: Sunday = 0, Monday = 1, ..., Saturday = 6
    const int days_since_monday = (today_tm.tm_wday + 6) % 7;

    return week_offset * 7 - days_since_monday;
  };

  auto get_selected_date = [&]() {
    const int week_start_offset = get_week_start_offset();

    return DateFromTodayOffset(week_start_offset + selected_day);
  };

  auto renderer = Renderer([&] {
    for (auto& ids : day_task_ids) ids.clear();
    const int week_start_offset = get_week_start_offset();
    Elements day_columns;

    for (int day_offset = 0; day_offset < 7; ++day_offset) {
      const DateValue date = DateFromTodayOffset(week_start_offset + day_offset);
      const std::string date_string = FormatDateValue(date);
      for (const Assignment& assignment : manager.GetAssignments()) {
        if (!assignment.GetCompleted() && assignment.GetDueDate() == date_string) {
          day_task_ids[day_offset].push_back(assignment.GetId());
        }
      }

      const std::string marker = day_offset == selected_day ? "> " : "  ";
      day_columns.push_back(
          vbox({text(marker + WeekdayName(date)) | bold,
            text(ShortDate(date)),
                text(std::to_string(day_task_ids[day_offset].size()) + " tasks") |
                    dim}) |
          (day_offset == selected_day ? inverted : nothing) | flex);
    }

    Elements selected_rows;
    for (std::size_t index = 0; index < day_task_ids[selected_day].size(); ++index) {
      const Assignment* assignment =
          manager.GetAssignmentById(day_task_ids[selected_day][index]);
      if (assignment == nullptr) continue;
      const Course* course = manager.GetCourseById(assignment->GetCourseId());
      Element row = text(std::string(selected_task >= 0 &&
                 index == static_cast<std::size_t>(selected_task)
                  ? "> " : "  ") +
                         PriorityLabel(assignment->GetPriority()) + " " +
                         assignment->GetName() +
                         (assignment->GetDueTime().empty()
                              ? ""
                              : "  " + assignment->GetDueTime())) |
                    color(course == nullptr ? Color::Default
                                            : ToUiColor(course->GetColour()));
      selected_rows.push_back(row);
    }

    return vbox({text("Weekly agenda") | bold,
                 separator(),
                 hbox(day_columns) | border,
                 separator(),
                 text("Selected day: " +
                      ReadableDate(FormatDateValue(DateFromTodayOffset(
                          week_start_offset + selected_day)))) | bold,
                 selected_rows.empty() ? text("No tasks") : vbox(selected_rows),
                 separator(),
                 text("Left/Right = day  Up/Down = task  Right on Sunday = next week  a = add  Enter = complete") |
                     dim}) |
           border;
  });

  auto app = CatchEvent(renderer, [&](const Event& event) {
    // --------------------------------------------------
    // Quit
    // --------------------------------------------------

    if (event == Event::Character('q') || event == Event::Escape) {
      screen.Exit();
      return true;
    }

    // --------------------------------------------------
    // Previous week
    // --------------------------------------------------

    // --------------------------------------------------
    // Previous day
    // --------------------------------------------------

    if (event == Event::ArrowLeft) {
      if (selected_day > 0) {
        --selected_day;
        selected_task = -1;
      } else if (week_offset > 0) {
        --week_offset;
        selected_day = 6;
        selected_task = -1;
      }

      return true;
    }

    // --------------------------------------------------
    // Next day
    // --------------------------------------------------

    if (event == Event::ArrowRight) {
      if (selected_day < 6) {
        ++selected_day;
        selected_task = -1;
      } else {
        ++week_offset;
        selected_day = 0;
        selected_task = -1;
      }

      return true;
    }

    // --------------------------------------------------
    // Previous task on selected day
    // --------------------------------------------------

    if (event == Event::ArrowUp) {
      if (selected_task > 0) {
        --selected_task;
      } else if (selected_task == 0) {
        selected_task = -1;
      }

      return true;
    }

    // --------------------------------------------------
    // Next task on selected day
    // --------------------------------------------------

    if (event == Event::ArrowDown) {
      const int task_count =
          static_cast<int>(day_task_ids[selected_day].size());

      if (selected_task < task_count - 1) {
        ++selected_task;
      }

      return true;
    }

    // --------------------------------------------------
    // Add task to selected day
    // --------------------------------------------------

    if (event == Event::Character('a')) {
      const DateValue selected_date = get_selected_date();

      RunAgendaAddTask(manager, FormatDateValue(selected_date));

      // Reset task selection because the list may have
      // changed after adding a task.
      selected_task = -1;

      return true;
    }

    // --------------------------------------------------
    // Complete selected task
    // --------------------------------------------------

    if (event == Event::Return) {
      const auto& tasks = day_task_ids[selected_day];

      if (!tasks.empty() && selected_task >= 0 &&
          selected_task < static_cast<int>(tasks.size())) {
        const int assignment_id = tasks[selected_task];

        manager.CompleteAssignmentById(assignment_id);

        // If we completed the last task, move the
        // selection back to the previous task.
        if (selected_task > 0) {
          --selected_task;
        } else {
          selected_task = 0;
        }
      }

      return true;
    }

    return false;
  });

  screen.Loop(app);
}

void RunTasksMenu(Manager& manager) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> filters{"Pending", "Completed", "Cleanup", "Add task",
                                   "Back"};
  int selected_filter = 0;
  int selected_task = 0;

  std::vector<std::string> task_entries;
  std::vector<int> task_ids;
  std::string status = "Select a task and press Enter to mark it completed.";
  std::string cleanup_age;
  std::vector<std::string> cleanup_units{"Days", "Weeks"};
  int selected_cleanup_unit = 0;
  int chosen_cleanup_unit = 0;

  MenuOption filter_option;
  filter_option.on_enter = [&] {
    if (selected_filter == 4) {
      screen.Exit();
    }
  };

  auto filter_menu = Menu(&filters, &selected_filter, filter_option);
  auto task_menu = Menu(&task_entries, &selected_task);
  auto add_panel = MakeAddTaskPanel(manager, std::nullopt, [&] {
    selected_filter = 0;
    status = "Task added.";
  });
  auto cleanup_age_input = Input(&cleanup_age, "Age");
  MenuOption cleanup_unit_option;
  cleanup_unit_option.entries_option.transform = [&](const EntryState& entry) {
    Element row = text(std::string(entry.index == chosen_cleanup_unit ? "[x] "
                                                                       : "[ ] ") +
                       cleanup_units[entry.index]);
    if (entry.focused) {
      row = row | inverted;
    }
    return row;
  };
  auto cleanup_unit_menu =
      Menu(&cleanup_units, &selected_cleanup_unit, cleanup_unit_option);
  auto cleanup_unit_selector = CatchEvent(
      cleanup_unit_menu, [&](const Event& event) {
        if (event == Event::Character(' ')) {
          chosen_cleanup_unit = selected_cleanup_unit;
          return true;
        }
        return false;
      });
  auto cleanup_button = Button("Remove completed tasks", [&] {
    int age = 0;
    if (!ParseId(cleanup_age, age) || age < 0) {
      status = "Enter a valid non-negative age.";
      return;
    }

    const int days = chosen_cleanup_unit == 1 ? age * 7 : age;
    const int removed = manager.RemoveCompletedOlderThanDays(days);
    status = "Removed " + std::to_string(removed) + " completed task" +
             (removed == 1 ? "." : "s.");
    cleanup_age.clear();
    selected_filter = 1;
  });
  auto cleanup_panel = Container::Vertical({
      cleanup_age_input,
      cleanup_unit_selector,
      cleanup_button,
  });

  auto left_panel = Container::Vertical({
      filter_menu,
      add_panel,
      cleanup_panel,
  });

  auto content = Container::Horizontal({
      left_panel,
      task_menu,
  });

  auto navigation = CatchEvent(content, [&](const Event& event) {
    if (event == Event::ArrowRight) {
      content->SetActiveChild(task_menu);
      return true;
    }

    if (event == Event::ArrowLeft) {
      content->SetActiveChild(left_panel);

      if (selected_filter == 3) {
        left_panel->SetActiveChild(add_panel);
        add_panel->TakeFocus();
      } else if (selected_filter == 2) {
        left_panel->SetActiveChild(cleanup_panel);
        cleanup_panel->TakeFocus();
      } else {
        left_panel->SetActiveChild(filter_menu);
      }

      return true;
    }

    return false;
  });

  auto renderer = Renderer(navigation, [&] {
    task_entries.clear();
    task_ids.clear();
    Elements task_rows;

    for (const Assignment& assignment : manager.GetAssignments()) {
      const bool show_task =
          (selected_filter == 0 && !assignment.GetCompleted()) ||
          (selected_filter == 1 && assignment.GetCompleted());

      if (show_task) {
        task_ids.push_back(assignment.GetId());
        task_entries.push_back(
            std::string(assignment.GetCompleted() ? "[x] " : "[ ] ") +
            PriorityLabel(assignment.GetPriority()) + " " +
            assignment.GetName() + "  " + ReadableDate(assignment.GetDueDate()));

        const Course* course = manager.GetCourseById(assignment.GetCourseId());
        const CourseColour colour =
            course == nullptr ? CourseColour::Default : course->GetColour();
        Element row = text(task_entries.back()) | color(ToUiColor(colour));
        if (task_rows.size() == static_cast<std::size_t>(selected_task)) {
          row = row | inverted;
        }
        task_rows.push_back(row);
      }
    }

    if (selected_task >= static_cast<int>(task_entries.size())) {
      selected_task = task_entries.empty() ? 0 : task_entries.size() - 1;
    }

    Element right_panel;

    if (selected_filter == 3) {
      right_panel = add_panel->Render() | border;
    } else if (selected_filter == 2) {
      right_panel = vbox({
                          text("Clean up completed tasks") | bold,
                          separator(),
                          text("Remove completed tasks older than:"),
                          cleanup_age_input->Render(),
                          cleanup_unit_selector->Render(),
                          cleanup_button->Render(),
                      }) |
                    border;
    } else if (selected_filter == 4) {
      right_panel = text("Returning...");
    } else {
      right_panel =
          vbox({
              text(selected_filter == 0 ? "Pending tasks" : "Completed tasks") |
                  bold,
              separator(),
              task_entries.empty() ? text("No tasks") : vbox(task_rows),
          }) |
          border;
    }

    return vbox({
               hbox({
                   vbox({
                       text("Tasks") | bold,
                       separator(),
                       filter_menu->Render(),
                   }) | border,
                   right_panel | flex,
               }),
               text(status) | dim,
           }) |
           border;
  });

  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }

    if (event == Event::Return && selected_filter < 2 && !task_ids.empty()) {
      manager.CompleteAssignmentById(task_ids[selected_task]);
      status = "Task status updated.";
      return true;
    }

    return false;
  });

  screen.Loop(app);
}

void RunCourseMenu(Manager& manager) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> filters{"Courses", "Add Course", "Back"};
  int selected_filter = 0;
  int selected_course = 0;

  std::vector<std::string> course_entries;
  std::vector<int> course_ids;
  std::vector<std::string> course_task_entries;
  std::vector<int> course_task_ids;
  int selected_course_task = 0;
  std::string course_name;
  int selected_course_id = -1;
  std::string status =
      "Use arrows to navigate, q to quit, and press ENTER to select.";

  MenuOption filter_option;
  filter_option.on_enter = [&] {
    if (selected_filter == 2) {
      screen.Exit();
    }
  };

  auto filter_menu = Menu(&filters, &selected_filter, filter_option);
  auto course_menu = Menu(&course_entries, &selected_course);
  auto course_task_menu = Menu(&course_task_entries, &selected_course_task);
  auto name_input = Input(&course_name, "Course name");

  std::vector<std::string> colour_options{"Red",     "Green", "Yellow", "Blue",
                                          "Magenta", "Cyan",  "White"};

  int selected_colour = 0;
  int chosen_colour = 0;
  std::vector<std::string> colour_entries = colour_options;

  auto colour_menu = Menu(&colour_entries, &selected_colour);
  auto colour_selector = CatchEvent(colour_menu, [&](const Event& event) {
    if (event == Event::Character(' ')) {
      chosen_colour = selected_colour;
      return true;
    }
    return false;
  });

  auto add_button = Button("Add course", [&] {
    if (course_name.empty()) {
      status = "Course name is required.";
      return;
    }

    manager.AddCourse(course_name,
                      Colors::from_string(colour_options[chosen_colour]));

    course_name.clear();
    selected_filter = 0;
    status = "Course added.";
  });

  auto form = Container::Vertical({
      name_input,
      colour_selector,
      add_button,
  });

  auto left_panel = Container::Vertical({
      filter_menu,
      form,
  });

  auto content = Container::Horizontal({
      left_panel,
      course_menu,
      course_task_menu,
  });

  auto navigation = CatchEvent(content, [&](const Event& event) {
    if (event == Event::ArrowRight) {
      content->SetActiveChild(selected_course_id == -1 ? course_menu
                                                       : course_task_menu);
      return true;
    }

    if (event == Event::ArrowLeft) {
      if (selected_course_id != -1) {
        selected_course_id = -1;
        content->SetActiveChild(course_menu);
        return true;
      }

      content->SetActiveChild(left_panel);

      if (selected_filter == 1) {
        left_panel->SetActiveChild(form);
        form->SetActiveChild(name_input);
      } else {
        left_panel->SetActiveChild(filter_menu);
      }

      return true;
    }

    return false;
  });

  auto renderer = Renderer(navigation, [&] {
    course_entries.clear();
    course_ids.clear();
    course_task_entries.clear();
    course_task_ids.clear();
    colour_entries.clear();
    Elements course_rows;
    Elements colour_rows;
    Elements course_task_rows;

    for (std::size_t index = 0; index < colour_options.size(); ++index) {
      colour_entries.push_back(
          std::string(index == static_cast<std::size_t>(chosen_colour)
                          ? "[x] "
                          : "[ ] ") +
          colour_options[index]);

      Element row =
          text(colour_entries.back()) |
          color(ToUiColor(Colors::from_string(colour_options[index])));
      if (index == static_cast<std::size_t>(selected_colour)) {
        row = row | inverted;
      }
      colour_rows.push_back(row);
    }

    for (const Course& course : manager.GetCourses()) {
      course_ids.push_back(course.GetId());
      course_entries.push_back(course.GetName() + "  [" +
                               Colors::to_string(course.GetColour()) + "]");

      Element row =
          text(course_entries.back()) | color(ToUiColor(course.GetColour()));
      if (course_rows.size() == static_cast<std::size_t>(selected_course)) {
        row = row | inverted;
      }
      course_rows.push_back(row);
    }

    if (selected_course >= static_cast<int>(course_entries.size())) {
      selected_course = course_entries.empty() ? 0 : course_entries.size() - 1;
    }

    if (!manager.GetCourses().empty() &&
        selected_course < static_cast<int>(manager.GetCourses().size())) {
      const Course& selected = manager.GetCourses()[selected_course];
      for (const Assignment& assignment : manager.GetAssignments()) {
        if (assignment.GetCourseId() != selected.GetId()) {
          continue;
        }

        Element row =
            text(std::string(assignment.GetCompleted() ? "[x] " : "[ ] ") +
                 PriorityLabel(assignment.GetPriority()) + " " +
                 assignment.GetName() + "  " + ReadableDate(assignment.GetDueDate())) |
            color(ToUiColor(selected.GetColour()));
        if (assignment.GetCompleted()) {
          row = row | dim;
        }
        course_task_rows.push_back(row);
      }
    }

    Element right_panel;
    if (selected_course_id != -1) {
      for (const Assignment& assignment : manager.GetAssignments()) {
        if (assignment.GetCourseId() != selected_course_id) {
          continue;
        }
        course_task_ids.push_back(assignment.GetId());
        course_task_entries.push_back(
            std::string(assignment.GetCompleted() ? "[x] " : "[ ] ") +
            PriorityLabel(assignment.GetPriority()) + " " +
            assignment.GetName() + "  " + ReadableDate(assignment.GetDueDate()));
      }
      right_panel = vbox({
                            text("Selected course tasks") | bold,
                            separator(),
                            course_task_entries.empty()
                                ? text("No tasks")
                                : course_task_menu->Render(),
                            separator(),
                            text("q = deselect, d = delete course") | dim,
                        }) |
                    border;
    } else if (selected_filter == 1) {
      right_panel = vbox({
                        text("Add course") | bold,
                        separator(),
                        name_input->Render(),
                        text("Colour") | bold,
                        vbox(colour_rows),
                        add_button->Render(),
                    }) |
                    border;
    } else if (selected_filter == 2) {
      right_panel = text("Returning...");
    } else {
      right_panel =
          vbox({
              text("Courses") | bold,
              separator(),
              course_entries.empty() ? text("No courses") : vbox(course_rows),
                separator(),
                text(course_entries.empty() ? "Course tasks"
                              : "Tasks for selected course") |
                  bold,
                course_task_rows.empty() ? text("No tasks") : vbox(course_task_rows),
          }) |
          border;
    }

    return vbox({
               hbox({
                   vbox({
                       text("Courses") | bold,
                       separator(),
                       filter_menu->Render(),
                   }) | border,
                   right_panel | flex,
               }),
               text(status) | dim,
           }) |
           border;
  });

  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q') && selected_course_id != -1) {
      selected_course_id = -1;
      content->SetActiveChild(course_menu);
      status = "Course deselected.";
      return true;
    }

    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }

    if (event == Event::Return && selected_course_id == -1 &&
      selected_filter == 0 && !course_ids.empty()) {
      selected_course_id = course_ids[selected_course];
      selected_course_task = 0;
      content->SetActiveChild(course_task_menu);
      status = "Selected course: " +
               manager.GetCourses()[selected_course].GetName();
      return true;
    }

    if (event == Event::Return && selected_course_id != -1 &&
        !course_task_ids.empty() &&
        selected_course_task < static_cast<int>(course_task_ids.size())) {
      manager.CompleteAssignmentById(course_task_ids[selected_course_task]);
      return true;
    }

    if (event == Event::Character('d') && selected_filter == 0 &&
        selected_course_id != -1) {
      manager.DeleteCourse(selected_course_id);
      selected_course_id = -1;
      content->SetActiveChild(course_menu);

      status = "Course deleted.";
      return true;
    }

    return false;
  });

  screen.Loop(app);
}

void RunInteractiveMenu(Manager& manager) {
  auto screen = ftxui::ScreenInteractive::TerminalOutput();

  std::vector<std::string> menu_entries{"Dashboard", "Tasks", "Courses",
                                        "Calendar", "Help", "Quit"};
  std::string status_msg =
      "Use UP/DOWN arrows to navigate, press ENTER to select.";
  int selected{0};

  // configure menu options
  ftxui::MenuOption option;
  option.on_enter = [&] {
    switch (selected) {
      case 0:
        status_msg = "Opening: Dashboard...";
        RunDashboard(manager);
        break;
      case 1:
        status_msg = "Opening: Tasks...";
        RunTasksMenu(manager);
        break;
      case 2:
        status_msg = "Opening: Courses...";
        RunCourseMenu(manager);
        break;
      case 3:
        status_msg = "Opening: Calendar...";
        RunCalendar(manager);
        break;
      case 4:
        status_msg = "Opening: Help...";
        RunHelp();
        break;
      case 5:
        screen.ExitLoopClosure()();
        break;  // Close terminal UI loop
    }
  };

  auto menu = ftxui::Menu(&menu_entries, &selected, option);
  auto renderer = ftxui::Renderer(menu, [&] {
    return ftxui::vbox(
               {ftxui::text(" COURSE MANAGER MENU ") | ftxui::bold |
                    ftxui::color(ftxui::Color::Cyan) | ftxui::center,
                ftxui::separator(), menu->Render(), ftxui::separator(),
                ftxui::text(status_msg) | ftxui::color(ftxui::Color::Yellow)}) |
           ftxui::border;
  });

  screen.Loop(renderer);

  std::cout << "Exited app. Final selected index: " << selected << "\n";
}

}  // namespace

int main(const int argc, const char* argv[]) {
  Manager manager;
  const int result = argc > 1 ? RunCommand(manager, argc, argv) : 0;
  if (argc == 1) {
    RunInteractiveMenu(manager);
  }

  manager.SaveData("../data/data.json");
  return result;
}
