#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// Estrutura que define um processo com todos os seus atributos para o escalonamento.
class Process {
    public:
    int pid;
    int creation_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int end_time;
    int weights; // Usado para a prioridade do processo

    // Membros não utilizados na lógica atual
    int ready_time;
    bool is_ready;
    bool is_finished;
    int tickets;
    int total_waiting_time;
    
    Process(int pid, int creation_time, int burst_time, int priority = 0) {
        this->pid = pid;
        this->creation_time = creation_time;
        this->burst_time = burst_time;
        this->remaining_time = burst_time;
        this->start_time = -1;
        this->end_time = -1;
        this->weights = priority;
        this->ready_time = creation_time;
        this->tickets = priority;
        this->is_ready = false;
        this->is_finished = false;
        this->total_waiting_time = 0;
    }
};

// Objeto de função para comparar a prioridade de dois processos.
struct CompareProcessPriority{
    bool operator()(const Process& a, const Process& b) const {
        // Compara pela prioridade (maior valor numérico vence).
        if (a.weights != b.weights) {
            return a.weights < b.weights;
        }
        // Em caso de empate, o processo que chegou antes (menor tempo de criação) tem preferência.
        return a.creation_time > b.creation_time; 
    }
};

// Classe principal que implementa o escalonador por prioridade.
class PriorityScheduler {
    private:
        std::vector<Process> ready_queue;       // Fila de processos prontos, gerenciada como um heap.
        std::vector<Process> pending_processes; // Lista de processos que ainda não chegaram.
        std::vector<Process> finalizados;       // Lista de processos concluídos.
        int current_time = 0;
        int quantum;
        CompareProcessPriority comparator;

        // Troca o conteúdo de dois objetos Process.
        void manual_swap(Process& a, Process& b){
            Process temp = a;
            a = b;
            b = temp;
        }

