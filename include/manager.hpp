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

        // TODO: implement a date-aware view mode for the next day / next week.
        void ShowAssignmentsForDate(const std::string& date) const;
        void ShowAssignmentsForCourse(const std::string& course) const;
        void ShowAssignmentsInRange(const std::string& start_date, const std::string& end_date) const;
        // TODO: add functions like GetAssignmentsForDay(date), GetAssignmentsInRange(start, end),
        //       and ShowCalendarForDays(int days).
        // TODO: sort assignments by date before rendering the calendar view.
        // TODO: allow filtering by course name in the date-range output.
        
        int GetCourseIdByName(const std::string& name) const;
        const Course* GetCourseById(const int id) const;
        Assignment* GetAssignmentById(const int id);
        const Assignment* GetAssignmentById(const int id) const;

        void CompleteAssignmentById(const int id);
    private:
        std::vector<Assignment> assignments_;
        std::vector<Course> courses_;
        int last_assignment_id_, last_course_id_;

        bool LoadData(const std::string& filename);


};

#endif