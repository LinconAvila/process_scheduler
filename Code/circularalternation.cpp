#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>
#include <iomanip>

using namespace std;

struct Processo {
    enum Estado { PRONTO, EXECUTANDO, BLOQUEADO, FINALIZADO };

    int momentoCriacao;
    string pid;
    int tempoRestante;
    int prioridade; // ignorado em RR
    Estado estado;
    bool criado = false;

    Processo(int m, string p, int t, int prio)
        : momentoCriacao(m), pid(p), tempoRestante(t), prioridade(prio), estado(PRONTO) {}

    string estadoComoString() const {
        switch (estado) {
            case PRONTO: return "PRONTO";
            case EXECUTANDO: return "EXECUTANDO";
            case BLOQUEADO: return "BLOQUEADO";
            case FINALIZADO: return "FINALIZADO";
            default: return "DESCONHECIDO";
        }
    }
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

            // Criação de novos processos
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

            // Troca por quantum ou ausência
            if (!processoAtual || tempoExecutadoNoQuantum == quantum) {
                if (processoAtual && processoAtual->tempoRestante > 0) {
                    processoAtual->estado = Processo::PRONTO;
                    filaProntos.push(processoAtual);
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

            // Executa o processo atual
            if (processoAtual) {
                processoAtual->tempoRestante--;
                tempoExecutadoNoQuantum++;
            }

            // Imprime o estado de todos os processos
            cout << "[Tempo " << tempo << "] Estado dos processos:\n";
            for (auto& p : todosProcessos) {
                cout << "    " << setw(4) << p->pid 
                     << " | Estado: " << setw(11) << p->estadoComoString() 
                     << " | Restante: " << p->tempoRestante << "\n";
            }

            if (!processoAtual)
                cout << "[Tempo " << tempo << "] CPU ociosa.\n";

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

    // Cabeçalho: ROUND_ROBIN|2
    getline(arquivo, linha);
    stringstream ss(linha);
    getline(ss, algoritmo, '|');
    ss >> quantum;

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
