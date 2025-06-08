#include <iostream>    // Para entrada/saída (std::cout, std::cin, std::cerr)
#include <fstream>     // Para manipulação de arquivos (std::ifstream)
#include <sstream>     // Para manipulação de strings (std::stringstream)
#include <string>      // Para std::string
#include <vector>      // Para std::vector
#include <algorithm>   // Para std::min
#include <iomanip>     // Para std::setw (formatação de saída)
#include <limits>      // Para std::numeric_limits

// =======================================================
// Variável global para o tempo atual da simulação
// É crucial para o controle de tempo no scheduler e nos processos (start_time, end_time).
// =======================================================
long long current_time_global = 0;

// =======================================================
// Enums e Protótipos de Classes
// Correção: Enums são definidos completamente aqui, antes de serem usados.
// =======================================================
enum Color {Red, Black}; // Definido para a Árvore Rubro-Negra

enum ProcessState { // Definido para os estados dos processos
    READY,
    RUNNING,
    TERMINATED
};

// Protótipos das classes
class Process;
class Node;
class Tree; // Mantendo o nome Tree
class Scheduler;


// =======================================================
// 1. Classe Process (Adaptada para CFS)
// =======================================================
class Process
{
public:
    Process(int pid, int creation_time, int burst_time, int tickets = 0);

    // Getters
    int get_pid() const { return pid; }
    int get_creation_time() const { return creation_time; }
    int get_end_time() const { return end_time; }
    int get_total_waiting_time() const { return total_waiting_time; } // Usado para estatísticas

    // Método para imprimir informações do processo
    void print_info() const;

private: // Membros privados
    int pid;
    int creation_time;
    int burst_time;
    int remaining_time;
    int start_time;
    int end_time;
    bool is_finished; // Indica se o processo terminou
    int tickets;      // Usado como peso (weights) no CFS
    int weights;      // Peso efetivo no CFS (diretamente dos tickets)
    int total_waiting_time; // Tempo total que o processo passou na fila de prontos

public: // Atributos CFS (tornados públicos para acesso direto no mini-projeto)
    long double vruntime; // Tempo de execução virtual, crucial para o CFS
    ProcessState state;   // Estado atual do processo

    // Classes amigas para acesso direto aos membros privados, conforme seu design original.
    friend class Node;     // Node precisa acessar Process::process
    friend class Tree;     // Tree precisa acessar Process::vruntime e Process::pid
    friend class Scheduler; // Scheduler precisa acessar vários membros de Process
};

// Implementação do Construtor de Process
Process::Process(int pid_val, int creation_time_val, int burst_time_val, int tickets_val)
{
    this->pid = pid_val;
    this->creation_time = creation_time_val;
    this->burst_time = burst_time_val;
    this->remaining_time = burst_time_val;
    this->start_time = -1;
    this->end_time = -1;
    this->tickets = tickets_val;
    this->weights = tickets_val; // Tickets usados como weights para o CFS
    this->is_finished = false;
    this->total_waiting_time = 0;
    this->vruntime = 0.0;        // Inicialização do vruntime
    this->state = READY;         // Inicialização do estado
}

// Implementação de print_info para Process
void Process::print_info() const {
    std::cout << "PID: " << pid
              << ", Burst: " << burst_time << "ms"
              << ", Criado: " << creation_time << "ms"
              << ", Tickets/Peso: " << weights
              << ", vruntime: " << vruntime
              << ", Tempo Restante: " << remaining_time << "ms"
              << ", Estado: ";
    switch (state) {
        case READY: std::cout << "READY"; break;
        case RUNNING: std::cout << "RUNNING"; break;
        case TERMINATED: std::cout << "TERMINATED"; break;
    }
    std::cout << std::endl;
}


// =======================================================
// 2. Classe FileReader
// Mantida do seu código original, responsável por ler o arquivo de entrada.
// =======================================================
class FileReader
{
public:
    FileReader(const std::string &filename);
    void read_file();
    std::string get_algorithm() const;
    int get_quantum() const;
    const std::vector<int> &get_pids() const;
    const std::vector<int> &get_creation_times() const;
    const std::vector<int> &get_burst_times() const;
    const std::vector<int> &get_ticket_values() const;

private: // Membros privados declarados no local correto
    std::string filename;
    std::string algorithm;
    int quantum;
    std::vector<int> ticket_values;
    std::vector<int> pids;
    std::vector<int> burst_times;
    std::vector<int> creation_times;
};

