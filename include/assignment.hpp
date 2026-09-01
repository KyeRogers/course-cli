#ifndef ASSIGNMENT_HPP
#define ASSIGNMENT_HPP

#include <string>

class Assignment {
    public:
        Assignment();
        Assignment(int id, std::string name, std::string due_date);

        void print() const;
    private:
        int id_;
        std::string name_;
        std::string due_date_;
        bool completed_;
};

#endif