#include <iostream>

class Process {
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
    }
};

// definindo as cores dos nodos 
enum Color {Red, Black};

class Node {
    public:
        Process* Process;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node(Process* p, Color c, Node* par = nullptr):
            process(p), color(c), parent(par), left(nullptr), right(nullptr){}
};

class Tree{
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

        Tree() {
            // Inicializa o nó sentinela NIL
            NIL = new Node(nullptr, Black); // NIL é um nó preto sem Processo associado
            NIL->left = NIL;
            NIL->right = NIL;
            NIL->parent = NIL;
            root = NIL; // A árvore inicia vazia, com a raiz apontando para NIL
        }

        void delete_tree_nodes(Node* node) {
            if(node == NIL) {
                return;
            }

            delete_tree_nodes(node->left);
            delete_tree_nodes(node->right);
            delete node;
        }

        void rotate_left(Node* x){
            Node* y = x->right;
            x->right = y->left; 

            if(y->left != NIL){
                y->left->parent = x;
            }

            y->parent = x->parent;

            if(x->parent == nullptr){
                root = y;
            }

            else if(x == x->parent->left){
                x->parent->left =y;
            }

            else{
                 x->parent->right = y;
            }

            x->left = x;
            x->parent = y;
        }

        void rotate_right(Node* x){
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

        void fix_insert(Node* z){
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

        void insert(Process* new_process_ptr){
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
            return current->procress;
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

        void transplant(Node* u, Node* v) {
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

        void RedBlackTree::fix_delete(Node* x) {
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

        void remove_process(int pid) {
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
        
        void in_order() const {
                in_order_traverse(root);
            }

        void in_order_traverse(Node* node) const {
            if (node != NIL) {
                in_order_traverse(node->left);
                std::cout << "(PID:" << node->process->pid << ", vR:" << node->process->vruntime
                        << (node->color == Red ? "R) " : "B) ");
                in_order_traverse(node->right);
            }
}

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

enum ProcessState {
    READY,
    RUNING,
    TERMINATED
}

class Scheduler {
    public:
        Tree red_black_tree;
        Process* current_process;
        int system_vruntime_min;
        int total_active_weight; 
        // verificar modificação dps
        int TARGET_LATENCY;
        int MIN_GRANULARITY;
        std::SCHED_HZ; // lógica Fake dos ciclos da CPU

        Scheduler(){
            current_process = nullptr;
            system_vruntime_min = 0;
            total_active_weight = 0;

            TARGET_LATENCY = 0;
            MIN_GRANULARITY = 0;

        }

        void add_process


}
int main(){

    return 0;
}
