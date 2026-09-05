#ifndef COLOURS_HPP
#define COLOURS_HPP

#include <string>
#include <string_view>

enum class CourseColour {
    Default,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightBlack,
    BrightBlue
};


namespace Colors {

    inline std::string_view to_ansi(CourseColour color) {
        switch (color) {
            case CourseColour::Red:         return "\033[31m";
            case CourseColour::Green:       return "\033[32m";
            case CourseColour::Yellow:      return "\033[33m";
            case CourseColour::Blue:        return "\033[34m";
            case CourseColour::Magenta:     return "\033[35m";
            case CourseColour::Cyan:        return "\033[36m";
            case CourseColour::White:       return "\033[37m";
            case CourseColour::BrightBlack: return "\033[90m"; 
            case CourseColour::BrightBlue:  return "\033[94m";
            case CourseColour::Default:
            default:                       return "\033[0m";  
        }
    }

    inline CourseColour from_string(const std::string& color_str) {
        if (color_str == "Red" || color_str == "red")         return CourseColour::Red;
        if (color_str == "Green" || color_str == "green")       return CourseColour::Green;
        if (color_str == "Yellow" || color_str == "yellow")      return CourseColour::Yellow;
        if (color_str == "Blue" || color_str == "blue")        return CourseColour::Blue;
        if (color_str == "Magenta" || color_str == "magenta")     return CourseColour::Magenta;
        if (color_str == "Cyan" || color_str == "cyan")        return CourseColour::Cyan;
        if (color_str == "White" || color_str == "white")       return CourseColour::White;
        if (color_str == "Gray" || color_str == "gray")        return CourseColour::BrightBlack;
        if (color_str == "BrightBlue" || color_str == "brightbluej")  return CourseColour::BrightBlue;
        
        return CourseColour::Default;
    }

    inline std::string to_string(CourseColour color) {
        switch (color) {
            case CourseColour::Red:         return "Red";
            case CourseColour::Green:       return "Green";
            case CourseColour::Yellow:      return "Yellow";
            case CourseColour::Blue:        return "Blue";
            case CourseColour::Magenta:     return "Magenta";
            case CourseColour::Cyan:        return "Cyan";
            case CourseColour::White:       return "White";
            case CourseColour::BrightBlack: return "Gray";
            case CourseColour::BrightBlue:  return "BrightBlue";
            default:                       return "Default";
        }
    }

} 

#endif 