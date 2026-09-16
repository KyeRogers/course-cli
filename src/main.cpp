#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <iostream>
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
      << "  course-cli today           Show today's assignments\n"
      << "  course-cli tomorrow        Show tomorrow's assignments\n"
      << "  course-cli week            Show the next seven days\n"
      << "  course-cli course NAME     Show assignments for a course\n"
      << "  course-cli add_task NAME COURSE DATE [TIME]\n"
      << "  course-cli add_course NAME COLOUR\n"
      << "  course-cli done ID         Toggle an assignment's completion\n";
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

int RunCommand(Manager& manager, const int argc, const char* argv[]) {
  const std::string command{argv[1]};
  if (command == "today") {
    manager.ShowAssignmentsForDate("today");
    return 0;
  }
  if (command == "tomorrow") {
    manager.ShowAssignmentsForDate("tomorrow");
    return 0;
  }
  if (command == "week") {
    manager.ShowAssignmentsInRange("today", "week");
    return 0;
  }
  if (command == "course" && argc == 3) {
    manager.ShowAssignmentsForCourse(argv[2]);
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

void RunDashboard(Manager& manager) {}

void RunTasksMenu(Manager& manager) {
  using namespace ftxui;

  auto screen = ScreenInteractive::TerminalOutput();
  std::vector<std::string> filters{"Pending", "Completed", "Add task", "Back"};
  int selected_filter = 0;
  int selected_task = 0;

  std::vector<std::string> task_entries;
  std::vector<int> task_ids;
  std::string task_name;
  std::string task_date;
  std::string task_time;
  std::vector<std::string> course_options;
  int selected_course = 0;
  bool course_selected = false;
  std::string status = "Select a task and press Enter to mark it completed.";

  MenuOption filter_option;
  filter_option.on_enter = [&] {
    if (selected_filter == 3) {
      screen.Exit();
    }
  };

  auto filter_menu = Menu(&filters, &selected_filter, filter_option);
  auto task_menu = Menu(&task_entries, &selected_task);
  auto name_input = Input(&task_name, "Task name");
  auto course_menu = Menu(&course_options, &selected_course);

  auto course_selector = CatchEvent(course_menu, [&](const Event& event) {
    if (event == Event::Character(' ') &&
        selected_course < static_cast<int>(manager.GetCourses().size())) {
      course_selected = true;
      return true;
    }
    return false;
  });

  auto date_input = Input(&task_date, "Due date (YYYY-MM-DD)");
  auto time_input = Input(&task_time, "Due time (optional)");

  auto add_button = Button("Add task", [&] {
    if (task_name.empty() || !course_selected || task_date.empty()) {
      status = "Name, course, and date are required.";
      return;
    }

    if (manager.AddTask(task_name,
                        manager.GetCourses()[selected_course].GetName(),
                        task_date, task_time)) {
      task_name.clear();
      task_date.clear();
      task_time.clear();
      course_selected = false;
      selected_filter = 0;
      status = "Task added.";
    } else {
      status = "That course does not exist.";
    }
  });

  auto form = Container::Vertical({
      name_input,
      course_selector,
      date_input,
      time_input,
      add_button,
  });

  auto left_panel = Container::Vertical({
      filter_menu,
      form,
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

      if (selected_filter == 2) {
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
    task_entries.clear();
    task_ids.clear();
    course_options.clear();
    Elements task_rows;

    for (std::size_t index = 0; index < manager.GetCourses().size(); ++index) {
      const Course& course = manager.GetCourses()[index];
      course_options.push_back(
          std::string(index == static_cast<std::size_t>(selected_course) &&
                              course_selected
                          ? "[x] "
                          : "[ ] ") +
          course.GetName());
    }

    for (const Assignment& assignment : manager.GetAssignments()) {
      const bool show_task =
          (selected_filter == 0 && !assignment.GetCompleted()) ||
          (selected_filter == 1 && assignment.GetCompleted());

      if (show_task) {
        task_ids.push_back(assignment.GetId());
        task_entries.push_back(
            std::string(assignment.GetCompleted() ? "[x] " : "[ ] ") +
            assignment.GetName() + "  " + assignment.GetDueDate());

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

    if (selected_filter == 2) {
      right_panel = vbox({
                        text("Add task") | bold,
                        separator(),
                        name_input->Render(),
                        text("Course (press Space to select)") | bold,
                        course_selector->Render(),
                        date_input->Render(),
                        time_input->Render(),
                        add_button->Render(),
                    }) |
                    border;
    } else if (selected_filter == 3) {
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
  std::string course_name;
  int selected_course_id = -1;
  std::string status = "Use arrows to navigate, q to quit, and press ENTER to select.";


  MenuOption filter_option;
  filter_option.on_enter = [&] {
    if (selected_filter == 2) {
      screen.Exit();
    }
  };

  auto filter_menu = Menu(&filters, &selected_filter, filter_option);
  auto course_menu = Menu(&course_entries, &selected_course);
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
  });

  auto navigation = CatchEvent(content, [&](const Event& event) {
    if (event == Event::ArrowRight) {
      content->SetActiveChild(course_menu);
      return true;
    }

    if (event == Event::ArrowLeft) {
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
    colour_entries.clear();
    Elements course_rows;
    Elements colour_rows;

    for (std::size_t index = 0; index < colour_options.size(); ++index) {
      colour_entries.push_back(
          std::string(index == static_cast<std::size_t>(chosen_colour)
                          ? "[x] "
                          : "[ ] ") +
          colour_options[index]);

      Element row = text(colour_entries.back()) |
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

      Element row = text(course_entries.back()) | color(ToUiColor(course.GetColour()));
      if (course_rows.size() == static_cast<std::size_t>(selected_course)) {
        row = row | inverted;
      }
      course_rows.push_back(row);
    }

    if (selected_course >= static_cast<int>(course_entries.size())) {
      selected_course = course_entries.empty() ? 0 : course_entries.size() - 1;
    }

    Element right_panel;
    if (selected_filter == 1) {
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
      right_panel = vbox({
                        text("Courses") | bold,
                        separator(),
                        course_entries.empty() ? text("No courses")
                                               : vbox(course_rows),
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
    if (event == Event::Character('q')) {
      screen.Exit();
      return true;
    }

    if (event == Event::Return && selected_filter == 0 && !course_ids.empty()) {
      selected_course_id = course_ids[selected_course];
      status = "Selected course: " +
               manager.GetCourses()[selected_course].GetName() +
               " - press d to delete";
      return true;
    }

    if (event == Event::Character('d') && selected_filter == 0 &&
        selected_course_id != -1) {
      manager.DeleteCourse(selected_course_id);
      selected_course_id = -1;

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
                                        "Calendar", "Quit"};
  std::string status_msg =
      "Use UP/DOWN arrows to navigate, press ENTER to select.";
  int selected{0};

  // configure menu options
  ftxui::MenuOption option;
  option.on_enter = [&] {
    switch (selected) {
      case 0:
        status_msg = "Opening: Dashoard...";
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
        break;
      case 4:
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
