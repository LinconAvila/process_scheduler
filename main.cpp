#include <iostream>
#include <vector>
#include <string>
#include <time.h>



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
        this->ready_time = creation_time;
        this->tickets = tickets;
        this->weights = tickets;
    }
};

class PriorityScheduler {
    private:
        std::vector<ScheduledProcess> ready_queue;                  // Fila de processos prontos
        std::vector<ScheduledProcess> pending_processes;        // Fila de processos pendentes  
        std::vector<ScheduledProcess> finalizados;           // Fila de processos finalizados
        int current_time = 0;
        int quantum;                                        // Quantum fixo para o escalonamento  = fração de cpu que cada processo pode usar antes de ser interrompido

    public:
        PriorityScheduler(int q) : quantum(q) {}                // Construtor que recebe o quantum
    // Método para inserir um processo na fila de prontos com base na prioridade

        void insert_by_priority(std::vector<ScheduledProcess>& q, const ScheduledProcess& p) {
        auto it = q.begin();
        while (it != q.end()) {
            if (it->weights > p.weights) {                  // Prioridade mais alta (menor valor de weights) se for o inverso é a maior valor de weights que é a priridade
                ++it;
            } else if (it->weights == p.weights && it->creation_time < p.creation_time) {
                ++it;
            } else {
                break;
            }
        }
        q.insert(it, p);
    }
    void addProcess(int pid, int creation_time, int burst_time, int weigths) {              // Método para adicionar um processo à fila de pendentes
        if (creation_time < 0 || burst_time < 0 || weigths < 0) {
            std::cerr << "Erro: creation_time, burst_time e weights devem ser não-negativos.\n";
            return;
        }

        ScheduledProcess p(pid, creation_time, burst_time, weigths);
        pending_processes.push_back(p);
    }

    void run() {
        std::cout << "Executando Priority Scheduler...\n";

        while (!ready_queue.empty() || !pending_processes.empty()) {
            // Mover processos pendentes para a fila de prontos
            for (auto it = pending_processes.begin(); it != pending_processes.end();){
                if (it->creation_time <= current_time) {
                    insert_by_priority(ready_queue, *it);
                    it = pending_processes.erase(it); // Remove o processo da lista de pendentes
                } else {
                    ++it; // Avança para o próximo processo
                }
            }

            if (!ready_queue.empty()) {
                ScheduledProcess p = ready_queue.front();
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
                    insert_by_priority(ready_queue, p); // Reinsere o processo na fila de prontos
                } else {
                    p.end_time = current_time;
                    finalizados.push_back(p); // Processo finalizado
                }
            } else {
                std::cout << "Tempo " << current_time << ": Nenhum processo pronto para executar.\n";
                current_time++; // Avança o tempo se não houver processos prontos
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



int main(){

    char num_P;

    std::cout << "Selecione o tipo de algoritmo que você deseja aplicar no seu processo : \n";
    std::cout << "1 - Alternância circular \n";
    std::cout << "2 - Prioridade \n";
    std::cout << "3 - Loteria \n";
    std::cout << "4 - CFS (Completely Fair Scheduler) \n";
    std::cin >> num_P; 

    switch (num_P) {
        case '1':
            std::cout << "Você escolheu o algoritmo de Alternância Circular.\n";
            break;
        case '2':{
            std::cout << "Você escolheu o algoritmo de Prioridade.\n";

            int quantum = 2; // Quantum fixo de exemplo
            PriorityScheduler scheduler(quantum);

            scheduler.addProcess(1, 0, 5, 2);
            scheduler.addProcess(2, 2, 3, 1);
            scheduler.addProcess(3, 1, 2, 3);

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