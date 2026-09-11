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
        
        bool AddTask(const std::string& name, const std::string& course, std::string& due_date, const std::string& due_time = "" );
        void AddCourse(const std::string& name, const CourseColour colour);
        bool SaveData(const std::string& filename) const;

        void ShowAssignments() const;
        void ShowCourses() const;

        void ShowAssignmentsForDate(const std::string& date) const;
        void ShowAssignmentsForCourse(const std::string& course) const;
        void ShowAssignmentsInRange(const std::string& start_date, const std::string& end_date) const;
        
        int GetCourseIdByName(const std::string& name) const;
        const Course* GetCourseById(const int id) const;
        Assignment* GetAssignmentById(const int id);
        const Assignment* GetAssignmentById(const int id) const;

        bool CompleteAssignmentById(const int id);
    private:
        std::vector<Assignment> assignments_;
        std::vector<Course> courses_;
        int last_assignment_id_, last_course_id_;

        bool LoadData(const std::string& filename);


};

#endif