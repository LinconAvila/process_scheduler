#include <iostream>

// prototipos classes enuns e structures
class Node;
class Tree;
class Schedule;
enum Color;
enum ProcessState;


class Process
{
public:
    Process(int pid, int creation_time, int burst_time, int tickets = 0);
    int get_pid() const;
    int get_creation_time() const;
    int get_end_time() const;
    int get_total_waiting_time() const;

private:
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

    // leco aqui vai ter alteracões para o meu funcionar
    long double vruntime; // Adicionado para o CFS
    ProcessState state;   // Adicionado para o CFS

    friend class LotteryScheduler;
    friend class PriorityScheduler;
    friend struct CompareProcessPriority;
};

Process::Process(int pid, int creation_time, int burst_time, int tickets)
{
    this->pid = pid;
    this->creation_time = creation_time;
    this->burst_time = burst_time;
    this->remaining_time = burst_time;
    this->start_time = -1;
    this->end_time = -1;
    this->tickets = tickets;
    this->weights = tickets;
    this->is_finished = false;
    this->total_waiting_time = 0;
}

int Process::get_pid() const { return pid; }
int Process::get_end_time() const { return end_time; }
int Process::get_creation_time() const { return creation_time; }
int Process::get_total_waiting_time() const { return total_waiting_time; }

// Classe para ler o arquivo de entrada e armazenar os dados dos processos

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

private:
    std::string filename;
    std::string algorithm;
    int quantum;
    std::vector<int> ticket_values;
    std::vector<int> pids;
    std::vector<int> burst_times;
    std::vector<int> creation_times;
};

FileReader::FileReader(const std::string &filename)
{
    this->filename = filename;
    this->quantum = 0;
}

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

std::string FileReader::get_algorithm() const { return algorithm; }
int FileReader::get_quantum() const { return quantum; }
const std::vector<int> &FileReader::get_pids() const { return pids; }
const std::vector<int> &FileReader::get_creation_times() const { return creation_times; }
const std::vector<int> &FileReader::get_burst_times() const { return burst_times; }
const std::vector<int> &FileReader::get_ticket_values() const { return ticket_values; }


// definindo as cores dos nodos 
enum Color {Red, Black};

enum ProcessState {
    READY,
    RUNNING,
    TERMINATED
};

class Node {
    public:
        Process* process;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node(Process* p, Color c, Node* par = nullptr):
            process(p), color(c), parent(par), left(nullptr), right(nullptr){}
};

class Tree {
    public:
        Node* root;
        Node* NIL; // representar folhas nulas

        // Construtor
        Tree(); // Declaração do construtor

        // Destrutor
        ~Tree(); // Declaração do destrutor

        void rotate_left(Node* x);
        void rotate_right(Node* x);
        void fix_insert(Node* z);
        void transplant(Node* u, Node* v);
        Node* tree_minimum(Node* node);
        void fix_delete(Node* x);
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

        Tree::Tree() {
            // Inicializa o nó sentinela NIL
            NIL = new Node(nullptr, Black); // NIL é um nó preto sem Processo associado
            NIL->left = NIL;
            NIL->right = NIL;
            NIL->parent = NIL;
            root = NIL; // A árvore inicia vazia, com a raiz apontando para NIL
        }

        // Implementação do Destrutor da Tree
        Tree::~Tree() {
            delete_tree_nodes(root);
            delete NIL;
        }

        void Tree::delete_tree_nodes(Node* node) {
            if(node == NIL) {
                return;
            }

            delete_tree_nodes(node->left);
            delete_tree_nodes(node->right);
            delete node;
        }

        void Tree::rotate_left(Node* x){
            Node* y = x->right;
            x->right = y->left; 

            if(y->left != NIL){
                y->left->parent = x;
            }

            y->parent = x->parent;

            if(x->parent == NIL){
                root = y;
            }

            else if(x == x->parent->left){
                x->parent->left =y;
            }

            else{
                 x->parent->right = y;
            }

            y->left = x;
            x->parent = y;
        }

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

            else if(x == x->parent->right){
                x->parent->right = y;
            }

            else{
                x->parent->left = y;
            }

