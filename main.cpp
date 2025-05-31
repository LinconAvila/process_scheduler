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
    bool is_ready;
    bool is_finished;
    int tickets;
    int weights;
    int total_waiting_time;

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
        this->is_ready = false;
        this->is_finished = false;
        this->total_waiting_time = 0;
    }
};

class LotteryScheduler {
public:
    std::vector<Process> processes;
    std::vector<Process*> ready_queue;
    int quantum;
    int total_tickets;
    int current_time;

    LotteryScheduler(int quantum) {
        this->quantum = quantum;
        this->total_tickets = 0;
        this->current_time = 0;
        std::srand(time(0)); 
    }

    void add_process(const Process& process) {
        processes.push_back(process);
    }

    void add_ready_process(Process* process) {
        ready_queue.push_back(process);
        process->is_ready = true;
        process->ready_time = current_time;
    }

    void update_total_tickets() {
        total_tickets = 0;
        for (auto* process : ready_queue) {
            if (!process->is_finished) {
                total_tickets += process->tickets;
            }
        }
    }

    Process* select_process() {
        update_total_tickets();
        if (total_tickets == 0) {
            return nullptr; 
        }

        int selected_ticket = std::rand() % total_tickets;
        int cumulative_tickets = 0;
        for (auto* process : ready_queue) {
            if (process->is_finished) {
                continue;
            }
            cumulative_tickets += process->tickets;
            if (selected_ticket < cumulative_tickets) {
                return process; 
            }
        }
        return nullptr; 
    }


    bool all_finished() {
        for (const auto& process : processes) {
            if (!process.is_finished) {
                return false;
            }
        }
        return true;
    }

    // Selection sort para ordenar os processos conforme o tempo de criação
    void sort_creation_time(){
        int n = processes.size();
        for(int i = 0; i < n - 1; i++) {
            int min_index = i;
            for(int j = i + 1; j < n; j++) {
                if(processes[j].creation_time < processes[min_index].creation_time) {
                    min_index = j;
                }
            }
            if (min_index != i) {
                Process temp = processes[i];
                processes[i] = processes[min_index];
                processes[min_index] = temp;
            }
        }
    }

    void remove_ready_queue(Process* process) {
        int index = -1;
        for (int i = 0; i < ready_queue.size(); ++i) {
            if (ready_queue[i] == process) {
                index = i;
                break;
            }
        }
        if (index != -1) {
            int last_index = ready_queue.size() - 1;
            for(int k = index; k < last_index; ++k) {
                ready_queue[k] = ready_queue[k + 1];
            }
            ready_queue.pop_back();
        }
    }
    void get_new_processes(int& next_process_id) {
        int n = processes.size();
        while (next_process_id < n && processes[next_process_id].creation_time <= current_time) {
            Process* new_process = &processes[next_process_id];
            new_process->is_ready = true;
            new_process->ready_time = current_time;
            ready_queue.push_back(new_process);
            next_process_id++;
        }
    }

    void update_waiting_times(int run_time) {
        for (auto* process : ready_queue) {
            if (!process->is_finished) {
                process->total_waiting_time += run_time;
            }
        }
    }

    void execute_quantum(){
        Process* selected_process = select_process();
        if (selected_process == nullptr) {
            current_time++;
            return; 
        }

        remove_ready_queue(selected_process);
        selected_process->is_ready = false; 

        int run_time = std::min(selected_process->remaining_time, quantum);

        if (selected_process->start_time < 0) {
            selected_process->start_time = current_time;
        }

        update_waiting_times(run_time);

        selected_process->remaining_time -= run_time;
        current_time += run_time;

        if (selected_process->remaining_time == 0) {
            selected_process->is_finished = true;
            selected_process->end_time = current_time;
        } else {
            selected_process->is_ready = true;
            selected_process->ready_time = current_time;
            ready_queue.push_back(selected_process);
        }
    }

    void run(){
        sort_creation_time();
        int next_process_id = 0;
        int n = processes.size();

        while (!all_finished()) {
            get_new_processes(next_process_id);
            update_total_tickets();

            if (total_tickets == 0) {
                current_time++;
                continue; 
            }

            execute_quantum();
        }
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
