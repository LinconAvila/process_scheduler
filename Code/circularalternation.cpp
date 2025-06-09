#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>
#include <iomanip>

using namespace std;

class Process {
public:
    enum Estado { PRONTO, EXECUTANDO, BLOQUEADO, FINALIZADO };

    Process(string pid, int creation_time, int burst_time)
        : pid(pid), creation_time(creation_time), burst_time(burst_time),
          remaining_time(burst_time), start_time(-1), end_time(-1),
          waiting_time(0), finished(false), criado(false), estado(PRONTO) {}

    string get_pid() const { return pid; }
    int get_creation_time() const { return creation_time; }
    int get_remaining_time() const { return remaining_time; }
    int get_waiting_time() const { return waiting_time; }
    int get_start_time() const { return start_time; }
    int get_end_time() const { return end_time; }
    bool is_finished() const { return finished; }
    bool foi_criado() const { return criado; }

    void execute() {
        if (remaining_time > 0) --remaining_time;
        if (remaining_time == 0) {
            finished = true;
            estado = FINALIZADO;
        } else {
            estado = EXECUTANDO;
        }
    }

    void set_start_time(int time) {
        if (start_time == -1) start_time = time;
    }

    void set_end_time(int time) {
        end_time = time;
        estado = FINALIZADO;
    }

    void increment_waiting_time() {
        ++waiting_time;
    }

    void marcarCriado() {
        criado = true;
        estado = PRONTO;
    }

    int get_lifetime() const {
        return (end_time >= creation_time) ? (end_time - creation_time) : burst_time;
    }

    string estadoComoString() const {
        switch (estado) {
            case PRONTO: return "PRONTO";
            case EXECUTANDO: return "EXECUTANDO";
            case BLOQUEADO: return "BLOQUEADO";
            case FINALIZADO: return "FINALIZADO";
            default: return "DESCONHECIDO";
        }
    }

private:
    string pid;
    int creation_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int end_time;
    int waiting_time;
    bool finished;
    bool criado;
    Estado estado;
};

class RoundRobinScheduler {
private:
    int quantum;
    int tempo = 0;
    vector<Process*> todosProcessos;
    queue<Process*> filaProntos;
    Process* processoAtual = nullptr;
    int tempoExecutadoNoQuantum = 0;

public:
    RoundRobinScheduler(int q) : quantum(q) {}

    void adicionarProcesso(Process* p) {
        todosProcessos.push_back(p);
    }

    void simular() {
        cout << "Simulacao iniciada (Round Robin - Quantum = " << quantum << ")\n\n";

        while (!todosFinalizados()) {
            // Criar processos no tempo atual
            for (auto& p : todosProcessos) {
                if (p->get_creation_time() == tempo && !p->foi_criado()) {
                    cout << "[Tempo " << tempo << "] Processo " << p->get_pid() << " criado.\n";
                    p->marcarCriado();
                    filaProntos.push(p);
                }
            }

            // Incrementar tempo de espera
            queue<Process*> temp;
            while (!filaProntos.empty()) {
                Process* p = filaProntos.front();
                filaProntos.pop();
                if (p != processoAtual)
                    p->increment_waiting_time();
                temp.push(p);
            }
            filaProntos = temp;

            // Troca de processo se quantum acabou
            if (tempoExecutadoNoQuantum == quantum || processoAtual == nullptr) {
                if (processoAtual && !processoAtual->is_finished()) {
                    processoAtual->marcarCriado(); // volta à fila
                    filaProntos.push(processoAtual);
                }

                if (!filaProntos.empty()) {
                    processoAtual = filaProntos.front();
                    filaProntos.pop();
                    processoAtual->set_start_time(tempo);
                    tempoExecutadoNoQuantum = 0;
                } else {
                    processoAtual = nullptr;
                }
            }

            // Executar processo atual
            if (processoAtual) {
                cout << "[Tempo " << tempo << "] Processo " << processoAtual->get_pid()
                     << " executando (restam " << processoAtual->get_remaining_time() << " unidades).\n";

                processoAtual->execute();
                tempoExecutadoNoQuantum++;

                // Se finalizou agora
                if (processoAtual->is_finished()) {
                    processoAtual->set_end_time(tempo + 1);
                    cout << "[Tempo " << tempo + 1 << "] Processo " << processoAtual->get_pid() << " finalizado.\n";
                    processoAtual = nullptr;
                    tempoExecutadoNoQuantum = 0;
                }
            }

            tempo++;
        }

        // Estatísticas finais
        cout << "\n--- Simulacao finalizada no tempo " << tempo << " ---\n";
        cout << "\n--- Estatisticas Finais ---\n";
        cout << left << setw(10) << "PID"
             << setw(20) << "Tempo de vida"
             << "Tempo de espera\n";
        for (auto& p : todosProcessos) {
            cout << left << setw(10) << p->get_pid()
                 << setw(20) << p->get_lifetime()
                 << p->get_waiting_time() << "\n";
        }

        for (auto p : todosProcessos)
            delete p;
    }

    bool todosFinalizados() {
        for (auto p : todosProcessos)
            if (!p->is_finished())
                return false;
        return true;
    }
};

int main() {
    ifstream arquivo("entrada.txt");
    if (!arquivo) {
        cerr << "Erro ao abrir o arquivo de entrada.\n";
        return 1;
    }

    string linha, algoritmo;
    int quantum;

    getline(arquivo, linha);
    stringstream ss(linha);
    getline(ss, algoritmo, '|');
    ss >> quantum;

    if (algoritmo != "ROUND_ROBIN") {
        cerr << "Erro: Apenas ROUND_ROBIN e suportado.\n";
        return 1;
    }

    RoundRobinScheduler escalonador(quantum);

    while (getline(arquivo, linha)) {
        stringstream sl(linha);
        string tmp;
        int momentoCriacao, tempoExecucao, prioridade;
        string pid;

        getline(sl, tmp, '|'); momentoCriacao = stoi(tmp);
        getline(sl, tmp, '|'); pid = tmp;
        getline(sl, tmp, '|'); tempoExecucao = stoi(tmp);
        getline(sl, tmp, '|'); prioridade = stoi(tmp); // ignorado

        escalonador.adicionarProcesso(new Process(pid, momentoCriacao, tempoExecucao));
    }

    escalonador.simular();
    return 0;
}
