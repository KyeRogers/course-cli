#ifndef ASSIGNMENT_HPP
#define ASSIGNMENT_HPP

#include <string>

class Assignment {
    public:
        Assignment();
        Assignment(const int id, const int course_id, const std::string& name, const std::string& due_date);
 
        int GetId() const;
        int GetCourseId() const;
        const std::string& GetName() const;
        const std::string& GetDueDate() const;
        const bool GetCompleted() const;
        
        
        void print() const;
    private:
        int id_, course_id_;
        std::string name_;
        std::string due_date_;
        bool completed_;
};

#endif