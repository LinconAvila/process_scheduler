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

class PriorityScheduler {
    private:
        std::vector<Process> ready_queue;                                                                            // Fila de processos prontos
        std::vector<Process> pending_processes;                                                                      // Fila de processos pendentes  
        std::vector<Process> finalizados;                                                                            // Fila de processos finalizados
        int current_time = 0;
        int quantum;                                                                                                          // Quantum fixo para o escalonamento  = fração de cpu que cada processo pode usar antes de ser interrompido

    public:
        PriorityScheduler(int q) : quantum(q) {}                                                                              // Construtor que recebe o quantum  //Método para inserir um processo na fila de prontos com base na prioridade

        void insert_by_priority(std::vector<Process>& q, const Process& p) {
        auto it = q.begin();
        while (it != q.end()) {
            if (it->weights > p.weights) {                                                                                     // Prioridade mais alta (menor valor de weights) se for o inverso é a maior valor de weights que é a priridade
                ++it;
            } else if (it->weights == p.weights && it->creation_time < p.creation_time) {
                ++it;
            } else {
                break;
            }
        }
        q.insert(it, p);
    }
    void addProcess(int pid, int creation_time, int burst_time, int weights) {                                                 // Método para adicionar um processo à fila de pendentes
        if (creation_time < 0 || burst_time < 0 || weights < 0)
        {
            std::cerr << "Erro: creation_time, burst_time e weights devem ser não-negativos.\n";
            return;
        }

        Process p(pid, creation_time, burst_time, weights);                                                          // Cria um novo processo
        pending_processes.push_back(p);                                                                                       // Adiciona o processo à fila de pendentes
    }

    void run() {
        std::cout << "Executando Priority Scheduler...\n";

    while (!ready_queue.empty() || !pending_processes.empty()) {                                                              // Enquanto houver processos prontos ou pendentes
            // Mover processos pendentes para a fila de prontos
            for (auto it = pending_processes.begin(); it != pending_processes.end();){                                        // Percorre a lista de processos pendentes
                if (it->creation_time <= current_time) {
                    insert_by_priority(ready_queue, *it);
                    it = pending_processes.erase(it);                                                                         // Remove o processo da lista de pendentes
                } else {
                    ++it;                                                                                                     // Avança para o próximo processo
                }
            }

            if (!ready_queue.empty()) {                                                                                       // Se houver processos prontos
                Process p = ready_queue.front();
                ready_queue.erase(ready_queue.begin());

                if (p.start_time == -1) {
                    p.start_time = current_time;
                }
                int execution_time = (quantum < p.remaining_time) ? quantum : p.remaining_time;

                std::cout << "Tempo " << current_time << ": Executando PID " << p.pid
                          << " por " << execution_time << " unidades de tempo\n";

                current_time += execution_time;
                p.remaining_time -= execution_time;

                if (p.remaining_time > 0) {
                    p.ready_time = current_time;
                    insert_by_priority(ready_queue, p);                                                                       // Reinsere o processo na fila de prontos
                } else {
                    p.end_time = current_time;
                    finalizados.push_back(p);                                                                                // Processo finalizado
                }
            } else {                                                                                                         // Se não houver processos prontos, avança o tempo
                std::cout << "Tempo " << current_time << ": Nenhum processo pronto para executar.\n";
                current_time++;                                                                                              // Avança o tempo se não houver processos prontos
            }

        }
        std::cout << "\nResumo da execução:\n";
        for (const auto& p : finalizados) {
            std::cout << "PID " << p.pid
                    << " | Criado: " << p.creation_time
                    << " | Inicio: " << p.start_time
                    << " | Fim: " << p.end_time
                    << " | Espera: " << (p.start_time - p.creation_time)
                    << " | Retorno: " << (p.end_time - p.creation_time)
                    << "\n";
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


int main(){

    char num_P;
    std::string filename = "entradaEscalonador.txt";

    std::cout << "Selecione o tipo de algoritmo que voce deseja aplicar no seu processo : \n";
    std::cout << "1 - Alternancia circular \n";
    std::cout << "2 - Prioridade \n";
    std::cout << "3 - Loteria \n";
    std::cout << "4 - CFS (Completely Fair Scheduler) \n";
    std::cin >> num_P;

    FileReader reader(filename);
    reader.read_file();
    switch  (num_P) {
        case '1':
            std::cout << "Voce escolheu o algoritmo de Alternância Circular.\n";
            break; // Adicione este break
        case '2':{
            std::cout << "Voce escolheu o algoritmo de Prioridade.\n";
            PriorityScheduler scheduler(reader.quantum);
            for (size_t i = 0; i < reader.pids.size(); ++i) {
                scheduler.addProcess(reader.pids[i], reader.creation_times[i],
                                     reader.burst_times[i], reader.ticket_values[i]);
            }
            scheduler.run();
            break;
        }
        case '3':
            std::cout << "Você escolheu o algoritmo de Loteria.\n";
            break;
        case '4':
            std::cout << "Você escolheu o algoritmo CFS (Completely Fair Scheduler).\n";
            break;
        default:
            std::cout << "Opção inválida. Por favor, selecione uma opção válida.\n";
    }

    return 0;
}