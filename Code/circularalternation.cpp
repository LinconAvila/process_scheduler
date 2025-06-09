#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>

using namespace std;

struct Processo {
    enum Estado { PRONTO, EXECUTANDO, BLOQUEADO, FINALIZADO };

    int momentoCriacao;
    string pid;
    int tempoRestante;
    int prioridade; // ignorado em Round Robin
    Estado estado;
    bool criado = false;

    Processo(int m, string p, int t, int prio)
        : momentoCriacao(m), pid(p), tempoRestante(t), prioridade(prio), estado(PRONTO) {}
};

class Escalonador {
private:
    int quantum;
    int tempo = 0;
    vector<Processo*> todosProcessos;
    queue<Processo*> filaProntos;
    Processo* processoAtual = nullptr;
    int tempoExecutadoNoQuantum = 0;

public:
    Escalonador(int quantum) : quantum(quantum) {}

    void adicionarProcesso(Processo* p) {
        todosProcessos.push_back(p);
    }

    void simular() {
        while (!todosFinalizados()) {

            // Criação de novos processos no tempo atual
            for (auto& p : todosProcessos) {
                if (!p->criado && p->momentoCriacao == tempo) {
                    cout << "[Tempo " << tempo << "] Processo " << p->pid << " criado.\n";
                    p->criado = true;
                    p->estado = Processo::PRONTO;
                    filaProntos.push(p);
                }
            }

            // Finalização do processo atual
            if (processoAtual && processoAtual->tempoRestante == 0) {
                cout << "[Tempo " << tempo << "] Processo " << processoAtual->pid << " finalizado.\n";
                processoAtual->estado = Processo::FINALIZADO;
                processoAtual = nullptr;
                tempoExecutadoNoQuantum = 0;
            }

            // Troca por fim de quantum ou ausência de processo atual
            if (!processoAtual || tempoExecutadoNoQuantum == quantum) {
                if (processoAtual && processoAtual->tempoRestante > 0) {
                    processoAtual->estado = Processo::PRONTO;
                    filaProntos.push(processoAtual); // devolve à fila
                }

                if (!filaProntos.empty()) {
                    processoAtual = filaProntos.front();
                    filaProntos.pop();
                    processoAtual->estado = Processo::EXECUTANDO;
                    tempoExecutadoNoQuantum = 0;
                } else {
                    processoAtual = nullptr;
                }
            }

            // Execução do processo atual
            if (processoAtual) {
                processoAtual->tempoRestante--;
                tempoExecutadoNoQuantum++;
                cout << "[Tempo " << tempo << "] Executando: " << processoAtual->pid
                     << " | Tempo restante: " << processoAtual->tempoRestante << "\n";
            } else {
                cout << "[Tempo " << tempo << "] CPU ociosa.\n";
            }

            tempo++;
        }
    }

    bool todosFinalizados() {
        for (auto p : todosProcessos)
            if (p->estado != Processo::FINALIZADO)
                return false;
        return true;
    }
};

int main() {
    ifstream arquivo("entrada.txt");
    string linha;
    string algoritmo;
    int quantum;

    // Leitura do cabeçalho
    getline(arquivo, linha);
    stringstream ss(linha);
    getline(ss, algoritmo, '|');
    if (!algoritmo.empty() && algoritmo.back() == '\r') {
        algoritmo.pop_back();
    }

    if (algoritmo != "ROUND_ROBIN") {
        cerr << "Erro: Este escalonador implementa apenas o algoritmo ROUND_ROBIN.\n";
        return 1;
    }

    Escalonador escalonador(quantum);

    // Leitura dos processos
    while (getline(arquivo, linha)) {
        stringstream sl(linha);
        string pid, tmp;
        int momento, tempo, prioridade;

        getline(sl, tmp, '|'); momento = stoi(tmp);
        getline(sl, pid, '|');
        getline(sl, tmp, '|'); tempo = stoi(tmp);
        getline(sl, tmp, '|'); prioridade = stoi(tmp);

        escalonador.adicionarProcesso(new Processo(momento, pid, tempo, prioridade));
    }

    escalonador.simular();
    return 0;
}
