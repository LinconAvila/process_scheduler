#include <iostream>
#include <fstream> // leitura de arquivos
#include <sstream> // manipulação de strings
#include <string> 
#include <iomanip> // formatação de saída
#include <cstdlib>  // gerar números aleatórios
#include <vector> // vetor de processos
#include <map> // Arvore rubro negra
#include <queue> // Fila de chegada


// Prototipos de classes e structs
class LotteryScheduler;
class PriorityScheduler;
class CFSScheduler;
struct CompareProcessPriority;
struct CFSKey;

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

    // Public para acesso de outras classes
    friend class LotteryScheduler; 
    friend class PriorityScheduler;
    friend class CFSScheduler;
    friend struct CompareProcessPriority;
};

Process::Process(int pid, int creation_time, int burst_time, int tickets)
{
    this->pid = pid;
    this->creation_time = creation_time;
    this->burst_time = burst_time; // tempo total de execução do processo
    this->remaining_time = burst_time; // tempo restante para o processo terminar
    this->start_time = -1;
    this->end_time = -1;
    this->tickets = tickets; //usado no escalonador por loteria
    this->weights = (tickets > 0) ? tickets : 1; // valor minimo de um processo -> Me guiei pelo CFS original onde o maior peso indica a maior prioridade
    this->is_finished = false;
    this->total_waiting_time = 0; // tempo total que o processo ficou na fila de espera
}
// Getters para acessar os atributos privados
int Process::get_pid() const { return pid; }
int Process::get_end_time() const { return end_time; }
int Process::get_creation_time() const { return creation_time; }
int Process::get_total_waiting_time() const { return total_waiting_time; }

// Classe para ler o arquivo de entrada e armazenar os dados dos processos

class FileReader
{
public: 
    FileReader(const std::string &filename); // Construtor que recebe o nome do arquivo
    void read_file();
    std::string get_algorithm() const;
    int get_quantum() const;
    const std::vector<int> &get_pids() const; // Retorna os PIDs dos processos
    const std::vector<int> &get_creation_times() const; // Retorna os tempos de criação dos processos
    const std::vector<int> &get_burst_times() const; // Retorna os tempos de execução dos processos
    const std::vector<int> &get_ticket_values() const; // Retorna os valores de tickets dos processos

private:
    std::string filename;
    std::string algorithm;
    int quantum;
    std::vector<int> ticket_values; // vetores para armazenar os tickets
    std::vector<int> pids; // vetores para armazenar os pids
    std::vector<int> burst_times; // vetores para armazenar os tempos de execução
    std::vector<int> creation_times; // vetores para armazenar os tempos de criação
};

FileReader::FileReader(const std::string &filename) //
{
    this->filename = filename;
    this->quantum = 0; // fatia de CPU inicializada como 0
}

void FileReader::read_file() // Lê o arquivo e armazena os dados dos processos
{
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return;
    }

    if (std::getline(file, line)) // Lê a primeira linha do arquivo que contem o nome do algoritmo e a fatia de CPU
    {
        std::stringstream ss(line);
        std::getline(ss, algorithm, '|'); // Separa a string pelo delimitador '|'
        std::string quantum_str;
        std::getline(ss, quantum_str);
        quantum = std::stoi(quantum_str);
    }

    while (std::getline(file, line)) // Lê as outras linhas do arquivo que contêm os dados dos processos
    {
        std::stringstream ss(line);
        std::string token;
        int pid_val, creation_time_val, burst_time_val, ticket_value;

        std::getline(ss, token, '|'); // Lê o tempo de criação do processo
        creation_time_val = std::stoi(token);
        std::getline(ss, token, '|'); // Lê o PID do processo
        pid_val = std::stoi(token);
        std::getline(ss, token, '|'); // Lê o tempo de execução do processo
        burst_time_val = std::stoi(token);
        std::getline(ss, token); // Lê o valor do ticket do processo
        ticket_value = std::stoi(token);

        creation_times.push_back(creation_time_val); // Armazena o tempo de criação do processo
        pids.push_back(pid_val); // Armazena o PID do processo
        burst_times.push_back(burst_time_val); // Armazena o tempo de execução do processo
        ticket_values.push_back(ticket_value); // Armazena o valor do ticket do processo
    }
    file.close();
}
// Getters para acessar os atributos privados
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
    void set_algorithm_name(const std::string &name); // Define o nome do algoritmo
    void set_quantum(int q); // Define a fatia de CPU
    void add_process(const Process &process); // Adiciona um processo ao escalonador
    void run(); // roda o escalonador
    void print_statistics(); // printa as estatísticas finais

