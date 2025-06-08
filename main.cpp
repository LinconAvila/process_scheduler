#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <ctime>

class LotteryScheduler;

class Process
{
public:
    Process(int pid, int creation_time, int burst_time, int tickets = 0);
    int get_pid() const;
    int get_creation_time() const;
    int get_end_time() const;
    int get_total_waiting_time() const;

private:
    int pid;
    int creation_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int end_time;
    bool is_finished;
    int tickets;
    int total_waiting_time;
    friend class LotteryScheduler;
};

class FileReader
{
public:
    FileReader(const std::string &filename);
    void read_file();
    std::string get_algorithm() const;
    int get_quantum() const;
    const std::vector<int> &get_pids() const;
    const std::vector<int> &get_creation_times() const;
    const std::vector<int> &get_burst_times() const;
    const std::vector<int> &get_ticket_values() const;

private:
    std::string filename;
    std::string algorithm;
    int quantum;
    std::vector<int> ticket_values;
    std::vector<int> pids;
    std::vector<int> burst_times;
    std::vector<int> creation_times;
};

class LotteryScheduler
{
public:
    LotteryScheduler();
    void set_algorithm_name(const std::string &name);
    void set_quantum(int q);
    void add_process(const Process &process);
    void run();
    void print_statistics();

private:
    void update_ready_queue();
    Process *select_winner();
    std::vector<Process> processes;
    std::vector<Process *> ready_queue;
    std::string algorithm_name;
    int quantum;
    int current_time;
};

Process::Process(int pid, int creation_time, int burst_time, int tickets)
{
    this->pid = pid;
    this->creation_time = creation_time;
    this->burst_time = burst_time;
    this->remaining_time = burst_time;
    this->start_time = -1;
    this->end_time = -1;
    this->tickets = tickets;
    this->is_finished = false;
    this->total_waiting_time = 0;
}

int Process::get_pid() const { return pid; }
int Process::get_end_time() const { return end_time; }
int Process::get_creation_time() const { return creation_time; }
int Process::get_total_waiting_time() const { return total_waiting_time; }

FileReader::FileReader(const std::string &filename)
{
    this->filename = filename;
    this->quantum = 0;
}

void FileReader::read_file()
{
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return;
    }

    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::getline(ss, algorithm, '|');
        std::string quantum_str;
        std::getline(ss, quantum_str);
        quantum = std::stoi(quantum_str);
    }

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string token;
        int pid_val, creation_time_val, burst_time_val, ticket_value;

        std::getline(ss, token, '|');
        creation_time_val = std::stoi(token);
        std::getline(ss, token, '|');
        pid_val = std::stoi(token);
        std::getline(ss, token, '|');
        burst_time_val = std::stoi(token);
        std::getline(ss, token);
        ticket_value = std::stoi(token);

        creation_times.push_back(creation_time_val);
        pids.push_back(pid_val);
        burst_times.push_back(burst_time_val);
        ticket_values.push_back(ticket_value);
    }
    file.close();
}

std::string FileReader::get_algorithm() const { return algorithm; }
int FileReader::get_quantum() const { return quantum; }
const std::vector<int> &FileReader::get_pids() const { return pids; }
const std::vector<int> &FileReader::get_creation_times() const { return creation_times; }
const std::vector<int> &FileReader::get_burst_times() const { return burst_times; }
const std::vector<int> &FileReader::get_ticket_values() const { return ticket_values; }

LotteryScheduler::LotteryScheduler()
{
    quantum = 0;
    current_time = 0;
}

void LotteryScheduler::set_algorithm_name(const std::string &name) { algorithm_name = name; }
void LotteryScheduler::set_quantum(int q) { quantum = q; }
void LotteryScheduler::add_process(const Process &process) { processes.push_back(process); }

void LotteryScheduler::update_ready_queue()
{
    for (auto &process : processes)
    {
        bool in_ready_queue = false;
        for (const auto &ready_process : ready_queue)
        {
            if (ready_process->get_pid() == process.get_pid())
            {
                in_ready_queue = true;
                break;
            }
        }

        if (!process.is_finished && !in_ready_queue && process.creation_time <= current_time)
        {
            process.total_waiting_time += (current_time - process.creation_time);
            ready_queue.push_back(&process);
        }
    }
}

