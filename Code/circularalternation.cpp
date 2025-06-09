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
    Estado estado;
    bool criado = false;

    int tempoInicioExecucao = -1;
    int tempoFinalizacao = -1;
    int tempoEmPronto = 0;

    Processo(int m, string p, int t)
        : momentoCriacao(m), pid(p), tempoRestante(t), estado(PRONTO) {}

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

            for (auto& p : todosProcessos) {
                if (!p->criado && p->momentoCriacao == tempo) {
                    cout << "[Tempo " << tempo << "] Processo " << p->pid << " criado.\n";
                    p->criado = true;
                    p->estado = Processo::PRONTO;
                    filaProntos.push(p);
                }
            }

            for (auto& p : todosProcessos) {
                if (p->estado == Processo::PRONTO)
                    p->tempoEmPronto++;
            }

            if (processoAtual && processoAtual->tempoRestante == 0) {
                processoAtual->estado = Processo::FINALIZADO;
                processoAtual->tempoFinalizacao = tempo;
                cout << "[Tempo " << tempo << "] Processo " << processoAtual->pid << " finalizado no tempo " << tempo << ".\n";
                processoAtual = nullptr;
                tempoExecutadoNoQuantum = 0;
            }

            if (!processoAtual || tempoExecutadoNoQuantum == quantum) {
                if (processoAtual && processoAtual->tempoRestante > 0) {
                    processoAtual->estado = Processo::PRONTO;
                    filaProntos.push(processoAtual);
                }

                if (!filaProntos.empty()) {
                    processoAtual = filaProntos.front();
                    filaProntos.pop();
                    processoAtual->estado = Processo::EXECUTANDO;
                    if (processoAtual->tempoInicioExecucao == -1)
                        processoAtual->tempoInicioExecucao = tempo;
                    tempoExecutadoNoQuantum = 0;
                } else {
                    processoAtual = nullptr;
                }
            }

            if (processoAtual) {
                processoAtual->tempoRestante--;
                tempoExecutadoNoQuantum++;
                cout << "[Tempo " << tempo << "] CPU executando " << processoAtual->pid
                     << " | Tempo restante no processo: " << processoAtual->tempoRestante << "\n";
            } else {
                cout << "[Tempo " << tempo << "] CPU ociosa.\n";
            }

            cout << "[Tempo " << tempo << "] Estado dos processos:\n";
            for (auto& p : todosProcessos) {
                cout << "    " << setw(4) << p->pid 
                     << " | Estado: " << setw(11) << p->estadoComoString() 
                     << " | Restante: " << p->tempoRestante << "\n";
            }

            tempo++;
        }

        cout << "\n--- Simulacao finalizada no tempo " << tempo << " ---\n";
        cout << "\n--- Estatisticas Finais ---\n";
        cout << left << setw(10) << "PID" << setw(25) << "Tempo de vida" << "Tempo Pronto" << "\n";
        for (auto& p : todosProcessos) {
            int tempoVida = p->tempoFinalizacao - p->momentoCriacao;
            cout << left << setw(10) << p->pid
                 << setw(25) << tempoVida
                 << p->tempoEmPronto << "\n";
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
        string tmp;
        int momento, tempo, prioridade;
        string pid;

        getline(sl, tmp, '|'); momento = stoi(tmp);
        getline(sl, tmp, '|'); pid = tmp; // aceita pid numérico como string
        getline(sl, tmp, '|'); tempo = stoi(tmp);
        getline(sl, tmp, '|'); prioridade = stoi(tmp);

        escalonador.adicionarProcesso(new Processo(momento, pid, tempo));
    }

    escalonador.simular();
    return 0;
}