private:
    void update_ready_queue(); // Atualiza a fila de processos prontos
    Process *select_winner(); // Seleciona o processo vencedor com base nos tickets
    std::vector<Process> processes; // vetor de processos
    std::vector<Process *> ready_queue; // fila de processos prontos
    std::string algorithm_name;
    int quantum; // fatia de CPU
    int current_time; // tempo atual do escalonador
};

LotteryScheduler::LotteryScheduler() // Construtor
{
    quantum = 0;
    current_time = 0;
}

void LotteryScheduler::set_algorithm_name(const std::string &name) { algorithm_name = name; } //
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
    std::srand(time(0)); // semente aleatória

    std::cout << "--- Iniciando Simulacao do Escalonador ---\n";
    std::cout << "Algoritmo: " << algorithm_name << " | Fatia de CPU: " << quantum << std::endl
              << std::endl;

    while (true) //loop do escalonador
    {
        update_ready_queue(); // Atualiza a fila de processos prontos

        bool all_finished = true; 
        for (const auto &process : processes) // percorre todos os processos e verifica se todos estão finalizados
        {
            if (!process.is_finished)
            {
                all_finished = false;
                break;
            }
        }
        if (all_finished) // se todos os processos estão finalizados, encerra a simulação
        {
            std::cout << "\n--- Simulacao finalizada no tempo " << current_time << " ---\n";
            break;
        }

        if (ready_queue.empty()) // se a fila de processos prontos está vazia, incrementa o tempo atual
        {
            current_time++;
            continue;
        }

        Process *winner = select_winner(); // Seleciona o processo vencedor com base nos tickets
        if (winner == nullptr)
        {
            current_time++;
            continue;
        }
        if (winner->start_time == -1) // Se o processo vencedor ainda não começou, define o tempo de início
        {
            winner->start_time = current_time;
        }

        int time_to_run = std::min(winner->remaining_time, quantum); // Calcula o tempo que o processo vencedor vai rodar na CPU

        std::cout << "Tempo[" << std::setw(3) << current_time << " -> " << std::setw(3) << current_time + time_to_run << "]: "
                  << "Processo " << winner->get_pid() << " esta na CPU. (Restante: "
                  << winner->remaining_time - time_to_run << ")" << std::endl;

        for (Process *process : ready_queue) // Atualiza o tempo de espera dos outros processos na fila prontos
        {
            if (process->get_pid() != winner->get_pid())
            {
                process->total_waiting_time += time_to_run;
            }
        }

        current_time += time_to_run; 
        winner->remaining_time -= time_to_run; // Atualiza o tempo restante do processo vencedor

        if (winner->remaining_time <= 0) // Se o processo vencedor terminou, marca como finalizado e remove da fila de prontos
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
        int turnaround_time = process.get_end_time() - process.get_creation_time(); // tempo total de existencia do processo
        int waiting_time = process.get_total_waiting_time();

        std::cout << std::left << std::setw(10) << process.get_pid()
                  << std::setw(25) << turnaround_time
                  << std::setw(25) << waiting_time << std::endl;
    }
}

// Escalonador por prioridade

struct CompareProcessPriority // struct para comparar processos
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

class PriorityScheduler // Classe do escalonador por prioridade
{
private:
    std::vector<Process> ready_queue; // Fila de processos prontos
    std::vector<Process> pending_processes; // Fila de processos pendentes
    std::vector<Process> finalizados; // Fila de processos finalizados
    int current_time = 0;
    int quantum; // Fatia de CPU
    CompareProcessPriority comparator; // Comparador de processos por prioridade

    void manual_swap(Process &a, Process &b) // Função para trocar dois processos
    {
        Process temp = a;
        a = b;
        b = temp;
    }

    void merge(std::vector<Process> &processes, int left, int mid, int right) // Função para mesclar dois subarrays
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

    void merge_sort(std::vector<Process> &processes, int left, int right) // Função para ordenar os processos com merge sort
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

    void heapify_up(int index) // Função para manter a propriedade do heap ao inserir um novo processo
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
    void heapify_down(int index) // Função para manter a propriedade do heap ao remover o processo de maior prioridade
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
    PriorityScheduler(int q) : quantum(q) {} // Construtor que recebe a fatia de CPU

    void addProcess(int pid, int creation_time, int burst_time, int priority) // Adiciona um processo à fila de pendentes
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

// Escalonador CFS

// Fornece a ordenação
struct CFSKey
{
    double vruntime;
    int pid;

