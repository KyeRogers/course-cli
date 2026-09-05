#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>
#include <vector>

#include "assignment.hpp"
#include "colours.hpp"
#include "course.hpp"

class Manager {
    public: 
        Manager();
        
        void AddTask(const std::string& name, const int course_id, const std::string& due_date);
        void AddCourse(const std::string& name, const CourseColour colour);
        bool SaveData(const std::string& filename) const;

        void ShowAssignments() const;
    private:
        std::vector<Assignment> assignments_;
        std::vector<Course> courses_;
        int last_assignment_id_, last_course_id_;

        void LoadData(const std::string& filename);


};

#endif