        // Parte "juntar" do algoritmo Merge Sort.
        void merge(std::vector<Process>& processes, int left, int mid, int right) {
            int n1 = mid - left + 1;
            int n2 = right - mid;

            std::vector<Process> L, R;
            L.reserve(n1);
            R.reserve(n2);

            for (int i = 0; i < n1; i++) L.push_back(processes[left + i]);
            for (int j = 0; j < n2; j++) R.push_back(processes[mid + 1 + j]);

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

        // Ordena um vetor de processos por tempo de criação usando Merge Sort.
        void merge_sort(std::vector<Process>& processes, int left, int right) {
            if (left >= right) return;
            int mid = left + (right - left) / 2;
            merge_sort(processes, left, mid);
            merge_sort(processes, mid + 1, right);
            merge(processes, left, mid, right);
        }

        // Garante a propriedade do heap "subindo" um elemento na árvore.
        void heapify_up(int index){ 
            if (index == 0) return;
            int parent_index = (index - 1) / 2;
            if (comparator(ready_queue[parent_index], ready_queue[index])) {
                manual_swap(ready_queue[parent_index], ready_queue[index]);
                heapify_up(parent_index);
            }
        }

        // Garante a propriedade do heap "descendo" um elemento na árvore.
        void heapify_down(int index) {
            size_t u_index = static_cast<size_t>(index);
            size_t left_child_index = 2 * u_index + 1;
            size_t right_child_index = 2 * u_index + 2;
            size_t largest = u_index;

            if (left_child_index < ready_queue.size() && comparator(ready_queue[largest], ready_queue[left_child_index])) {
                largest = left_child_index;
            }
            if (right_child_index < ready_queue.size() && comparator(ready_queue[largest], ready_queue[right_child_index])) {
                largest = right_child_index;
            }
            if (largest != u_index) {
                manual_swap(ready_queue[u_index], ready_queue[largest]);
                heapify_down(static_cast<int>(largest));
            }
        }

    public:
        // Construtor do escalonador.
        PriorityScheduler(int q) : quantum(q) {}

        // Adiciona um novo processo ao sistema.
        void addProcess(int pid, int creation_time, int burst_time, int priority) {
            pending_processes.emplace_back(pid, creation_time, burst_time, priority);
        }

        // Inicia a simulação do escalonador.
        void run() {
            std::cout << "Executando Priority Scheduler...\n";
            
            // Ordena os processos por tempo de chegada antes de iniciar a simulação.
            if (!pending_processes.empty()) {
                merge_sort(pending_processes, 0, pending_processes.size() - 1);
            }
            
            // Loop principal: executa enquanto houver processos pendentes ou na fila de prontos.
            while (!ready_queue.empty() || !pending_processes.empty()) {
                // Move processos que já chegaram para a fila de prontos.
                auto it = pending_processes.begin();
                while (it != pending_processes.end()){
                    if (it->creation_time <= current_time) {
                        ready_queue.push_back(*it);
                        heapify_up(ready_queue.size() - 1);
                        it = pending_processes.erase(it);
                    } else {
                        break;
                    }
                }
                
                // Se houver processos prontos, executa o de maior prioridade.
                if (!ready_queue.empty()) {
                    Process p = ready_queue.front();
                    
                    // Remove o processo de maior prioridade do heap de forma eficiente.
                    manual_swap(ready_queue.front(), ready_queue.back());
                    ready_queue.pop_back();
                    if (!ready_queue.empty()) {
                        heapify_down(0);
                    }

                    if (p.start_time == -1) p.start_time = current_time;
                    
                    int execution_time = (quantum < p.remaining_time) ? quantum : p.remaining_time;
                    current_time += execution_time;
                    p.remaining_time -= execution_time;

                    // Se o processo não terminou, o reinsere na fila de prontos. Caso contrário, finaliza.
                    if (p.remaining_time > 0){
                        ready_queue.push_back(p);
                        heapify_up(ready_queue.size() - 1);
                    } else {
                        p.end_time = current_time;
                        finalizados.push_back(p);
                    }
                } else {
                    // Se a CPU estiver ociosa, avança o tempo para o próximo evento.
                    if (!pending_processes.empty()) {
                        current_time = pending_processes.front().creation_time;
                    }
                }
            }
            
            // Ao final, exibe o resumo com as métricas de desempenho.
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

// Classe responsável por ler e interpretar o arquivo de entrada.
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
        if (!file.is_open()) {
            std::cerr << "Error opening file." << std::endl;
            return;
        }
        std::string line;
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
            std::getline(ss, token, '|'); creation_time = std::stoi(token);
            std::getline(ss, token, '|'); pid = std::stoi(token);
            std::getline(ss, token, '|'); burst_time = std::stoi(token);
            std::getline(ss, token);      ticket_value = std::stoi(token);
            creation_times.push_back(creation_time);
            pids.push_back(pid);
            burst_times.push_back(burst_time);
            ticket_values.push_back(ticket_value);
        }
        file.close();
    }
};

// Função principal que orquestra a execução do programa.
int main() {
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
    
    switch (num_P) {
        case '2': {
            if (reader.algorithm == "prioridade") {
                std::cout << "Voce escolheu o algoritmo de Prioridade.\n";
                PriorityScheduler scheduler(reader.quantum);
                for (size_t i = 0; i < reader.pids.size(); ++i) {
                    scheduler.addProcess(reader.pids[i], reader.creation_times[i],
                                         reader.burst_times[i], reader.ticket_values[i]);
                }
                scheduler.run();
            } else {
                std::cout << "O algoritmo no arquivo nao corresponde a 'prioridade'.\n";
            }
            break;
        }
        case '1':
        case '3':
        case '4':
            std::cout << "Algoritmo ainda nao implementado.\n";
            break;
        default:
            std::cout << "Opcao invalida. Por favor, selecione uma opção válida.\n";
    }
    return 0;
}