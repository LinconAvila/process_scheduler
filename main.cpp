#include <iostream>
#include <string>
#include <time.h>
#include <vector>

class Process {
    public:
        std::string pid;
        int creation_time;
        int burst_time;
        int remaining_time;
        int start_time;
        int end_time;
        int ready_time;
        int tickets;
        int weights;

        Process(std::string pid, int creation_time, int burst_time, int tickets= 0) {
            this->pid = pid;
            this->creation_time = creation_time;
            this->burst_time = burst_time;
            this->tickets = tickets;
            this->weights = tickets;
    }
};






    
        



int main(){

    
   

   

   


    return 0;
}


    