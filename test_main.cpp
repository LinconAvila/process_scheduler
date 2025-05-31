#include <gtest/gtest.h>
#include <sstream>
#include <iostream>
#include <string>

// Redefine ScheduledProcess and PriorityScheduler for testing
class ScheduledProcess {
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

    ScheduledProcess(int pid, int creation_time, int burst_time, int tickets = 0) {
        this->pid = pid;
        this->creation_time = creation_time;
        this->burst_time = burst_time;
        this->remaining_time = burst_time;
        this->start_time = -1;
        this->end_time = -1;