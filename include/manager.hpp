#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <array>
#include <string>
#include <vector>

#include "assignment.hpp"
#include "colours.hpp"
#include "course.hpp"

enum class CompletionFilter { All, Pending, Completed };
enum class AssignmentSort { DueDate, Priority, Course, Name };

struct CalendarDay {
    std::string date;
    std::vector<Assignment> assignments;
};

struct CalendarWeek {
    std::string monday;
    std::string sunday;
    std::string previous_monday;
    std::string next_monday;
    std::array<CalendarDay, 7> days;
};

class Manager {
    public: 
        Manager();
        
        bool AddTask(const std::string& name, const std::string& course,
                 std::string& due_date, const std::string& due_time = "",
                 const AssignmentPriority priority = AssignmentPriority::Normal,
                 const Recurrence recurrence = Recurrence::None);
        void AddCourse(const std::string& name, const CourseColour colour);
        bool SaveData(const std::string& filename) const;
        bool UndoLastAction();

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
        std::vector<Assignment> SearchAssignments(
            const std::vector<Assignment>& source,
            const std::string& query) const;
        std::vector<Assignment> SortAssignments(
            const std::vector<Assignment>& source,
            const AssignmentSort sort) const;
        CalendarWeek GetCalendarWeek(
            const std::vector<Assignment>& source,
            const std::string& date) const;
        Assignment* GetAssignmentById(const int id);
        const Assignment* GetAssignmentById(const int id) const;

        bool DeleteAssignmentById(const int id);
        void DeleteCourse(const int course_id);

        bool CompleteAssignmentById(const int id);
        bool UpdateAssignment(const int id, const std::string& name,
                      const std::string& course,
                      const std::string& due_date,
                      const std::string& due_time,
                      const AssignmentPriority priority,
                      const Recurrence recurrence = Recurrence::None);
        int RemoveCompletedOlderThanDays(const int days);
    private:
        struct Snapshot {
            std::vector<Assignment> assignments;
            std::vector<Course> courses;
            int last_assignment_id;
            int last_course_id;
        };

        std::vector<Assignment> assignments_;
        std::vector<Course> courses_;
        int last_assignment_id_, last_course_id_;
        std::vector<Snapshot> undo_stack_;

        void CaptureState();
        bool LoadData(const std::string& filename);


};

#endif