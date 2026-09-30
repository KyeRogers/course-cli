#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <string>
#include <vector>

#include "assignment.hpp"
#include "colours.hpp"
#include "course.hpp"

enum class CompletionFilter { All, Pending, Completed };

class Manager {
    public: 
        Manager();
        
        bool AddTask(const std::string& name, const std::string& course,
                 std::string& due_date, const std::string& due_time = "",
                 const AssignmentPriority priority = AssignmentPriority::Normal);
        void AddCourse(const std::string& name, const CourseColour colour);
        bool SaveData(const std::string& filename) const;

        int GetCourseIdByName(const std::string& name) const;
        const Course* GetCourseById(const int id) const;
        const std::vector<Course>& GetCourses() const;
        const std::vector<Assignment>& GetAssignments() const;
        std::vector<Assignment> FilterAssignmentsByCompletion(
            const std::vector<Assignment>& source,
            const CompletionFilter completion) const;
        std::vector<Assignment> FilterAssignmentsForDate(
            const std::vector<Assignment>& source,
            const std::string& date) const;
        std::vector<Assignment> FilterAssignmentsInRange(
            const std::vector<Assignment>& source,
            const std::string& start_date,
            const std::string& end_date) const;
        std::vector<Assignment> FilterAssignmentsBeforeDate(
            const std::vector<Assignment>& source,
            const std::string& date) const;
        std::vector<Assignment> FilterAssignmentsAfterDate(
            const std::vector<Assignment>& source,
            const std::string& date) const;
        std::vector<Assignment> FilterAssignmentsForCourse(
            const std::vector<Assignment>& source,
            const std::string& course) const;
        Assignment* GetAssignmentById(const int id);
        const Assignment* GetAssignmentById(const int id) const;

        void DeleteCourse(const int course_id);

        bool CompleteAssignmentById(const int id);
        bool UpdateAssignment(const int id, const std::string& name,
                      const std::string& course,
                      const std::string& due_date,
                      const std::string& due_time,
                      const AssignmentPriority priority);
        int RemoveCompletedOlderThanDays(const int days);
    private:
        std::vector<Assignment> assignments_;
        std::vector<Course> courses_;
        int last_assignment_id_, last_course_id_;

        bool LoadData(const std::string& filename);


};

#endif