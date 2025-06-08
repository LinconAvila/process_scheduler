#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <ctime>

// Prototipos de classes e structs
class LotteryScheduler;
class PriorityScheduler;
struct CompareProcessPriority;

// Classe para os processos

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
    int weights;
    int total_waiting_time;

    friend class LotteryScheduler;
    friend class PriorityScheduler;
    friend struct CompareProcessPriority;
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
    this->weights = tickets;
    this->is_finished = false;
    this->total_waiting_time = 0;
}

int Process::get_pid() const { return pid; }
int Process::get_end_time() const { return end_time; }
int Process::get_creation_time() const { return creation_time; }
int Process::get_total_waiting_time() const { return total_waiting_time; }

// Classe para ler o arquivo de entrada e armazenar os dados dos processos

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

// Escalonador por loteria

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
              << std::setw(25) << "Tempo Pronto" << std::endl;
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

// Escalonador por prioridade

struct CompareProcessPriority
{
    bool operator()(const Process &a, const Process &b) const
    {
        if (a.weights != b.weights)
        {
            return a.weights > b.weights;
        }
        return a.creation_time > b.creation_time;
    }
};

class PriorityScheduler
{
private:
    std::vector<Process> ready_queue;
    std::vector<Process> pending_processes;
    std::vector<Process> finalizados;
    int current_time = 0;
    int quantum;
    CompareProcessPriority comparator;

    void manual_swap(Process &a, Process &b)
    {
        Process temp = a;
        a = b;
        b = temp;
    }

    void merge(std::vector<Process> &processes, int left, int mid, int right)
    {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        std::vector<Process> L, R;
        L.reserve(n1);
        R.reserve(n2);

        for (int i = 0; i < n1; i++)
        {
            L.push_back(processes[left + i]);
        }
        for (int j = 0; j < n2; j++)
        {
            R.push_back(processes[mid + 1 + j]);
        }

        int i = 0, j = 0;
        int k = left;
        while (i < n1 && j < n2)
        {
            if (L[i].creation_time <= R[j].creation_time)
            {
                processes[k++] = L[i++];
            }
            else
            {
                processes[k++] = R[j++];
            }
        }
        while (i < n1)
            processes[k++] = L[i++];
        while (j < n2)
            processes[k++] = R[j++];
    }

    void merge_sort(std::vector<Process> &processes, int left, int right)
    {
        if (left >= right)
        {
            return;
        }

        int mid = left + (right - left) / 2;

        merge_sort(processes, left, mid);
        merge_sort(processes, mid + 1, right);

        merge(processes, left, mid, right);
    }

    void heapify_up(int index)
    {
        if (index == 0)
            return;
        int parent_index = (index - 1) / 2;

        if (comparator(ready_queue[parent_index], ready_queue[index]))
        {
            manual_swap(ready_queue[parent_index], ready_queue[index]);
            heapify_up(parent_index);
        }
    }
    void heapify_down(int index)
    {
        int left_child_index = 2 * index + 1;
        int right_child_index = 2 * index + 2;
        int largest = index;

        if (left_child_index < ready_queue.size() && comparator(ready_queue[largest], ready_queue[left_child_index]))
        {
            largest = left_child_index;
        }
        if (right_child_index < ready_queue.size() && comparator(ready_queue[largest], ready_queue[right_child_index]))
        {
            largest = right_child_index;
        }

        if (largest != index)
        {
            manual_swap(ready_queue[index], ready_queue[largest]);
            heapify_down(largest);
        }
    }

public:
    PriorityScheduler(int q) : quantum(q) {}

    void addProcess(int pid, int creation_time, int burst_time, int priority)
    {
        pending_processes.emplace_back(pid, creation_time, burst_time, priority);
    }

    void run()
    {
        std::cout << "--- Iniciando Simulacao do Escalonador ---\n";
        std::cout << "Algoritmo: prioridade | Fatia de CPU: " << quantum << std::endl
                  << std::endl;

        if (!pending_processes.empty())
        {
            merge_sort(pending_processes, 0, pending_processes.size() - 1);
        }
        while (!ready_queue.empty() || !pending_processes.empty())
        {
            auto it = pending_processes.begin();
            while (it != pending_processes.end())
            {
                if (it->creation_time <= current_time)
                {
                    ready_queue.push_back(*it);
                    heapify_up(ready_queue.size() - 1);

                    it = pending_processes.erase(it);
                }
                else
                {
                    break;
                }
            }

            if (!ready_queue.empty())
            {
                Process p = ready_queue.front();
                manual_swap(ready_queue.front(), ready_queue.back());
                ready_queue.pop_back();
                if (!ready_queue.empty())
                {
                    heapify_down(0);
                }
                if (p.start_time == -1)
                {
                    p.start_time = current_time;
                }
                int execution_time = (quantum < p.remaining_time) ? quantum : p.remaining_time;

                std::cout << "Tempo[" << std::setw(3) << current_time << " -> " << std::setw(3) << current_time + execution_time << "]: "
                          << "Processo " << p.pid << " esta na CPU. (Restante: "
                          << p.remaining_time - execution_time << ")" << std::endl;

                current_time += execution_time;
                p.remaining_time -= execution_time;

                if (p.remaining_time > 0)
                {
                    ready_queue.push_back(p);
                    heapify_up(ready_queue.size() - 1);
                }
                else
                {
                    p.end_time = current_time;
                    std::cout << ">>> Processo " << p.pid << " finalizado no tempo " << current_time << " <<<" << std::endl;
                    finalizados.push_back(p);
                }
            }
            else
            {
                current_time++;
            }
        }
        std::cout << "\n--- Estatisticas Finais ---\n";
        std::cout << std::left << std::setw(10) << "PID"
                  << std::setw(25) << "Tempo Total"
                  << std::setw(25) << "Tempo Pronto" << std::endl;
        std::cout << "------------------------------------------------------------\n";
        for (const auto &p : finalizados)
        {
            int turnaround_time = (p.end_time - p.creation_time);
            int waiting_time = turnaround_time - p.burst_time;
            std::cout << std::left << std::setw(10) << p.pid
                      << std::setw(25) << turnaround_time
                      << std::setw(25) << waiting_time << std::endl;
        }
    }
};

int main()
{
    std::cout << "Digite o nome do arquivo de entrada: ";
    std::string filename;
    std::cin >> filename;

    FileReader file_reader(filename);
    file_reader.read_file();

    std::string algorithm = file_reader.get_algorithm();
    for (char &c : algorithm)
    {
        c = std::tolower(static_cast<unsigned char>(c));
    }

    if (algorithm == "loteria")
    {
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
    }
    else if (algorithm == "prioridade")
    {
        PriorityScheduler scheduler(file_reader.get_quantum());

        const std::vector<int> &pids = file_reader.get_pids();
        const std::vector<int> &creation_times = file_reader.get_creation_times();
        const std::vector<int> &burst_times = file_reader.get_burst_times();
        const std::vector<int> &ticket_values = file_reader.get_ticket_values();

        for (size_t i = 0; i < pids.size(); ++i)
        {
            scheduler.addProcess(pids[i], creation_times[i], burst_times[i], ticket_values[i]);
        }
        scheduler.run();
    }
    else
    {
        std::cerr << "Erro: Algoritmo '" << algorithm << "' nao reconhecido. Verifique o arquivo de entrada." << std::endl;
    }

    return 0;
}