            y->right = x;
            x->parent = y;
        }

        void Tree::fix_insert(Node* z){
            while(z->parent->color == Red){
                Node* parent = z->parent;
                Node* grandparent = parent->parent;

                if(parent == grandparent->left){
                    Node* uncle = grandparent->right;

                    if(uncle->color == Red){
                        parent->color = Black;
                        uncle->color = Black;
                        grandparent->color = Red;
                        z = grandparent;
                }

                else{
                    if(z == parent->right){
                        z = parent;
                        rotate_left(z);
                        parent = z->parent; // Atualiza o pai após a rotação
                    }
                    parent->color = Black;
                    grandparent->color = Red;
                    rotate_right(grandparent);
                }
                else{
                    Node* uncle = grandparent->left;

                    if(uncle->color == Red){
                        parent->color = Black;
                        uncle->color = Black;
                        grandparent->color = Red;
                        z = grandparent;
                    }
                    else{
                        if (z == parent->left) {
                        z = parent;
                        rotate_right(z);
                        parent = z->parent; // Atualiza o pai após a rotação
                        }
                        parent->color = Black;
                        grandparent->color = Red;
                        rotate_left(grandparent);
                    }
            }
            root->color = Black;
        }

        void Tree::insert(Process* new_process_ptr){
            Node* new_node = new Node(new_process_ptr, Red);
            new_node->left = NIL;
            new_node->right = NIL;

            Node* parent = NIL;
            Node* current = root;

            while (current != NIL) {
                parent = current;
                if(new_process_ptr->vruntime < current->process->vruntime) {
                    current = current->left;
                } 
                else if(new_process_ptr->vruntime > current->process->vruntime) {
                    current = current->right;
                } 
                else {
                    // Desempate por PID se vruntime for igual
                    if (new_process_ptr->pid < current->process->pid) {
                        current = current->left;
                    } 
                    else if (new_process_ptr->pid > current->process->pid) {
                        current = current->right;
                    } 
                    else {
                        // Processo com mesmo vruntime e PID já existe, não insere.
                        // Em um sistema real, isso poderia ser uma atualização.
                        delete new_node; // Libera o nó que não será usado
                        return;
                    }
                }
            }

            new_node->parent = parent;

            if(parent == NIL) {
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
                if new_process_ptr->pid < parent->process->pid) {
                    parent->left = new_node;
                } 
                else {
                    parent->right = new_node;
                }
            }

            fix_insert(new_node);
        }

        Process* Tree::get_min_vruntime_process() const {
            if(root == NIL) {
                return nullptr;
            }
            Node* current = root;
            while(current->left != NIL) {
                current = current->left;
            }
            return current->process;
        }

        Node* Tree::search_node_by_pid(int pid) const {
            std::vector<Node*> queue_nodes;
            if (root != NIL) {
                queue_nodes.push_back(root);
            }
            int head = 0;
            while (head < queue_nodes.size()) {
                Node* node = queue_nodes[head++];
                if (node->process->pid == pid) {
                    return node;
                }
                if (node->left != NIL) queue_nodes.push_back(node->left);
                if (node->right != NIL) queue_nodes.push_back(node->right);
            }
            return nullptr;
        }

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
            if(v != NIL) {
                v->parent = u->parent;
            } 
        }

        void Tree::fix_delete(Node* x) {
            // A correção da árvore após a remoção de um nó preto (`original_y_color == Black`)
            // é necessária para manter as propriedades da Árvore Rubro-Negra.
            // O nó `x` é o que "substituiu" o nó removido ou movido, e pode ser um nó de "duplo-preto" virtual.

            // O loop continua enquanto `x` não for a raiz e sua cor for preta.
            // Isso indica uma violação da propriedade 2 (raiz é preta) ou 4 (caminho com nós pretos).
            while (x != root && x->color == Black) {
                if (x == x->parent->left) { // Caso 1: `x` é filho esquerdo de seu pai
                    Node* w = x->parent->right; // `w` é o irmão de `x`

                    // Caso 1: Irmão `w` é Vermelho.
                    // Recolore w e o pai, e rotaciona à esquerda no pai.
                    if (w->color == Red) {
                        w->color = Black;
                        x->parent->color = Red;
                        rotate_left(x->parent);
                        w = x->parent->right; // Atualiza w para o novo irmão
                    }

                    // Agora, o irmão `w` é Preto (ou se tornou preto devido ao Caso 1).
                    // Precisamos considerar os filhos de `w`.

                    // Caso 2: Irmão `w` é Preto, e ambos os filhos de `w` são Pretos.
                    // Recolore `w` para Vermelho, e move `x` para cima (para o pai).
                    if (w->left->color == Black && w->right->color == Black) {
                        w->color = Red;
                        x = x->parent; // Move a "dupla-preta" para cima
                    } 
                    // Caso 3: Irmão `w` é Preto, filho esquerdo de `w` é Vermelho, filho direito de `w` é Preto.
                    // Recolore filho esquerdo de `w` e `w`, rotaciona à direita em `w`.
                    else { 
                        if (w->right->color == Black) { // Se o filho direito de w é preto
                            w->left->color = Black;
                            w->color = Red;
                            rotate_right(w);
                            w = x->parent->right; // Atualiza w para o novo irmão
                        }
                        // Caso 4: Irmão `w` é Preto, e filho direito de `w` é Vermelho.
                        // Recolore `w` e o pai, recolore filho direito de `w` para preto, rotaciona à esquerda no pai.
                        w->color = x->parent->color;
                        x->parent->color = Black;
                        w->right->color = Black;
                        rotate_left(x->parent);
                        x = root; // A árvore está balanceada, termina o loop
                    }
                } else { // Caso simétrico: `x` é filho direito de seu pai
                    Node* w = x->parent->left; // `w` é o irmão de `x`

                    // Caso 1 (simétrico): Irmão `w` é Vermelho.
                    if (w->color == Red) {
                        w->color = Black;
                        x->parent->color = Red;
                        rotate_right(x->parent);
                        w = x->parent->left; // Atualiza w para o novo irmão
                    }

                    // Agora, o irmão `w` é Preto (ou se tornou preto devido ao Caso 1).

                    // Caso 2 (simétrico): Irmão `w` é Preto, e ambos os filhos de `w` são Pretos.
                    if (w->right->color == Black && w->left->color == Black) {
                        w->color = Red;
                        x = x->parent; // Move a "dupla-preta" para cima
                    } 
                    // Caso 3 (simétrico): Irmão `w` é Preto, filho direito de `w` é Vermelho, filho esquerdo de `w` é Preto.
                    else {
                        if (w->left->color == Black) { // Se o filho esquerdo de w é preto
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
            // A raiz deve ser sempre preta
            if (x != NIL) { // Garante que NIL não seja pintado de preto (embora NIL sempre seja Black)
                x->color = Black;
            }
        }

        void Tree::remove_process(int pid) {
            Node* z = search_node_by_pid(pid);
            if(z == nullptr) {
                std::cout << "Processo com PID " << pid << " não encontrado para remoção." << std::endl;
                return;
            }

            Node* y = z;
            Node* x = NIL; // Inicializado com NIL para consistência
            Color original_y_color = y->color;

            if(z->left == NIL) {
                x = z->right;
                transplant(z, z->right);
            } 
            else if(z->right == NIL) {
                x = z->left;
                transplant(z, z->left);
            } 
            else {
                y = tree_minimum(z->right);
                original_y_color = y->color;
                x = y->right;

                if (y->parent == z) {
                    // Se y é filho direto de z, x já tem o pai correto (y).
                    // Apenas certifique-se que x não é NIL antes de tentar atribuir parent.
                    if (x != NIL) x->parent = y; // Garante que x aponte para y
                } 
                else {
                    transplant(y, y->right);
                    y->right = z->right;
                    y->right->parent = y;
                }
                transplant(z, y);
                y->left = z->left;
                y->left->parent = y;
                y->color = z->color;
            }

            // Não deletamos o Processo* aqui, apenas o Node*
            delete z; 

            if (original_y_color == Black) {
                fix_delete(x);
            }
            
        }
        
        void Tree::in_order() const {
                in_order_traverse(root);
            }

        void Tree::in_order_traverse(Node* node) const {
            if (node != NIL) {
                in_order_traverse(node->left);
                std::cout << "(PID:" << node->process->pid << ", vR:" << node->process->vruntime
                        << (node->color == Red ? "R) " : "B) ");
                in_order_traverse(node->right);
            }
}

class Scheduler {
    public:
        Tree run_queue;
        Process* current_process;

        // Lista de todos os processos conhecidos pelo scheduler (para gerenciar vida útil)
        std::vector<Process*> all_processes; 

        int system_vruntime_min;
        int total_active_weight; 
        // verificar modificação dps
        int TARGET_LATENCY;
        int MIN_GRANULARITY;

        std::SCHED_HZ; // adptar para o metodo de geral depois 
        
        Scheduler() : current_process(nullptr), system_vruntime_min(0), total_active_weight(0), SCHED_HZ(0) {
        // A run_queue já é inicializada pelo seu próprio construtor.
        }

        ~Scheduler() {
            // Libera a memória de todos os objetos Processo alocados dinamicamente
            for (Process* p : all_processes) {
                delete p;
            }
            all_processes.clear();
        }

        void init_scheduler(init quantum_from_file) {
           // Limpa processos antigos se init for chamado múltiplas vezes
            for (Process* p : all_processes) {
                delete p; 
            }

            all_process* p : all_processes.clear();

            // Re-inicializa a run_queue criando uma nova instância (chama o destrutor da antiga)
            run_queue = RedBlackTree(); 

            current_process = nullptr;
            system_vruntime_min = 0;
            total_active_weight = 0;
            SCHED_HZ = quantum_from_file; // Usando o quantum do arquivo como base para SCHED_HZ

            std::cout << "Scheduler inicializado com Quantum/SCHED_HZ: " << SCHED_HZ << "ms." << std::endl;
    

        }

        void add_process(init pid, int creation_time, int burst_time, int tickets) {
            Process* new_process = new Process(pid, creation_time, burst_time, tickets);
            all_processes.push_back(new_process); // Adiciona à lista de todos os processos 

            // Para a simulação, vamos inicializar o vruntime de todos os processos ao adicionar.
            // Em um sistema real, o vruntime de um novo processo (forked) é o do pai,
            // ou o min_vruntime do sistema se for um processo novo.
            if (!run_queue.is_empty()) {
                new_process->vruntime = run_queue.get_min_vruntime_process()->vruntime;
            } 
            else {
                new_process->vruntime = system_vruntime_min;
            }
            
            run_queue.insert(new_process); // Insere o ponteiro na árvore RB
            total_active_weight += new_process->weights; // Atualiza o peso total ativo

            std::cout << "Processo " << new_process->pid << " (peso: " << new_process->weights << ") adicionado ao scheduler. vruntime inicial: " << new_process->vruntime << std::endl;
        }

        Process* pick_next_task() {
            if (run_queue.is_empty()) {
                current_process = nullptr;
                return nullptr; // Nenhuma tarefa pronta
            }

            Process* next_task_ptr = run_queue.get_min_vruntime_process(); // Obtém o ponteiro para o processo com menor vruntime
            
            // Remove o nó correspondente a este processo da run_queue.
            run_queue.remove_process(next_task_ptr->pid); // Remove o nó da árvore RB
            
            current_process = next_task_ptr;
            current_process->state = RUNNING;
            return current_process;
        }

        void schedule_tick(long long delta_time) {
        // 1. Se não há processo rodando, escolha o próximo
        if (current_process == nullptr) {
            current_process = pick_next_task();
            if (current_process == nullptr) {
                // Nenhum processo pronto para executar, sistema ocioso.
                system_vruntime_min += delta_time; // Avança o vruntime mínimo do sistema
                return;
            }
            std::cout << "--> Iniciando processo PID: " << current_process->pid << " (vR: " << current_process->vruntime << ")" << std::endl;
            if (current_process->start_time == -1) {
                current_process->start_time = current_time_global; // Registrar tempo de início real
            }
        }

        // 2. Atualiza o vruntime do processo atual
        // vruntime_increment = delta_time * (NICE_0_WEIGHT_APPROX / current_process->weights)
        long double vruntime_increment = static_cast<long double>(delta_time) * (NICE_0_WEIGHT_APPROX / current_process->weights);
        current_process->vruntime += vruntime_increment;
        current_process->remaining_time -= delta_time;

        // 3. Atualiza o vruntime mínimo do sistema
        // O system_vruntime_min deve ser o vruntime do nó mais à esquerda da árvore RB.
        // É a referência para o tempo "ideal" que o processo mais justo deveria ter.
        if (!run_queue.is_empty()) {
            // Compara o vruntime do processo atual com o menor da fila
            system_vruntime_min = std::min(current_process->vruntime, run_queue.get_min_vruntime_process()->vruntime);
        } else {
            // Se a fila está vazia, o processo atual é a única referência
            system_vruntime_min = current_process->vruntime; 
        }

        // 4. Verifica o término do processo
        if (current_process->remaining_time <= 0) {
            current_process->state = TERMINATED;
            total_active_weight -= current_process->weights; // Remove o peso do processo terminado
            current_process->end_time = current_time_global + delta_time; // Registrar tempo de término real
            std::cout << "<-- Processo PID: " << current_process->pid << " TERMINOU." << std::endl;
            
            current_process = nullptr; // CPU livre
            current_process = pick_next_task(); // Escolhe o próximo imediatamente
            if (current_process) {
                std::cout << "--> Iniciando processo PID: " << current_process->pid << " (vR: " << current_process->vruntime << ")" << std::endl;
                if (current_process->start_time == -1) {
                    current_process->start_time = current_time_global + delta_time;
                }
            }
            return; // Retorna para o próximo tick, um novo processo já está executando ou CPU ociosa
        }

        // 5. Verifica a preempção (context switch)
        // Se há outros processos na fila E o vruntime do processo atual é maior que o do processo mais justo na fila.
        if (!run_queue.is_empty()) {
            Process* next_in_queue = run_queue.get_min_vruntime_process();
            // A preempção ocorre se o processo atual está "atrasando" outros processos
            // A condição é que o vruntime do processo atual seja maior que o do próximo na fila
            // mais uma pequena margem (MIN_GRANULARITY para evitar oscilações excessivas).
            if (current_process->vruntime > next_in_queue->vruntime + MIN_GRANULARITY) {
                
                // Reinserir o processo atual na run_queue (ele será ordenado pelo novo vruntime)
                run_queue.insert(current_process); 
                current_process->state = READY; // Volta para estado de pronto
                std::cout << "<-- Processo PID: " << current_process->pid << " preempção (vR: " << current_process->vruntime << " vs. " << next_in_queue->vruntime << "). " << std::endl;
                
                // Escolhe o próximo processo
                current_process = pick_next_task();
                if (current_process) {
                    std::cout << "--> Iniciando processo PID: " << current_process->pid << " (vR: " << current_process->vruntime << ")" << std::endl;
                    if (current_process->start_time == -1) {
                        current_process->start_time = current_time_global + delta_time;
                    }
                }
            }
        }
    }

    // calculate_time_slice não é estritamente necessário para o CFS em si,
    // pois a preempção é baseada no vruntime, mas mantido para referência.
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

        std::cout << "\nFila de Processos Prontos (Run Queue - RB Tree In-Order):" << std::endl;
        run_queue.in_order(); // Chama o método in_order da RB Tree
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
};

// Variável global para o tempo atual da simulação (para start_time/end_time)
long long current_time_global = 0; 
        


}

    
int main(){
    // Exemplo de uso: o usuário deve fornecer o caminho para o arquivo de entrada
    std::string input_filename = "entradaEscalonador.txt"; // Nome do arquivo de entrada padrão

    // Para simulação, você pode criar este arquivo manualmente ou usar o script Python 'geradorEntrada.py'
    // Exemplo de conteúdo para 'entradaEscalonador.txt':
    // CFS|10
    // 0|1|100|100   (creation_time|pid|burst_time|tickets)
    // 0|2|200|50
    // 0|3|50|200

    FileReader file_reader(input_filename);
    file_reader.read_file();

    if (file_reader.pids.empty()) {
        std::cerr << "Nenhum processo lido do arquivo. Saindo." << std::endl;
        return 1;
    }

    Scheduler scheduler;
    scheduler.init_scheduler(file_reader.quantum); // Passa o quantum lido para o scheduler

    // Adiciona os processos lidos do arquivo ao scheduler
    for (size_t i = 0; i < file_reader.pids.size(); ++i) {
        scheduler.add_process(
            file_reader.pids[i],
            file_reader.creation_times[i],
            file_reader.burst_times[i],
            file_reader.ticket_values[i]
        );
    }

    // O delta_time para a simulação é o quantum lido do arquivo
    long long delta_time = scheduler.SCHED_HZ; 

    // Simular por um tempo máximo ou até todos os processos terminarem
    long long max_simulation_time = 1000; // Por exemplo, simular por 1000ms

    std::cout << "\nIniciando simulação do CFS..." << std::endl;

    while (current_time_global < max_simulation_time) {
        std::cout << "\n==========================================" << std::endl;
        std::cout << "Tempo de Simulação Atual: " << current_time_global << "ms" << std::endl;

        scheduler.schedule_tick(delta_time);
        scheduler.print_scheduler_state();
        
        current_time_global += delta_time; // Atualiza o tempo global da simulação

        // Condição de parada: todos os processos terminaram
        bool all_terminated = true;
        for (const auto& p_ptr : scheduler.all_processes) {
            if (p_ptr->state != TERMINATED) {
                all_terminated = false;
                break;
            }
        }
        if (all_terminated) {
            std::cout << "Todos os processos terminaram!" << std::endl;
            break;
        }

        // Adiciona um pequeno atraso para facilitar a leitura na console (opcional)
        // std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
    }

    std::cout << "\nSimulação CFS finalizada." << std::endl;

    
    return 0;
}
