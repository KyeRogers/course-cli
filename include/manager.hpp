#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <vector>
#include "assignment.hpp"


class Manager {
    public: 
        Manager();
        
        void AddTask(std::string name, std::string due_date);
        void ShowTasks() const;
    private:
        std::vector<Assignment> tasks_;
        int last_id_;


};

#endif