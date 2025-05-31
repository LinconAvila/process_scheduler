#include <iostream>
#include <fstream>
#include <sstream>
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

    Process(int pid, int creation_time, int burst_time, int tickets = 0) {
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

class FileReader {
public:
    std::string filename;
    std::string algorithm;
    int quantum;
    std::vector<int> ticket_values;
    std::vector<int> pids;
    std::vector<int> burst_times;
    std::vector<int> creation_times;

    FileReader(const std::string& filename) {
        this->filename = filename;
    }

    void read_file() {
        std::ifstream file(filename);
        std::string line;

        if (!file.is_open()) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }

        if (std::getline(file, line)) {
            std::stringstream ss(line);
            std::getline(ss, algorithm, '|');
            std::string quantum_str;
            std::getline(ss, quantum_str);
            quantum = std::stoi(quantum_str);
        }

        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string token;
            int pid, creation_time, burst_time, ticket_value;

            std::getline(ss, token, '|');
            creation_time = std::stoi(token);

            std::getline(ss, token, '|');
            pid = std::stoi(token);

            std::getline(ss, token, '|');
            burst_time = std::stoi(token);

            std::getline(ss, token);
            ticket_value = std::stoi(token);

            creation_times.push_back(creation_time);
            pids.push_back(pid);
            burst_times.push_back(burst_time);
            ticket_values.push_back(ticket_value);
        }
        file.close();
    }
};

int main() {
    return 0;
}