    bool operator<(const CFSKey &other) const
    {
        //compara os vruntimes
        if (vruntime == other.vruntime)
            return pid < other.pid; // Se os vruntimes são iguais, usa o PID como desempate
        return vruntime < other.vruntime;
    }
};

// Classe do Escalonador CFS
class CFSScheduler
{
private:
    std::map<CFSKey, Process> run_queue; // fila processos -> usa a arvore da lib
    std::queue<Process> arrival_queue;   // Fila de chegada
    std::vector<Process> finished_processes;
    int cpu_time = 0;
    // define a quantidade de tempo que um processo pode rodar na cpu, apos isso ele muda -> IMPORTANTE Pode mudar mas olhe bem os arquivos nao coloque um número absurdo
    const int TIME_SLICE = 5;
    //peso mínimo que um processo pode ter vai ser usado para o cálculo do vrtime
    const int MIN_WEIGHT = 1;

public:
    void load_processes(const FileReader &reader)
    {
        const auto &pids = reader.get_pids();
        const auto &creation_times = reader.get_creation_times();
        const auto &burst_times = reader.get_burst_times();
        const auto &tickets = reader.get_ticket_values();

        for (size_t i = 0; i < pids.size(); ++i)
        {
            Process proc(pids[i], creation_times[i], burst_times[i], tickets[i]);
            arrival_queue.push(proc);
        }
    }

    void run()
    {
        std::cout << "\n--- Iniciando Simulacao do Escalonador ---\n";
        std::cout << "Algoritmo: CFS | Fatia de CPU: " << TIME_SLICE << "\n\n";

        while (!run_queue.empty() || !arrival_queue.empty())
        {
            // mover processos para a fila de execução
            while (!arrival_queue.empty() && arrival_queue.front().creation_time <= cpu_time)
            {
                Process proc = arrival_queue.front();
                arrival_queue.pop();
                run_queue.insert({{0.0, proc.pid}, proc});
            }

            if (run_queue.empty())
            {
                cpu_time++;
                continue;
            }

            auto it = run_queue.begin();
            CFSKey key = it->first;
            Process proc = it->second;
            run_queue.erase(it);

            int slice = std::min(TIME_SLICE, proc.remaining_time);
            int start = cpu_time;
            int end = cpu_time + slice;

            std::cout << "Tempo[" << std::setw(3) << start << " -> " << std::setw(3) << end << "]: Processo "
                      << proc.pid << " esta na CPU. (Restante: " << (proc.remaining_time - slice) << ")\n";

            if (proc.start_time == -1)
                proc.start_time = cpu_time;

            cpu_time += slice;
            proc.remaining_time -= slice;

            // Calcula o novo vruntime baseado no tempo de execução do processo (slice).
            // Fórmula: vruntime += (tempo_executado * MIN_WEIGHT) / peso_do_processo
            // Quanto maior o peso (maior prioridade), mais lentamente o vruntime cresce,
            // permitindo que o processo tenha mais tempo de CPU ao longo do tempo.
            double new_vruntime = key.vruntime + (static_cast<double>(slice) * MIN_WEIGHT) / proc.weights;

            proc.total_waiting_time = cpu_time - proc.creation_time - (proc.burst_time - proc.remaining_time);

            if (proc.remaining_time > 0)
            {
                run_queue.insert({{new_vruntime, proc.pid}, proc});
            }
            else
            {
                proc.end_time = cpu_time;
                proc.is_finished = true;
                std::cout << ">>> Processo " << proc.pid << " finalizado no tempo " << cpu_time << " <<<\n";
                finished_processes.push_back(proc);
            }
        }

        std::cout << "\n--- Simulacao finalizada no tempo " << cpu_time << " ---\n\n";
        print_statistics();
    }

    void print_statistics()
    {
        std::cout << "--- Estatisticas Finais ---\n";
        std::cout << "PID       Tempo Total              Tempo Pronto\n";
        std::cout << "------------------------------------------------------------\n";

        for (const auto &proc : finished_processes)
        {
            int tempo_total = proc.end_time - proc.creation_time;
            std::cout << std::left << std::setw(10) << proc.pid
                      << std::setw(25) << tempo_total
                      << proc.total_waiting_time << "\n";
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
    else if (algorithm == "cfs")
    {
        CFSScheduler scheduler;
        scheduler.load_processes(file_reader);
        scheduler.run();
    }
    else
    {
        std::cerr << "Algoritmo não suportado ou ainda não implementado.\n";
    }

    return 0;
}
