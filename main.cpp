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

struct CompareProcessPriority{
    bool operator()(const Process& a, const Process& b) const {
        if (a.weights != b.weights)
        {
            return a.weights < b.weights; // Prioridade mais alta (menor valor de weights)
        }
        return a.creation_time > b.creation_time; 
    }
};

class PriorityScheduler {
    private:
        std::vector<Process> ready_queue;                                                                            // Fila de processos prontos
        std::vector<Process> pending_processes;                                                                      // Fila de processos pendentes  
        std::vector<Process> finalizados;                                                                            // Fila de processos finalizados
        int current_time = 0;
        int quantum;                                                                                      // Quantum fixo para o escalonamento  = fração de cpu que cada processo pode usar antes de ser interrompido
        CompareProcessPriority comparator;                                                                 // Função de comparação para ordenar os processos por prioridade

        void manual_swap(Process& a, Process& b){
            Process temp = a;
            a = b;                                                                                                 
            b = temp;
            }

        void merge(std::vector<Process>& processes, int left, int mid, int right) {
                int n1 = mid - left + 1;
                int n2 = right - mid;

                std::vector<Process> L, R;
                L.reserve(n1);
                R.reserve(n2);

                for (int i = 0; i < n1; i++) {
                    L.push_back(processes[left + i]);
                }
                for (int j = 0; j < n2; j++) {
                    R.push_back(processes[mid + 1 + j]);
                }

                int i = 0, j = 0;
                int k = left;
                while (i < n1 && j < n2) {
                    if (L[i].creation_time <= R[j].creation_time) {
                        processes[k++] = L[i++];
                    } else {
                        processes[k++] = R[j++];
                    }
                }
                while (i < n1) processes[k++] = L[i++];
                while (j < n2) processes[k++] = R[j++];
            }

        void merge_sort(std::vector<Process>& processes, int left, int right) {
            if (left >= right) {
                return; // Condição de parada: a sub-lista tem 0 ou 1 elemento.
            }

            int mid = left + (right - left) / 2;
            

            merge_sort(processes, left, mid);
            merge_sort(processes, mid + 1, right);
            

            merge(processes, left, mid, right);
        }

        void heapify_up(int index){ 
            if (index == 0) return;                                                                      // Se o índice for 0, não há pai para comparar
            int parent_index = (index - 1) / 2;                                                            // Índice do pai

            if (comparator(ready_queue[parent_index], ready_queue[index])) {
                manual_swap(ready_queue[parent_index], ready_queue[index]);                            // Troca o processo atual com o pai se a prioridade do pai for menor que a do filho
                heapify_up(parent_index);                                                         // Recursivamente aplica o heapify_up no pai
            }

    
        }
        void heapify_down(int index) {
            int left_child_index = 2 * index + 1; // Índice do filho esquerdo
            int right_child_index = 2 * index + 2; // Índice do filho direito
            int largest = index; // Inicializa o maior como o índice atual

            if (left_child_index < ready_queue.size() && comparator(ready_queue[largest], ready_queue[left_child_index])) {
                largest = left_child_index; // Se o filho esquerdo for maior, atualiza o maior
            }
            if (right_child_index < ready_queue.size() && comparator(ready_queue[largest], ready_queue[right_child_index])) {
                largest = right_child_index; // Se o filho direito for maior, atualiza o maior
            }
            
            if (largest != index ){
                manual_swap(ready_queue[index], ready_queue[largest]); // Troca o processo atual com o maior filho
                heapify_down(largest); // Recursivamente aplica o heapify_down no maior filho
            }
    }
    public:
    PriorityScheduler(int q) : quantum(q) {}

    void addProcess(int pid, int creation_time, int burst_time, int priority) {
        pending_processes.emplace_back(pid, creation_time, burst_time, priority);
    }

    void run() {
        std::cout << "Executando Priority Scheduler...\n";
        if (!pending_processes.empty()) {
            merge_sort(pending_processes, 0, pending_processes.size() - 1);
        }
        while (!ready_queue.empty() || !pending_processes.empty()) {                                                              // Enquanto houver processos prontos ou pendentes
            auto it = pending_processes.begin();
            while (it != pending_processes.end()){
                if (it->creation_time <= current_time) {
                    ready_queue.push_back(*it);                                                          // Adiciona o processo à fila de prontos
                    heapify_up(ready_queue.size() - 1);          // Reorganiza a fila de prontos após inserir o novo processo
                
                    it = pending_processes.erase(it);                                                                // Remove o processo da lista de pendentes
                } else {
                    break;
                }
            }
            

            if (!ready_queue.empty()) {                                                                                       // Se houver processos prontos
                Process p = ready_queue.front();
                manual_swap(ready_queue.front(), ready_queue.back()); // Move o último processo para o início
                ready_queue.pop_back(); // Remove o último processo (agora no início)
                if (!ready_queue.empty()) {
                    heapify_down(0); // Reorganiza a fila de prontos após remover o processo
                }
                if (p.start_time == -1) {
                    p.start_time = current_time;
                }
                int execution_time = (quantum < p.remaining_time) ? quantum : p.remaining_time;
                current_time += execution_time;
                p.remaining_time -= execution_time;

                if (p.remaining_time > 0){
                    ready_queue.push_back(p);                                                                       // Reinsere o processo na fila de prontos
                    heapify_up(ready_queue.size() - 1); // Reorganiza a fila de prontos após inserir o processo
                }else{
                    p.end_time = current_time;
                    finalizados.push_back(p);                                                                                // Processo finalizado
                }

            } else {                                                                                                         // Se não houver processos prontos, avança o tempo
                std::cout << "Tempo " << current_time << ": Nenhum processo pronto para executar.\n";
                current_time++;                                                                                              // Avança o tempo se não houver processos prontos
            }
        } // Fim do while principal
        std::cout << "\nResumo da execucao:\n";
        for (const auto& p : finalizados) {
            std::cout << "PID " << p.pid
                    << " | Criado: " << p.creation_time
                    << " | Inicio: " << p.start_time
                    << " | Fim: " << p.end_time
                    << " | Espera: " << (p.end_time - p.creation_time) - p.burst_time
                    << " | Retorno: " << (p.end_time - p.creation_time)
                    << "\n";
        }
        std::cout << "Tempo total de execucao: " << current_time << " unidades de tempo.\n";
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