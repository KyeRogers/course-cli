#include "../include/ui.hpp"

#include <array>
#include <ctime>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "../include/manager.hpp"

namespace {

using namespace ftxui;

struct DateValue {
  int year;
  int month;
  int day;
};

struct TaskFormState {
  std::string name;
  std::string date;
  std::string time;
  std::vector<std::string> courses;
  std::vector<std::string> priorities{"!", "-", "."};
  int selected_course = 0;
  int confirmed_course = -1;
  int selected_priority = 1;
  int confirmed_priority = 1;
  std::string message;
};

Color ToUiColor(const CourseColour value) {
  switch (value) {
    case CourseColour::Red: return Color::Red;
    case CourseColour::Green: return Color::Green;
    case CourseColour::Yellow: return Color::Yellow;
    case CourseColour::Blue: return Color::Blue;
    case CourseColour::Magenta: return Color::Magenta;
    case CourseColour::Cyan: return Color::Cyan;
    case CourseColour::White: return Color::White;
    case CourseColour::BrightBlack: return Color::GrayDark;
    case CourseColour::BrightBlue: return Color::BlueLight;
    case CourseColour::Default: return Color::Default;
  }
  return Color::Default;
}

DateValue Today() {
  const std::time_t now = std::time(nullptr);
  const std::tm* local = std::localtime(&now);
  return {local->tm_year + 1900, local->tm_mon + 1, local->tm_mday};
}

std::string IsoDate(const DateValue& date) {
  std::ostringstream output;
  output << std::setfill('0') << std::setw(4) << date.year << '-'
         << std::setw(2) << date.month << '-'
         << std::setw(2) << date.day;
  return output.str();
}

std::string ReadableDate(const std::string& value) {
  if (value.size() != 10) return value;
  std::tm date{};
  date.tm_year = std::stoi(value.substr(0, 4)) - 1900;
  date.tm_mon = std::stoi(value.substr(5, 2)) - 1;
  date.tm_mday = std::stoi(value.substr(8, 2));
  date.tm_hour = 12;
  std::mktime(&date);
  char output[64]{};
  std::strftime(output, sizeof(output), "%A %d %B", &date);
  return output;
}

std::string ShortDate(const DateValue& date) {
  static const std::array<const char*, 12> months{
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  return std::to_string(date.day) + " " + months[date.month - 1];
}

std::string Weekday(const DateValue& date) {
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

std::string PrioritySymbol(const AssignmentPriority priority) {
  switch (priority) {
    case AssignmentPriority::High: return "!";
    case AssignmentPriority::Low: return ".";
    case AssignmentPriority::Normal: return "-";
  }
  return "-";
}

Element TaskRow(const Assignment& task, const Course* course,
                const bool selected = false) {
  Element row = text(std::string(task.GetCompleted() ? "[x] " : "[ ] ") +
                     PrioritySymbol(task.GetPriority()) + " " +
                     task.GetName() + "  " + ReadableDate(task.GetDueDate()) +
                     (course == nullptr ? "" : "  " + course->GetName())) |
                color(course == nullptr ? Color::Default
                                        : ToUiColor(course->GetColour()));
  if (task.GetCompleted()) row = row | dim;
  if (selected) row = row | inverted;
  return row;
}

Component MakeTaskForm(Manager& manager,
                       const std::optional<std::string>& fixed_date,
                       const std::optional<int>& edit_id,
                       const std::function<void()>& on_saved) {
  auto state = std::make_shared<TaskFormState>();
  if (edit_id.has_value()) {
    const Assignment* task = manager.GetAssignmentById(*edit_id);
    if (task != nullptr) {
      state->name = task->GetName();
      state->date = task->GetDueDate();
      state->time = task->GetDueTime();
      for (std::size_t index = 0; index < manager.GetCourses().size(); ++index) {
        if (manager.GetCourses()[index].GetId() == task->GetCourseId()) {
          state->selected_course = static_cast<int>(index);
          state->confirmed_course = static_cast<int>(index);
          break;
        }
      }
      state->selected_priority = task->GetPriority() == AssignmentPriority::High
                                     ? 0
                                     : task->GetPriority() == AssignmentPriority::Low
                                           ? 2
                                           : 1;
      state->confirmed_priority = state->selected_priority;
    }
  }
  if (fixed_date.has_value()) state->date = *fixed_date;
  for (const Course& course : manager.GetCourses()) {
    state->courses.push_back(course.GetName());
  }

  auto name_input = Input(&state->name, "Task name");
  auto date_input = Input(&state->date, "Due date YYYY-MM-DD");
  auto time_input = Input(&state->time, "Time (optional)");

  MenuOption course_option;
  course_option.entries_option.transform = [state, &manager](
                                               const EntryState& entry) {
    const Course& course = manager.GetCourses()[entry.index];
    Element row = text(std::string(entry.index == state->confirmed_course
                                       ? "[x] " : "[ ] ") + course.GetName()) |
                      color(ToUiColor(course.GetColour()));
    if (entry.focused) row = row | inverted;
    return row;
  };
  auto course_menu = Menu(&state->courses, &state->selected_course, course_option);
  auto course_selector = CatchEvent(course_menu, [state](const Event& event) {
    if (event == Event::Character(' ') && !state->courses.empty()) {
      state->confirmed_course = state->selected_course;
      return true;
    }
    return false;
  });

  MenuOption priority_option;
  priority_option.entries_option.transform = [state](const EntryState& entry) {
    Element row = text(std::string(entry.index == state->confirmed_priority
                                       ? "[x] " : "[ ] ") +
                       state->priorities[entry.index]);
    if (entry.focused) row = row | inverted;
    return row;
  };
  auto priority_menu =
      Menu(&state->priorities, &state->selected_priority, priority_option);
  auto priority_selector = CatchEvent(priority_menu, [state](const Event& event) {
    if (event == Event::Character(' ')) {
      state->confirmed_priority = state->selected_priority;
      return true;
    }
    return false;
  });

  auto save_button = Button(edit_id.has_value() ? "Save changes" : "Add task",
                            [state, &manager, fixed_date, edit_id, on_saved] {
    const std::string due_date = fixed_date.has_value() ? *fixed_date : state->date;
    if (state->name.empty() || due_date.empty() || state->confirmed_course < 0 ||
        state->confirmed_course >= static_cast<int>(manager.GetCourses().size())) {
      state->message = "Enter a name, date, and confirm a course with Space.";
      return;
    }
    const AssignmentPriority priority = state->confirmed_priority == 0
                                           ? AssignmentPriority::High
                                           : state->confirmed_priority == 2
                                                 ? AssignmentPriority::Low
                                                 : AssignmentPriority::Normal;
    bool saved = false;
    if (edit_id.has_value()) {
      saved = manager.UpdateAssignment(
          *edit_id, state->name,
          manager.GetCourses()[state->confirmed_course].GetName(), due_date,
          state->time, priority);
    } else {
      std::string mutable_date = due_date;
      saved = manager.AddTask(
          state->name, manager.GetCourses()[state->confirmed_course].GetName(),
          mutable_date, state->time, priority);
    }
    if (!saved) {
      state->message = "Could not save task.";
      return;
    }
    if (on_saved) on_saved();
  });

  std::vector<Component> fields{name_input, course_selector, priority_selector};
  if (!fixed_date.has_value()) fields.push_back(date_input);
  fields.push_back(time_input);
  fields.push_back(save_button);
  auto form = Container::Vertical(std::move(fields));
  return Renderer(form, [state, edit_id, fixed_date, name_input, course_selector,
                         priority_selector, date_input, time_input, save_button] {
    return vbox({text(edit_id.has_value() ? "Edit task" : "Add task") | bold,
                 separator(), name_input->Render(), text("Course (Space confirms)"),
                 course_selector->Render(), text("Priority (Space confirms)"),
                 priority_selector->Render(),
                 fixed_date.has_value() ? text("Due: " + ReadableDate(*fixed_date))
                                        : date_input->Render(),
                 time_input->Render(), save_button->Render(),
                 text(state->message) | color(Color::Yellow)}) | border;
  });
}

void RunTaskForm(Manager& manager, const std::optional<std::string>& date = {},
                 const std::optional<int>& edit_id = {}) {
  auto screen = ScreenInteractive::TerminalOutput();
  auto panel = MakeTaskForm(manager, date, edit_id, [&] { screen.Exit(); });
  auto app = CatchEvent(panel, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.Exit();
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

void RunCleanup(Manager& manager) {
  auto screen = ScreenInteractive::TerminalOutput();
  std::string age_text;
  std::string message;
  std::vector<std::string> units{"Days", "Weeks"};
  int selected_unit = 0;
  int confirmed_unit = 0;
  auto age_input = Input(&age_text, "Age");
  MenuOption unit_option;
  unit_option.entries_option.transform = [&](const EntryState& entry) {
    Element row = text(std::string(entry.index == confirmed_unit ? "[x] " : "[ ] ") +
                       units[entry.index]);
    if (entry.focused) row = row | inverted;
    return row;
  };
  auto unit_menu = Menu(&units, &selected_unit, unit_option);
  auto unit_selector = CatchEvent(unit_menu, [&](const Event& event) {
    if (event == Event::Character(' ')) {
      confirmed_unit = selected_unit;
      return true;
    }
    return false;
  });
  auto remove_button = Button("Remove old completed tasks", [&] {
    try {
      std::size_t parsed = 0;
      const int amount = std::stoi(age_text, &parsed);
      if (parsed != age_text.size() || amount < 0) {
        message = "Enter a non-negative whole number.";
        return;
      }
      const int removed = manager.RemoveCompletedOlderThanDays(
          confirmed_unit == 1 ? amount * 7 : amount);
      message = "Removed " + std::to_string(removed) + " task(s).";
    } catch (const std::exception&) {
      message = "Enter a valid age.";
    }
  });
  auto form = Container::Vertical({age_input, unit_selector, remove_button});
  auto renderer = Renderer(form, [&] {
    return vbox({text("Cleanup completed tasks") | bold, separator(),
                 age_input->Render(), text("Unit (Space confirms)"),
                 unit_selector->Render(), remove_button->Render(),
                 text(message) | color(Color::Yellow),
                 text("Enter removes  q/Esc back") | dim}) | border;
  });
  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Escape || event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

void RunAddCourse(Manager& manager) {
  auto screen = ScreenInteractive::TerminalOutput();
  std::string name;
  std::vector<std::string> colours{"Red", "Green", "Yellow", "Blue",
                                   "Magenta", "Cyan", "White"};
  int selected = 0;
  int confirmed = -1;
  auto name_input = Input(&name, "Course name");
  auto colour_menu = Menu(&colours, &selected);
  auto colour_selector = CatchEvent(colour_menu, [&](const Event& event) {
    if (event == Event::Character(' ')) {
      confirmed = selected;
      return true;
    }
    return false;
  });
  auto add = Button("Add course", [&] {
    if (!name.empty() && confirmed >= 0) {
      manager.AddCourse(name, Colors::from_string(colours[confirmed]));
      screen.Exit();
    }
  });
  auto form = Container::Vertical({name_input, colour_selector, add});
  auto renderer = Renderer(form, [&] {
    Elements options;
    for (std::size_t index = 0; index < colours.size(); ++index) {
      Element row = text(std::string(static_cast<int>(index) == confirmed
                                         ? "[x] " : "[ ] ") + colours[index]) |
                        color(ToUiColor(Colors::from_string(colours[index])));
      if (static_cast<int>(index) == selected) row = row | inverted;
      options.push_back(row);
    }
    return vbox({text("Add course") | bold, name_input->Render(),
                 text("Course colour (Space confirms)"), vbox(options),
                 add->Render()}) | border;
  });
  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Escape || event == Event::Character('q')) {
      screen.Exit();
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

void RunHelp() {
  auto screen = ScreenInteractive::TerminalOutput();
  auto page = Renderer([&] {
    return vbox({text("COURSE MANAGER") | bold | color(Color::Cyan) | center,
                 text("Keyboard guide") | dim | center, separator(),
                 text("Dashboard   Left/Right sections, Up/Down tasks, Enter toggles"),
                 text("Tasks       Left/Right status, Up/Down tasks, a add, e edit"),
                 text("Courses     Enter opens, q returns, d deletes"),
                 text("Calendar    Left/Right day, Up/Down tasks, a add, e edit"),
                 separator(),
                 text("Space confirms a choice     ? help     q / Esc back") | dim}) |
           border;
  });
  auto app = CatchEvent(page, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.Exit();
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

void RunDashboard(Manager& manager) {
  auto screen = ScreenInteractive::TerminalOutput();
  const std::array<std::string, 4> names{"Overdue", "Today", "Upcoming", "Completed"};
  int section = 0;
  int selected = 0;
  std::vector<int> ids;
  auto renderer = Renderer([&] {
    const auto& all = manager.GetAssignments();
    const auto pending_tasks = manager.FilterAssignmentsByCompletion(
      all, CompletionFilter::Pending);
    const auto completed_tasks = manager.FilterAssignmentsByCompletion(
      all, CompletionFilter::Completed);
    std::array<std::vector<Assignment>, 4> groups{
      manager.FilterAssignmentsBeforeDate(pending_tasks, "today"),
      manager.FilterAssignmentsForDate(pending_tasks, "today"),
      manager.FilterAssignmentsAfterDate(pending_tasks, "today"),
      completed_tasks};
    const auto all_today = manager.FilterAssignmentsForDate(all, "today");
    const auto all_week = manager.FilterAssignmentsInRange(all, "today", "week");
    const auto today_done = manager.FilterAssignmentsByCompletion(
      all_today, CompletionFilter::Completed);
    const auto week_done = manager.FilterAssignmentsByCompletion(
      all_week, CompletionFilter::Completed);
    const int pending = static_cast<int>(pending_tasks.size());
    const int completed = static_cast<int>(completed_tasks.size());
    const int due_today = static_cast<int>(all_today.size());
    const int done_today = static_cast<int>(today_done.size());
    const int due_week = static_cast<int>(all_week.size());
    const int done_week = static_cast<int>(week_done.size());
    if (groups[2].size() > 5) groups[2].resize(5);
    ids.clear();
    Elements rows;
    for (const Assignment& task : groups[section]) {
      ids.push_back(task.GetId());
      rows.push_back(TaskRow(task, manager.GetCourseById(task.GetCourseId()),
                             rows.size() == static_cast<std::size_t>(selected)));
    }
    if (selected >= static_cast<int>(ids.size())) selected = ids.empty() ? 0 : ids.size() - 1;
    return vbox({text("Dashboard") | bold | color(Color::Cyan),
                 text(ReadableDate(IsoDate(Today()))) | dim, separator(),
                 hbox({text("Pending ") | bold, text(std::to_string(pending)),
                       text("    Completed ") | bold,
                       text(std::to_string(completed))}),
                 text("Today: " + std::to_string(done_today) + "/" +
                      std::to_string(due_today) + "    Week: " +
                      std::to_string(done_week) + "/" + std::to_string(due_week)),
                 separator(),
                 hbox({text((section == 0 ? "> " : "  ") + names[0] + " " +
                            std::to_string(groups[0].size())),
                       text("   " + std::string(section == 1 ? "> " : "  ") +
                            names[1] + " " + std::to_string(groups[1].size())),
                       text("   " + std::string(section == 2 ? "> " : "  ") +
                            names[2] + " " + std::to_string(groups[2].size())),
                       text("   " + std::string(section == 3 ? "> " : "  ") +
                            names[3] + " " + std::to_string(groups[3].size()))}),
                 rows.empty() ? text("No tasks") : vbox(rows), separator(),
                 text("Left/Right section   Up/Down task   Enter toggle   ? help   q back") | dim}) |
           border;
  });
  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) { screen.Exit(); return true; }
    if (event == Event::Character('?')) { RunHelp(); return true; }
    if (event == Event::ArrowLeft) { if (section > 0) --section; selected = 0; return true; }
    if (event == Event::ArrowRight) { if (section < 3) ++section; selected = 0; return true; }
    if (event == Event::ArrowUp) { if (selected > 0) --selected; return true; }
    if (event == Event::ArrowDown) { if (selected + 1 < static_cast<int>(ids.size())) ++selected; return true; }
    if (event == Event::Return && !ids.empty()) { manager.CompleteAssignmentById(ids[selected]); return true; }
    return false;
  });
  screen.Loop(app);
}

void RunTasks(Manager& manager) {
  auto screen = ScreenInteractive::TerminalOutput();
  bool completed_view = false;
  int selected = 0;
  std::vector<int> ids;
  auto renderer = Renderer([&] {
    ids.clear();
    Elements rows;
    const auto tasks = manager.FilterAssignmentsByCompletion(
        manager.GetAssignments(), completed_view ? CompletionFilter::Completed
                                                 : CompletionFilter::Pending);
    for (const Assignment& task : tasks) {
      ids.push_back(task.GetId());
      rows.push_back(TaskRow(task, manager.GetCourseById(task.GetCourseId()),
                             rows.size() == static_cast<std::size_t>(selected)));
    }
    if (selected >= static_cast<int>(ids.size())) selected = ids.empty() ? 0 : ids.size() - 1;
    return vbox({text("Tasks") | bold, separator(),
                 text((completed_view ? "Completed" : "Pending") +
                      std::string("  [Left/Right to switch]")) | bold,
                 rows.empty() ? text("No tasks") : vbox(rows), separator(),
                 text("Up/Down select  Enter toggle  a add  e edit  c cleanup  ? help  q back") | dim}) |
           border;
  });
  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q') || event == Event::Escape) { screen.Exit(); return true; }
    if (event == Event::Character('?')) { RunHelp(); return true; }
    if (event == Event::ArrowLeft || event == Event::ArrowRight) { completed_view = !completed_view; selected = 0; return true; }
    if (event == Event::ArrowUp) { if (selected > 0) --selected; return true; }
    if (event == Event::ArrowDown) { if (selected + 1 < static_cast<int>(ids.size())) ++selected; return true; }
    if (event == Event::Character('a')) { RunTaskForm(manager); selected = 0; return true; }
    if (event == Event::Character('e') && !ids.empty()) { RunTaskForm(manager, {}, ids[selected]); return true; }
    if (event == Event::Character('c')) { RunCleanup(manager); return true; }
    if (event == Event::Return && !ids.empty()) { manager.CompleteAssignmentById(ids[selected]); return true; }
    return false;
  });
  screen.Loop(app);
}

void RunCourses(Manager& manager) {
  auto screen = ScreenInteractive::TerminalOutput();
  int course_index = 0;
  int task_index = 0;
  bool detail = false;
  std::vector<int> task_ids;
  auto renderer = Renderer([&] {
    Elements courses;
    Elements tasks;
    const auto& all_courses = manager.GetCourses();
    if (course_index >= static_cast<int>(all_courses.size())) course_index = all_courses.empty() ? 0 : all_courses.size() - 1;
    task_ids.clear();
    const Course* course = all_courses.empty() ? nullptr : &all_courses[course_index];
    for (std::size_t i = 0; i < all_courses.size(); ++i) {
      Element row = text((i == static_cast<std::size_t>(course_index) ? "> " : "  ") +
                         all_courses[i].GetName()) | color(ToUiColor(all_courses[i].GetColour()));
      if (detail && i == static_cast<std::size_t>(course_index)) row = row | inverted;
      courses.push_back(row);
    }
    if (course != nullptr) {
      const auto course_tasks = manager.FilterAssignmentsForCourse(
          manager.GetAssignments(), course->GetName());
      for (const Assignment& task : course_tasks) {
        task_ids.push_back(task.GetId());
        tasks.push_back(TaskRow(task, course,
                                detail && tasks.size() == static_cast<std::size_t>(task_index)));
      }
    }
    if (task_index >= static_cast<int>(task_ids.size())) task_index = task_ids.empty() ? 0 : task_ids.size() - 1;
    return vbox({text(detail && course ? course->GetName() + " tasks" : "Courses") | bold,
                 separator(), detail ? (tasks.empty() ? text("No tasks") : vbox(tasks))
                                     : (courses.empty() ? text("No courses") : vbox(courses)),
                 separator(), text(detail ? "Up/Down tasks  Enter toggle  e edit  q courses  d delete course"
                                          : "Up/Down courses  Enter open  a add  d delete  ? help  q back") | dim}) |
           border;
  });
  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('?')) { RunHelp(); return true; }
    if (event == Event::Character('q') || event == Event::Escape) {
      if (detail) { detail = false; task_index = 0; }
      else screen.Exit();
      return true;
    }
    if (event == Event::ArrowUp) {
      if (detail) { if (task_index > 0) --task_index; }
      else if (course_index > 0) --course_index;
      return true;
    }
    if (event == Event::ArrowDown) {
      if (detail) { if (task_index + 1 < static_cast<int>(task_ids.size())) ++task_index; }
      else if (course_index + 1 < static_cast<int>(manager.GetCourses().size())) ++course_index;
      return true;
    }
    if (event == Event::Character('a') && !detail) { RunAddCourse(manager); return true; }
    if (event == Event::Character('d') && !manager.GetCourses().empty()) {
      const int id = manager.GetCourses()[course_index].GetId();
      manager.DeleteCourse(id);
      detail = false;
      if (manager.GetCourses().empty()) course_index = 0;
      else course_index = std::min(course_index,
                   static_cast<int>(manager.GetCourses().size()) - 1);
      return true;
    }
    if (event == Event::Character('e') && detail && !task_ids.empty()) {
      RunTaskForm(manager, {}, task_ids[task_index]);
      return true;
    }
    if (event == Event::Return) {
      if (!detail && !manager.GetCourses().empty()) { detail = true; task_index = 0; }
      else if (detail && !task_ids.empty()) manager.CompleteAssignmentById(task_ids[task_index]);
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

void RunCalendar(Manager& manager) {
  auto screen = ScreenInteractive::TerminalOutput();
  std::string selected_date = IsoDate(Today());
  std::size_t day = 0;
  int task = -1;
  CalendarWeek displayed_week;
  auto renderer = Renderer([&] {
    const auto pending = manager.FilterAssignmentsByCompletion(
        manager.GetAssignments(), CompletionFilter::Pending);
    displayed_week = manager.GetCalendarWeek(pending, selected_date);
    for (std::size_t index = 0; index < displayed_week.days.size(); ++index) {
      if (displayed_week.days[index].date == selected_date) {
        day = index;
        break;
      }
    }

    Elements columns;
    for (std::size_t offset = 0; offset < displayed_week.days.size(); ++offset) {
      const CalendarDay& calendar_day = displayed_week.days[offset];
      const DateValue date{std::stoi(calendar_day.date.substr(0, 4)),
                           std::stoi(calendar_day.date.substr(5, 2)),
                           std::stoi(calendar_day.date.substr(8, 2))};
      columns.push_back(
          vbox({text((offset == day ? "> " : "  ") + Weekday(date)) | bold,
                text(ShortDate(date)),
                text(std::to_string(calendar_day.assignments.size()) + " tasks") |
                    dim}) |
          (offset == day ? inverted : nothing) | flex);
    }

    const auto& selected_tasks = displayed_week.days[day].assignments;
    if (task >= static_cast<int>(selected_tasks.size())) {
      task = selected_tasks.empty() ? -1 : static_cast<int>(selected_tasks.size()) - 1;
    }
    Elements rows;
    for (std::size_t index = 0; index < selected_tasks.size(); ++index) {
      const Assignment& assignment = selected_tasks[index];
      rows.push_back(TaskRow(assignment,
                             manager.GetCourseById(assignment.GetCourseId()),
                             static_cast<int>(index) == task));
    }

    return vbox({text("Weekly calendar") | bold, separator(),
                 text(ReadableDate(displayed_week.monday) + " to " +
                      ReadableDate(displayed_week.sunday)) | dim,
                 hbox(columns) | border, separator(),
                 text("Selected day: " + ReadableDate(selected_date)) | bold,
                 rows.empty() ? text("No tasks") : vbox(rows), separator(),
                 text("Left/Right day  Up/Down task  a add  e edit  Enter toggle  ? help  q back") |
                     dim}) |
           border;
  });
  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('?')) { RunHelp(); return true; }
    if (event == Event::Character('q') || event == Event::Escape) { screen.Exit(); return true; }
    if (event == Event::ArrowLeft) {
      if (day > 0) {
        selected_date = displayed_week.days[day - 1].date;
      } else {
        const CalendarWeek previous = manager.GetCalendarWeek(
            manager.GetAssignments(), displayed_week.previous_monday);
        if (previous.monday >=
            manager.GetCalendarWeek(manager.GetAssignments(), IsoDate(Today())).monday) {
          selected_date = previous.days.back().date;
        }
      }
      task = -1;
      return true;
    }
    if (event == Event::ArrowRight) {
      if (day + 1 < displayed_week.days.size()) {
        selected_date = displayed_week.days[day + 1].date;
      } else {
        selected_date = displayed_week.next_monday;
      }
      task = -1;
      return true;
    }
    if (event == Event::ArrowUp) { if (task > 0) --task; else if (task == 0) task = -1; return true; }
    if (event == Event::ArrowDown) {
      if (task < static_cast<int>(displayed_week.days[day].assignments.size()) - 1) ++task;
      return true;
    }
    if (event == Event::Character('a')) {
      RunTaskForm(manager, selected_date);
      task = -1;
      return true;
    }
    if (event == Event::Character('e') && task >= 0 &&
        task < static_cast<int>(displayed_week.days[day].assignments.size())) {
      RunTaskForm(manager, {}, displayed_week.days[day].assignments[task].GetId());
      return true;
    }
    if (event == Event::Return && task >= 0 &&
        task < static_cast<int>(displayed_week.days[day].assignments.size())) {
      manager.CompleteAssignmentById(
          displayed_week.days[day].assignments[task].GetId());
      task = -1;
      return true;
    }
    return false;
  });
  screen.Loop(app);
}

}  // namespace

void RunInteractiveMenu(Manager& manager) {
  auto screen = ftxui::ScreenInteractive::TerminalOutput();
  std::vector<std::string> entries{"Dashboard", "Tasks", "Courses", "Calendar", "Help", "Quit"};
  int selected = 0;
  ftxui::MenuOption option;
  option.on_enter = [&] {
    switch (selected) {
      case 0: RunDashboard(manager); break;
      case 1: RunTasks(manager); break;
      case 2: RunCourses(manager); break;
      case 3: RunCalendar(manager); break;
      case 4: RunHelp(); break;
      case 5: screen.Exit(); break;
    }
  };
  auto menu = ftxui::Menu(&entries, &selected, option);
  auto navigation = ftxui::CatchEvent(menu, [&](const ftxui::Event& event) {
    if (event == ftxui::Event::Character('q') || event == ftxui::Event::Escape) {
      screen.Exit();
      return true;
    }
    if (event == ftxui::Event::Character('?')) {
      RunHelp();
      return true;
    }
    return false;
  });
  auto renderer = ftxui::Renderer(navigation, [&] {
    return ftxui::vbox({ftxui::text("COURSE MANAGER") | ftxui::bold |
                            ftxui::color(ftxui::Color::Cyan) | ftxui::center,
                        ftxui::separator(), navigation->Render(),
                        ftxui::text("Enter select  Up/Down navigate  q quit") | ftxui::dim}) |
           ftxui::border;
  });
  screen.Loop(renderer);
}