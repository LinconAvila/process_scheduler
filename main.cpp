#include <iostream>
#include <string>
#include <vector>

class Process {
    public:
        int pid;
        int creation_time;
        int burst_time;
        int remaining_time;
        int start_time;
        int end_time;
        int ready_time;
        int tickets;
        int weights;

        Process(int pid, int creation_time, int burst_time, int tickets= 0) {
            this->pid = pid;
            this->creation_time = creation_time;
            this->burst_time = burst_time;
            this->remaining_time = burst_time;
            this->start_time = -1;
            this->end_time = -1;
            this->ready_time = creation_time;
            this->tickets = tickets;
            this->weights = tickets;
    }
};

class LotteryScheduler {
    public:
        int num_processes;
        std::vector<Process> processes;
        std::vector<Process> ready_queue;
        int quantum;
        int total_tickets;
        int current_time = 0;


    LotteryScheduler(int num_processes, int quantum, int total_tickets = 0) {
        this->num_processes = num_processes;
        this->quantum = quantum;
        this->total_tickets = total_tickets;
        this->processes.reserve(num_processes);
        this->ready_queue.reserve(num_processes);
    }

    



    void add_process(const Process& process) {
        processes.push_back(process);
    }
};
        



int main(){

    
   

   

   


    return 0;
}


    