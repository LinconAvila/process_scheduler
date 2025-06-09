#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <vector>
#include <map> // arvore de rubro negra
#include <queue>
#include <cctype>


class Process {
public:
    Process() : pid(0), creation_time(0), burst_time(0), tickets(0) {}
    Process(int pid, int creation_time, int burst_time, int tickets = 0);

    int get_pid() const;
    int get_creation_time() const;
    int get_end_time() const;
    int get_total_waiting_time() const;

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

    friend class CFSScheduler;
};

Process::Process(int pid, int creation_time, int burst_time, int tickets) {
    this->pid = pid;
    this->creation_time = creation_time;
    this->burst_time = burst_time;
    this->remaining_time = burst_time;
    this->start_time = -1;
    this->end_time = -1;
    this->tickets = tickets;
    this->weights = (tickets > 0) ? tickets : 1; // valor minimo de um processo -> Me guei pelo CFS original onde o maior peso indica a maior prioridade 
    this->is_finished = false;
    this->total_waiting_time = 0;
}

int Process::get_pid() const { return pid; }
int Process::get_end_time() const { return end_time; }
int Process::get_creation_time() const { return creation_time; }
int Process::get_total_waiting_time() const { return total_waiting_time; }

class FileReader {
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

FileReader::FileReader(const std::string &filename) {
    this->filename = filename;
    this->quantum = 0;
}

void FileReader::read_file() {
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
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

// Fornece a ornenação 
struct CFSKey {
    double vruntime;
    int pid;

    bool operator<(const CFSKey &other) const {
        //compara os vruntimes
        if (vruntime == other.vruntime)
            return pid < other.pid; // Se os vruntimes são iguais, usa o PID como desempate
        return vruntime < other.vruntime;
    }
};

class CFSScheduler {
private:
    std::map<CFSKey, Process> run_queue; // fila processos -> usa a arvore da lib 
    std::queue<Process> arrival_queue;
    int cpu_time = 0;
    const int TIME_SLICE = 5; // define a quantidade de tempo que um processo pode rodar na cpu, apos isso ele muda -> IMPORTANTE Pode mudar mas olhe bem os arquivos nao coloque um número absurdo
    const int MIN_WEIGHT = 1; //peso mínimo que um processo pode ter vai ser usado para o cálculo do vrtime


public:
    void load_processes(const FileReader& reader) {
        const auto& pids = reader.get_pids();
        const auto& creation_times = reader.get_creation_times();
        const auto& burst_times = reader.get_burst_times();
        const auto& tickets = reader.get_ticket_values();

        for (size_t i = 0; i < pids.size(); ++i) {
            Process proc(pids[i], creation_times[i], burst_times[i], tickets[i]);
            arrival_queue.push(proc);
        }
    }

    void run() {
        std::cout << "\n--- Iniciando Simulacao do Escalonador ---\n";
        std::cout << "Algoritmo: CFS | Fatia de CPU: " << TIME_SLICE << "\n\n";

        while (!run_queue.empty() || !arrival_queue.empty()) {
            // mover processos para a fila de execução
            while (!arrival_queue.empty() && arrival_queue.front().creation_time <= cpu_time) {
                Process proc = arrival_queue.front();
                arrival_queue.pop();
                run_queue[{0.0, proc.pid}] = proc;
            }

            if (run_queue.empty()) {
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

            if (proc.remaining_time > 0) {
                run_queue[{new_vruntime, proc.pid}] = proc;
            } else {
                proc.end_time = cpu_time;
                proc.is_finished = true;
                std::cout << ">>> Processo " << proc.pid << " finalizado no tempo " << cpu_time << " <<<\n";
                finished_processes.push_back(proc);
            }
        }

        std::cout << "\n--- Simulacao finalizada no tempo " << cpu_time << " ---\n\n";
        print_statistics();
    }

    void print_statistics() {
        std::cout << "--- Estatisticas Finais ---\n";
        std::cout << "PID       Tempo Total              Tempo Pronto\n";
        std::cout << "------------------------------------------------------------\n";

        for (const auto& proc : finished_processes) {
            int tempo_total = proc.end_time - proc.creation_time;
            std::cout << std::left << std::setw(10) << proc.pid
                      << std::setw(25) << tempo_total
                      << proc.total_waiting_time << "\n";
        }
    }

private:
    std::vector<Process> finished_processes;
};

int main() {
    std::cout << "Digite o nome do arquivo de entrada: ";
    std::string filename;
    std::cin >> filename;

    FileReader file_reader(filename);
    file_reader.read_file();

    std::string algorithm = file_reader.get_algorithm();
    for (char &c : algorithm)
        c = std::tolower(static_cast<unsigned char>(c));

    if (algorithm == "cfs") {
        CFSScheduler scheduler;
        scheduler.load_processes(file_reader);
        scheduler.run();
    } else {
        std::cerr << "Algoritmo não suportado ou ainda não implementado.\n";
    }

    return 0;
}

/*
  +----+
  |    |
  O    |
 /|\   |
 / \   |
       |
=========
*/

