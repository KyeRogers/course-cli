#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

int main() {
  using namespace ftxui;

  std::vector<std::string> menu_entries{
      "Today",
      "All assignments",
      "Courses",
      "Quit",
  };
  int selected = 0;

  auto menu = Menu(&menu_entries, &selected);
  auto screen = ScreenInteractive::TerminalOutput();

  auto renderer = Renderer(menu, [&] {
    return vbox({
               text("Course CLI") | bold | center,
               separator(),
               menu->Render(),
               separator(),
               text("Use the arrow keys and Enter. Press q to quit.") | dim,
           }) |
           border | size(WIDTH, GREATER_THAN, 40);
  });

  auto app = CatchEvent(renderer, [&](const Event& event) {
    if (event == Event::Character('q') || selected == 3) {
      screen.Exit();
      return true;
    }
    return false;
  });

  screen.Loop(app);
}