// Implementação do Construtor de FileReader
FileReader::FileReader(const std::string &filename_val) : filename(filename_val)
{
    this->quantum = 0;
}

// Implementação de read_file para FileReader
void FileReader::read_file()
{
    std::ifstream file(filename);
    std::string line;

    if (!file.is_open())
    {
        std::cerr << "Erro ao abrir o arquivo: " << filename << std::endl;
        return;
    }

    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::getline(ss, algorithm, '|');
        std::string quantum_str;
        std::getline(ss, quantum_str);
        quantum = std::stoi(quantum_str);
    }

    while (std::getline(file, line))
    {
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

// Implementação dos Getters de FileReader
std::string FileReader::get_algorithm() const { return algorithm; }
int FileReader::get_quantum() const { return quantum; }
const std::vector<int> &FileReader::get_pids() const { return pids; }
const std::vector<int> &FileReader::get_creation_times() const { return creation_times; }
const std::vector<int> &FileReader::get_burst_times() const { return burst_times; }
const std::vector<int> &FileReader::get_ticket_values() const { return ticket_values; }


// =======================================================
// 3. Classes Node e Tree (Árvore Rubro-Negra)
// A classe Tree foi corrigida para a sintaxe C++ padrão (declarações e definições separadas)
// e para o funcionamento correto da árvore Rubro-Negra.
// =======================================================
class Node {
public:
    Process* process; // Corrigido de 'Process* Process' para 'Process* process'
    Color color;
    Node* left;
    Node* right;
    Node* parent;

    Node(Process* p, Color c, Node* par = nullptr) :
        process(p), color(c), parent(par), left(nullptr), right(nullptr) {}
};

class Tree { // Mantendo o nome 'Tree'
public: // Todos os membros são públicos conforme sua preferência
    Node* root;
    Node* NIL; // representar folhas nulas (sentinela)

    // Construtor
    Tree();

    // Destrutor
    ~Tree();

    // Métodos auxiliares
    void rotate_left(Node* x);
    void rotate_right(Node* x);
    void fix_insert(Node* z);
    void transplant(Node* u, Node* v);
    Node* tree_minimum(Node* node);
    void fix_delete(Node* x); // Implementação esquelética
    void in_order_traverse(Node* node) const;
    void delete_tree_nodes(Node* node); 

    // Operações da árvore
    void insert(Process* new_process_ptr); // Recebe um ponteiro para o Processo
    Process* get_min_vruntime_process() const; // Obtém o processo com menor vruntime
    Node* search_node_by_pid(int pid) const; // Busca um nó pelo PID do processo (para remoção)
    void remove_process(int pid); // Remove um processo pela PID
    void in_order() const; // Travessia em ordem (interface pública)
    
    bool is_empty() const { 
        return root == NIL; 
    }
};

// Implementação do Construtor da Tree
Tree::Tree() {
    NIL = new Node(nullptr, Black);
    NIL->left = NIL;
    NIL->right = NIL;
    NIL->parent = NIL;
    root = NIL;
}

// Implementação do Destrutor da Tree
Tree::~Tree() {
    delete_tree_nodes(root);
    delete NIL;
}

// Implementação de delete_tree_nodes (recursivo para liberar memória dos nós)
void Tree::delete_tree_nodes(Node* node) {
    if (node == NIL) {
        return;
    }
    delete_tree_nodes(node->left);
    delete_tree_nodes(node->right);
    delete node; // Deleta o nó atual (mas não o Processo*, que é gerenciado pelo Scheduler)
}

// Implementação de rotate_left (rotação à esquerda)
void Tree::rotate_left(Node* x){
    Node* y = x->right;
    x->right = y->left; 

    if(y->left != NIL){
        y->left->parent = x;
    }

    y->parent = x->parent;

    if(x->parent == NIL){ // Se x era a raiz (seu pai é NIL)
        root = y;
    }
    else if(x == x->parent->left){ // Se x era filho esquerdo
        x->parent->left =y;
    }
    else{ // Se x era filho direito
         x->parent->right = y;
    }

    y->left = x; // Coloca x como filho esquerdo de y
    x->parent = y; // Atualiza o pai de x para y
}

// Implementação de rotate_right (rotação à direita)
void Tree::rotate_right(Node* x){
    Node* y = x->left;
    x->left = y->right;

    if(y->right != NIL){
        y->right->parent = x;
    }

    y->parent = x->parent;

    if(x->parent == NIL){
        root = y;
    }
    else if(x == x->parent->right){ // Se x era filho direito
        x->parent->right = y;
    }
    else{ // Se x era filho esquerdo
        x->parent->left = y;
    }

    y->right = x; // Coloca x como filho direito de y
    x->parent = y; // Atualiza o pai de x para y
}

// Implementação de fix_insert (corrige violações após inserção)
void Tree::fix_insert(Node* z){
    while(z->parent->color == Red){ // Enquanto o pai de z é vermelho (violação da propriedade 4)
        Node* parent = z->parent;
        Node* grandparent = parent->parent;

        if(parent == grandparent->left){ // Caso 1: Pai é filho esquerdo do avô
            Node* uncle = grandparent->right;

            if(uncle->color == Red){ // Caso 1.1: Tio é vermelho
                parent->color = Black;
                uncle->color = Black;
                grandparent->color = Red;
                z = grandparent; // Move z para cima para continuar a correção
            } else { // Caso 1.2: Tio é preto
                if (z == parent->right) { // Caso 1.2.1: Z é filho direito do pai (caso triângulo)
                    z = parent;
                    rotate_left(z); // Rotação à esquerda no pai
                    parent = z->parent; // Atualiza o pai após a rotação
                }
                // Caso 1.2.2: Z é filho esquerdo do pai (ou se tornou após a rotação anterior)
                parent->color = Black;
                grandparent->color = Red;
                rotate_right(grandparent); // Rotação à direita no avô
            }
        } else { // Caso simétrico: Pai é filho direito do avô
            Node* uncle = grandparent->left;

            if(uncle->color == Red){ // Caso 2.1: Tio é vermelho
                parent->color = Black;
                uncle->color = Black;
                grandparent->color = Red;
                z = grandparent;
            } else { // Caso 2.2: Tio é preto
                if (z == parent->left) { // Caso 2.2.1: Z é filho esquerdo do pai (caso triângulo)
                    z = parent;
                    rotate_right(z);
                    parent = z->parent; // Atualiza o pai após a rotação
                }
                // Caso 2.2.2: Z é filho direito do pai (ou se tornou após a rotação anterior)
                parent->color = Black;
                grandparent->color = Red;
                rotate_left(grandparent);
            }
        }
    }
    root->color = Black; // A raiz deve ser sempre preta
}

// Implementação de insert (insere um novo processo na árvore)
void Tree::insert(Process* new_process_ptr){
    Node* new_node = new Node(new_process_ptr, Red); // Novos nós são sempre vermelhos inicialmente
    new_node->left = NIL; // Filhos de novos nós apontam para NIL
    new_node->right = NIL;

    Node* parent = NIL; // Inicia parent como NIL
    Node* current = root;

    // Busca a posição correta para inserção como em uma BST
    while (current != NIL) {
        parent = current;
        if(new_process_ptr->vruntime < current->process->vruntime) {
            current = current->left;
        } 
        else if(new_process_ptr->vruntime > current->process->vruntime) {
            current = current->right;
        } 
        else {
            // Desempate por PID se vruntime for igual (garante unicidade da chave para Tree)
            if (new_process_ptr->pid < current->process->pid) { 
                current = current->left;
            } 
            else if (new_process_ptr->pid > current->process->pid) {
                current = current->right;
            } 
            else {
                delete new_node; // Se PID e vruntime são iguais, já existe, não insere
                return;
            }
        }
    }

    new_node->parent = parent; // Define o pai do novo nó

    // Conecta o novo nó à árvore
    if(parent == NIL) { // Árvore vazia, new_node é a raiz
        root = new_node;
    } 
    else if (new_process_ptr->vruntime < parent->process->vruntime) {
        parent->left = new_node;
    } 
    else if(new_process_ptr->vruntime > parent->process->vruntime) {
        parent->right = new_node;
    } 
    else {
        // Desempate por PID novamente se vruntime for igual
        if (new_process_ptr->pid < parent->process->pid) { 
            parent->left = new_node;
        } 
        else {
            parent->right = new_node;
        }
    }

    fix_insert(new_node); // Corrige as propriedades da Árvore Rubro-Negra
}

// Implementação de get_min_vruntime_process (retorna o processo com menor vruntime, o mais à esquerda)
Process* Tree::get_min_vruntime_process() const {
    if(root == NIL) {
        return nullptr;
    }
    Node* current = root;
    while(current->left != NIL) {
        current = current->left;
    }
    return current->process; // Retorna o Processo* armazenado no nó
}

// Implementação de search_node_by_pid (busca um nó pelo PID do processo)
Node* Tree::search_node_by_pid(int pid) const {
    std::vector<Node*> queue_nodes;
    if (root != NIL) {
        queue_nodes.push_back(root);
    }
    int head = 0;
    // Correção: Comparação entre signed e unsigned integer expressions (head < queue_nodes.size())
    while (static_cast<size_t>(head) < queue_nodes.size()) { 
        Node* node = queue_nodes[head++];
        if (node->process->pid == pid) {
            return node;
        }
        if (node->left != NIL) queue_nodes.push_back(node->left);
        if (node->right != NIL) queue_nodes.push_back(node->right);
    }
    return nullptr;
}

// Implementação de transplant (auxiliar para remoção)
void Tree::transplant(Node* u, Node* v) {
    if(u->parent == NIL) {
        root = v;
    } 
    else if(u == u->parent->left) {
        u->parent->left = v;
    } 
    else{
        u->parent->right = v;
    }
    if(v != NIL) { // Garante que v não é o NIL antes de atribuir o pai
        v->parent = u->parent;
    } 
}

// Implementação de tree_minimum (auxiliar para remoção: encontra o sucessor in-order)
Node* Tree::tree_minimum(Node* node) {
    while (node->left != NIL) {
        node = node->left;
    }
    return node;
}

// Implementação de fix_delete (corrige violações após remoção - Esquelética, com a lógica dos 4 casos)
// Note: Uma implementação completa e robusta exige depuração extensiva.
void Tree::fix_delete(Node* x) {
    while (x != root && x->color == Black) {
        if (x == x->parent->left) { // Caso 1: `x` é filho esquerdo de seu pai
            Node* w = x->parent->right; // `w` é o irmão de `x`

            if (w->color == Red) { // Caso 1: Irmão `w` é Vermelho.
                w->color = Black;
                x->parent->color = Red;
                rotate_left(x->parent);
                w = x->parent->right; // Atualiza w para o novo irmão
            }

            if (w->left->color == Black && w->right->color == Black) { // Caso 2: Irmão `w` é Preto, e ambos os filhos de `w` são Pretos.
                w->color = Red;
                x = x->parent; // Move a "dupla-preta" para cima
            } 
            else { 
                if (w->right->color == Black) { // Caso 3: Irmão `w` é Preto, filho esquerdo de `w` é Vermelho, filho direito de `w` é Preto.
                    w->left->color = Black;
                    w->color = Red;
                    rotate_right(w);
                    w = x->parent->right; // Atualiza w para o novo irmão
                }
                // Caso 4: Irmão `w` é Preto, e filho direito de `w` é Vermelho.
                w->color = x->parent->color;
                x->parent->color = Black;
                w->right->color = Black;
                rotate_left(x->parent);
                x = root; // A árvore está balanceada, termina o loop
            }
        } else { // Caso simétrico: `x` é filho direito de seu pai
            Node* w = x->parent->left; // `w` é o irmão de `x`

            if (w->color == Red) { // Caso 1 (simétrico): Irmão `w` é Vermelho.
                w->color = Black;
                x->parent->color = Red;
                rotate_right(x->parent);
                w = x->parent->left; // Atualiza w para o novo irmão
            }

            if (w->right->color == Black && w->left->color == Black) { // Caso 2 (simétrico): Irmão `w` é Preto, e ambos os filhos de `w` são Pretos.
                w->color = Red;
                x = x->parent; // Move a "dupla-preta" para cima
            } 
            else {
                if (w->left->color == Black) { // Caso 3 (simétrico): Irmão `w` é Preto, filho direito de `w` é Vermelho, filho esquerdo de `w` é Preto.
                    w->right->color = Black;
                    w->color = Red;
                    rotate_left(w);
                    w = x->parent->left; // Atualiza w para o novo irmão
                }
                // Caso 4 (simétrico): Irmão `w` é Preto, e filho esquerdo de `w` é Vermelho.
                w->color = x->parent->color;
                x->parent->color = Black;
                w->left->color = Black;
                rotate_right(x->parent);
                x = root; // A árvore está balanceada, termina o loop
            }
        }
    }
    if (x != NIL) {
        x->color = Black;
    }
}

// Implementação de remove_process (remove um nó da árvore pela PID do processo)
void Tree::remove_process(int pid) {
    Node* z = search_node_by_pid(pid);
    if(z == nullptr) { // Se o nó não for encontrado
        return;
    }

    Node* y = z;
    Node* x = NIL; // Inicializado com NIL para consistência
    Color original_y_color = y->color;

    if(z->left == NIL) { // Caso 1: Z não tem filho esquerdo
        x = z->right;
        transplant(z, z->right);
    } 
    else if(z->right == NIL) { // Caso 2: Z não tem filho direito
        x = z->left;
        transplant(z, z->left);
    } 
    else { // Caso 3: Z tem ambos os filhos
        y = tree_minimum(z->right); // Encontra o sucessor de z na subárvore direita (o menor da subárvore direita)
        original_y_color = y->color; // Guarda a cor original do sucessor
        x = y->right; // x será o filho direito de y

        if (y->parent == z) { // Se y é filho direto de z
            // x já tem o pai correto (y), não precisa de `x->parent = y;`
        } 
        else { // Se y não é filho direto de z
            transplant(y, y->right); // Move o filho direito de y para a posição de y
            y->right = z->right; // O filho direito de z se torna o filho direito de y
            y->right->parent = y; // Atualiza o pai do antigo filho direito de z
        }
        transplant(z, y); // Move y para a posição de z
        y->left = z->left; // O filho esquerdo de z se torna o filho esquerdo de y
        y->left->parent = y; // Atualiza o pai do antigo filho esquerdo de z
        y->color = z->color; // A cor de y se torna a cor original de z
    }

    delete z; // Libera a memória do nó (mas não do Processo*, que é gerenciado pelo Scheduler)

    if (original_y_color == Black) { // Se um nó preto foi removido ou movido, é necessário balancear
        fix_delete(x);
    }
}

// Implementação de in_order (inicia a travessia em ordem)
void Tree::in_order() const {
    in_order_traverse(root);
}

// Implementação de in_order_traverse (função auxiliar recursiva para travessia em ordem)
void Tree::in_order_traverse(Node* node) const {
    if (node != NIL) {
        in_order_traverse(node->left);
        std::cout << "(PID:" << node->process->pid << ", vR:" << node->process->vruntime
                  << (node->color == Red ? "R) " : "B) ");
        in_order_traverse(node->right);
    }
}


// =======================================================
// 4. Classe Scheduler (CFS)
// Responsável pela lógica principal do escalonamento CFS.
// =======================================================

// Constantes do CFS
// Correção Principal 4: As constantes do CFS são declaradas globalmente (fora de qualquer classe).
const long double TARGET_LATENCY = 20.0; // Milissegundos (o tempo ideal para cada processo rodar uma vez)
const long double MIN_GRANULARITY = 1.0; // Milissegundos (tempo mínimo que um processo deve executar)
const long double NICE_0_WEIGHT_APPROX = 1024.0; // Uma aproximação do peso para nice=0 no Linux


class Scheduler { 
public:
    Tree run_queue; // Fila de processos prontos (Árvore Rubro-Negra)
    Process* current_process; // Processo atualmente na CPU
    
    std::vector<Process*> all_processes; // Lista de todos os processos (gerencia a vida útil)

    long double system_vruntime_min; // Menor vruntime em todo o sistema (referência para justiça)
    long long total_active_weight;   // Soma dos pesos de todos os processos na run_queue
    
    long long SCHED_HZ; // Frequência do timer (quantum de simulação)

    Scheduler() : current_process(nullptr), system_vruntime_min(0.0), total_active_weight(0), SCHED_HZ(0) {
        // O construtor de Tree() é chamado automaticamente aqui para run_queue.
    }

    ~Scheduler() {
        // Libera a memória de todos os objetos Processo alocados dinamicamente
        for (Process* p : all_processes) {
            delete p; 
        }
        all_processes.clear();
    }

    void init_scheduler(int quantum_from_file) {
        // Limpa processos antigos e reinicializa o estado do scheduler
        for (Process* p : all_processes) {
            delete p; 
        }
        all_processes.clear();

        run_queue = Tree(); // Re-inicializa a run_queue (cria uma nova instância da árvore vazia)

        current_process = nullptr;
        system_vruntime_min = 0.0; 
        total_active_weight = 0;
        SCHED_HZ = quantum_from_file; // O quantum do arquivo é usado como o tick de simulação

        std::cout << "Scheduler inicializado com Quantum/SCHED_HZ: " << SCHED_HZ << "ms." << std::endl;
    }

    // Adiciona um novo processo ao scheduler
    void add_process(int pid, int creation_time, int burst_time, int tickets) {
        Process* new_process = new Process(pid, creation_time, burst_time, tickets);
        all_processes.push_back(new_process); // Adiciona à lista geral de processos

        // Inicializa o vruntime do novo processo.
        // Se a run_queue não estiver vazia, o vruntime inicial é o do processo mais "justo" (menor vruntime).
        // Caso contrário, é o system_vruntime_min.
        if (!run_queue.is_empty()) {
            new_process->vruntime = run_queue.get_min_vruntime_process()->vruntime;
        } 
        else {
            new_process->vruntime = system_vruntime_min;
        }
        
        run_queue.insert(new_process); // Insere o processo na fila de prontos (Árvore Rubro-Negra)
        total_active_weight += new_process->weights; // Atualiza o peso total ativo

        std::cout << "Processo " << new_process->pid << " (peso: " << new_process->weights << ") adicionado ao scheduler. vruntime inicial: " << new_process->vruntime << std::endl;
    }

    // Seleciona o próximo processo para ser executado
    Process* pick_next_task() {
        if (run_queue.is_empty()) {
            current_process = nullptr;
            return nullptr; // Nenhuma tarefa pronta
        }

        Process* next_task_ptr = run_queue.get_min_vruntime_process(); // Obtém o processo com menor vruntime
        run_queue.remove_process(next_task_ptr->get_pid()); // Remove o nó correspondente da árvore
        
        current_process = next_task_ptr;
        current_process->state = RUNNING; // Define o estado como em execução
        return current_process;
    }

    // Simula um tick de tempo (a lógica principal do escalonamento)
    void schedule_tick(long long delta_time) {
        // 1. Se não há processo rodando, escolha o próximo
        if (current_process == nullptr) {
            current_process = pick_next_task();
            if (current_process == nullptr) {
                // Nenhum processo pronto para executar, sistema ocioso.
                system_vruntime_min += delta_time; // Avança o vruntime mínimo do sistema
                return;
            }
            std::cout << "--> Iniciando processo PID: " << current_process->get_pid() << " (vR: " << current_process->vruntime << ")" << std::endl;
            if (current_process->start_time == -1) { // Registra o tempo de início da primeira execução
                current_process->start_time = current_time_global;
            }
        }

        // 2. Atualiza o vruntime e o tempo restante do processo atual
        // Aumenta o vruntime de forma proporcional ao tempo de execução e inversamente proporcional ao peso.
        long double vruntime_increment = static_cast<long double>(delta_time) * (NICE_0_WEIGHT_APPROX / current_process->weights);
        current_process->vruntime += vruntime_increment;
        current_process->remaining_time -= delta_time;

        // 3. Atualiza o vruntime mínimo do sistema (referência para justiça)
        if (!run_queue.is_empty()) {
            system_vruntime_min = std::min(current_process->vruntime, run_queue.get_min_vruntime_process()->vruntime);
        } else {
            system_vruntime_min = current_process->vruntime; // Se só tem o processo atual rodando
        }

        // 4. Verifica o término do processo
        if (current_process->remaining_time <= 0) {
            current_process->state = TERMINATED;
            total_active_weight -= current_process->weights; // Remove o peso do processo terminado
            current_process->end_time = current_time_global + delta_time; // Registra o tempo de término
            std::cout << "<-- Processo PID: " << current_process->get_pid() << " TERMINOU." << std::endl;
            
            current_process = nullptr; // CPU fica livre
            current_process = pick_next_task(); // Escolhe o próximo imediatamente
            if (current_process) {
                std::cout << "--> Iniciando processo PID: " << current_process->get_pid() << " (vR: " << current_process->vruntime << ")" << std::endl;
                if (current_process->start_time == -1) { // Registra o tempo de início se for a primeira vez
                    current_process->start_time = current_time_global + delta_time;
                }
            }
            return; // Retorna para o próximo tick, um novo processo já está executando ou CPU ociosa
        }

        // 5. Verifica a preempção (troca de contexto)
        // Ocorre se o processo atual está "atrasando" outros (seu vruntime é muito maior que o próximo da fila).
        if (!run_queue.is_empty()) {
            Process* next_in_queue = run_queue.get_min_vruntime_process();
            if (current_process->vruntime > next_in_queue->vruntime + MIN_GRANULARITY) {
                run_queue.insert(current_process); // Reinserir o processo atual na fila (será ordenado pelo novo vruntime)
                current_process->state = READY; // Volta para estado de pronto
                std::cout << "<-- Processo PID: " << current_process->get_pid() << " preempção (vR: " << current_process->vruntime << " vs. " << next_in_queue->vruntime << "). " << std::endl;
                
                current_process = pick_next_task(); // Escolhe o próximo processo (o que tinha menor vruntime)
                if (current_process) {
                    std::cout << "--> Iniciando processo PID: " << current_process->get_pid() << " (vR: " << current_process->vruntime << ")" << std::endl;
                    if (current_process->start_time == -1) { // Registra o tempo de início se for a primeira vez
                        current_process->start_time = current_time_global + delta_time;
                    }
                }
            }
        }
    }

    // Calcula a fatia de tempo ideal (quantum) para um processo.
    // No CFS, a preempção é baseada no vruntime, mas esta função pode ser usada para referência.
    long double calculate_time_slice(const Process& p) const {
        if (total_active_weight == 0) {
            return MIN_GRANULARITY; // Evita divisão por zero
        }
        long double base_time_slice = (static_cast<long double>(p.weights) / total_active_weight) * TARGET_LATENCY;
        if (base_time_slice < MIN_GRANULARITY) {
            base_time_slice = MIN_GRANULARITY;
        }
        return base_time_slice;
    }
     
    // Imprime o estado atual do escalonador e das filas.
    void print_scheduler_state() const {
        std::cout << "\n--- Estado Atual do Scheduler (Tick: " << SCHED_HZ << "ms) ---" << std::endl;
        std::cout << "Tempo Virtual Mínimo do Sistema (system_vruntime_min): " << system_vruntime_min << std::endl;
        std::cout << "Peso Total Ativo (total_active_weight): " << total_active_weight << std::endl;

        if (current_process) {
            std::cout << "Processo Atualmente Executando:" << std::endl;
            current_process->print_info();
        } else {
            std::cout << "CPU Ociosa (Nenhum processo executando)." << std::endl;
        }

        std::cout << "\nFila de Processos Prontos (Run Queue - Tree In-Order):" << std::endl;
        run_queue.in_order();
        if (run_queue.is_empty()) {
            std::cout << "  (Vazia)" << std::endl;
        }

        std::cout << "\nTodos os Processos Conhecidos (incluindo terminados):" << std::endl;
        if (all_processes.empty()) {
            std::cout << "  (Nenhum processo conhecido)" << std::endl;
        } else {
            for (const auto& p_ptr : all_processes) {
                std::cout << "  ";
                p_ptr->print_info();
            }
        }
        std::cout << "--------------------------------------------------------\n" << std::endl;
    }

    // Método para imprimir estatísticas finais dos processos
    void print_statistics() const {
        std::cout << "\n--- Estatisticas Finais do CFS ---\n";
        std::cout << std::left << std::setw(10) << "PID"
                  << std::setw(20) << "Tempo Total" // Turnaround Time
                  << std::setw(20) << "Tempo Pronto" // Waiting Time
                  << std::setw(15) << "Burst Time"
                  << std::endl;
        std::cout << "------------------------------------------------------------------\n";

        for (const auto &p_ptr : all_processes) {
            if (p_ptr->state == TERMINATED) {
                int turnaround_time = p_ptr->get_end_time() - p_ptr->get_creation_time();
                // O waiting_time no CFS não é acumulado da mesma forma que em RR/Prioridade.
                // Para uma métrica tradicional, seria turnaround_time - burst_time.
                int waiting_time = turnaround_time - p_ptr->burst_time; 
                
                std::cout << std::left << std::setw(10) << p_ptr->get_pid()
                          << std::setw(20) << turnaround_time
                          << std::setw(20) << waiting_time
                          << std::setw(15) << p_ptr->burst_time
                          << std::endl;
            }
        }
        std::cout << "------------------------------------------------------------------\n";
    }
};


// =======================================================
// 5. Função principal (main) - Executa APENAS o CFS
// =======================================================

int main(){
    std::cout << "Este programa executa a simulacao do Completely Fair Scheduler (CFS)." << std::endl;
    std::cout << "Digite o nome do arquivo de entrada: ";
    std::string filename;
    std::cin >> filename;

    FileReader file_reader(filename);
    file_reader.read_file();

    if (file_reader.get_pids().empty()) { // Verifica se há processos para simular
        std::cerr << "Nenhum processo lido do arquivo. Saindo." << std::endl;
        return 1;
    }

    Scheduler cfs_scheduler; // Instância do seu Scheduler CFS
    cfs_scheduler.init_scheduler(file_reader.get_quantum());

    const std::vector<int> &pids = file_reader.get_pids();
    const std::vector<int> &creation_times = file_reader.get_creation_times();
    const std::vector<int> &burst_times = file_reader.get_burst_times();
    const std::vector<int> &ticket_values = file_reader.get_ticket_values();

    for (size_t i = 0; i < pids.size(); ++i) {
        // Adiciona os processos ao CFS Scheduler
        cfs_scheduler.add_process(
            pids[i],
            creation_times[i],
            burst_times[i],
            ticket_values[i]
        );
    }

    long long delta_time = cfs_scheduler.SCHED_HZ; 
    long long max_simulation_time = 1000; // Tempo máximo de simulação em ms (pode ajustar)

    std::cout << "\nIniciando simulação do CFS..." << std::endl;

    while (current_time_global < max_simulation_time) {
        std::cout << "\n==========================================" << std::endl;
        std::cout << "Tempo de Simulação Atual: " << current_time_global << "ms" << std::endl;

        cfs_scheduler.schedule_tick(delta_time);
        cfs_scheduler.print_scheduler_state();
        
        current_time_global += delta_time; // Atualiza o tempo global da simulação

        bool all_terminated = true;
        for (const auto& p_ptr : cfs_scheduler.all_processes) {
            if (p_ptr->state != TERMINATED) {
                all_terminated = false;
                break;
            }
        }
        if (all_terminated) {
            std::cout << "Todos os processos terminaram!" << std::endl;
            break;
        }
        // Opcional: Adicionar um pequeno atraso para facilitar a leitura no console
        // #include <chrono> e #include <thread> seriam necessários para isso
        // std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    }
    std::cout << "\nSimulação CFS finalizada." << std::endl;

    // Imprime as estatísticas finais do CFS
    cfs_scheduler.print_statistics();

    return 0;
}