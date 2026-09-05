#ifndef COURSE_HPP
#define COURSE_HPP

#include <string>

#include "colours.hpp"

class Course {
    public:
        Course();
        Course(const int id, const std::string& name, const CourseColour colour);

        int GetId() const;
        const std::string& GetName() const;
        CourseColour GetColour() const;

        void Print() const;

    private:
        int id_;
        std::string name_;
        CourseColour colour_;
        
};


#endif