Process *LotteryScheduler::select_winner()
{
    int total_tickets = 0;
    for (const auto &process : ready_queue)
    {
        total_tickets += process->tickets;
    }
    if (total_tickets == 0)
    {
        return nullptr;
    }
    int winning_ticket = std::rand() % total_tickets;
    int current_ticket_sum = 0;
    for (auto &process : ready_queue)
    {
        current_ticket_sum += process->tickets;
        if (winning_ticket < current_ticket_sum)
        {
            return process;
        }
    }
    return nullptr;
}

void LotteryScheduler::run()
{
    std::srand(time(0));

    std::cout << "--- Iniciando Simulacao do Escalonador ---\n";
    std::cout << "Algoritmo: " << algorithm_name << " | Fatia de CPU: " << quantum << std::endl
              << std::endl;

    while (true)
    {
        update_ready_queue();

        bool all_finished = true;
        for (const auto &process : processes)
        {
            if (!process.is_finished)
            {
                all_finished = false;
                break;
            }
        }
        if (all_finished)
        {
            std::cout << "\n--- Simulacao finalizada no tempo " << current_time << " ---\n";
            break;
        }

        if (ready_queue.empty())
        {
            current_time++;
            continue;
        }

        Process *winner = select_winner();
        if (winner == nullptr)
        {
            current_time++;
            continue;
        }
        if (winner->start_time == -1)
        {
            winner->start_time = current_time;
        }

        int time_to_run = std::min(winner->remaining_time, quantum);

        std::cout << "Tempo[" << std::setw(3) << current_time << " -> " << std::setw(3) << current_time + time_to_run << "]: "
                  << "Processo " << winner->get_pid() << " esta na CPU. (Restante: "
                  << winner->remaining_time - time_to_run << ")" << std::endl;

        for (Process *process : ready_queue)
        {
            if (process->get_pid() != winner->get_pid())
            {
                process->total_waiting_time += time_to_run;
            }
        }

        current_time += time_to_run;
        winner->remaining_time -= time_to_run;

        if (winner->remaining_time <= 0)
        {
            winner->is_finished = true;
            winner->end_time = current_time;
            std::cout << ">>> Processo " << winner->get_pid() << " finalizado no tempo " << current_time << " <<<" << std::endl;

            for (size_t i = 0; i < ready_queue.size(); ++i)
            {
                if (ready_queue[i]->get_pid() == winner->get_pid())
                {
                    ready_queue.erase(ready_queue.begin() + i);
                    break;
                }
            }
        }
    }
}

void LotteryScheduler::print_statistics()
{
    std::cout << "\n--- Estatisticas Finais ---\n";
    std::cout << std::left << std::setw(10) << "PID"
              << std::setw(25) << "Tempo Total"
              << std::setw(25) << "Tempo em Estado Pronto" << std::endl;
    std::cout << "------------------------------------------------------------\n";

    for (const auto &process : processes)
    {
        int turnaround_time = process.get_end_time() - process.get_creation_time();
        int waiting_time = process.get_total_waiting_time();

        std::cout << std::left << std::setw(10) << process.get_pid()
                  << std::setw(25) << turnaround_time
                  << std::setw(25) << waiting_time << std::endl;
    }
}

int main()
{
    std::cout << "Digite o nome do arquivo de entrada: ";
    std::string filename;
    std::cin >> filename;

    FileReader file_reader(filename);
    file_reader.read_file();

    LotteryScheduler scheduler;
    scheduler.set_algorithm_name(file_reader.get_algorithm());
    scheduler.set_quantum(file_reader.get_quantum());

    const std::vector<int> &pids = file_reader.get_pids();
    const std::vector<int> &creation_times = file_reader.get_creation_times();
    const std::vector<int> &burst_times = file_reader.get_burst_times();
    const std::vector<int> &ticket_values = file_reader.get_ticket_values();

    for (size_t i = 0; i < pids.size(); ++i)
    {
        Process process(pids[i], creation_times[i], burst_times[i], ticket_values[i]);
        scheduler.add_process(process);
    }

    scheduler.run();
    scheduler.print_statistics();

    return 0;
}