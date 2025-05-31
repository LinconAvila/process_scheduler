#include <iostream>

// definindo as cores dos nodos 
enum Color {Red, Black};

class Node{
    public:
    int key;
        Color color;
        Node* left;
        Node* right;
        Node* parent;

        Node(Color color, Node* parent = nullptr);

};

class Tree{
    public:
        Node* root;

        void rotate_left(){
            Node* y = x->right;
            x->right = y->left; 

            if(y->left != nullptr){
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

        void rotate_right(){
            Node* y = x->left;
            x->left = y->right;

            if(y->right != nullptr){
                y->right->parent = x;
            }

            y->parent = x->parent;

            if(x->parent == nullptr){
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

        void fix_insert(){
            while(node != root && node->parent->color == Red){
                Node* grandparent = node->parent->parent;
                Node* uncle  = nullptr;

                if(node->parent == grandparent->left){
                    uncle = grandparent->right;

                    if(uncle && uncle->color == Red){
                        node->parent->color = Black;
                        uncle->color = Black;
                        grandparent->color = Red;
                        node = grandparent;
                }

                else{
                    if(node == node->parent->right){
                        node = node->parent;
                        rotate_left(node);
                    }
                    node->parent->color = Black
                    grandparent->color = Red;
                    rotate_right(grandparent);
                }
                else{
                    uncle = grandparent->left;

                    if(uncle && uncle->color == Red){
                        node->parent->color = Black;
                        uncle->color = Black;
                        grandparent->color = Red;
                        node = grandparent;
                    }
                    else{
                        if(node == node->parent->left){
                            node = node->parent;
                            rotate_right(node);
                        }
                        node->parent->color = Black;
                        grandparent->color = Red;
                        rotate_right(grandparent);
                    }
            }
            root->color = Black;
        }

        void insert(){
            Node* new_node = new Node(key);
            Node* parent = nullptr;
            Node* current = root;

            while(current !=nullptr){
                parent = current;
                if(key < current->key){
                    current = current->left
                }
                else{
                    current =  current->right
                }
            }

            new_node->parent = parent;

            if(parent == nullptr){
                root = new_node;
            }
            else if(key < parent->key){
                parent->left = new_node;
            }
            else{
                parent->right = new_node
            }

            new_node->color = Red;
            fix_insert(new_node);
        }
        void in_order(){
            if(node != nullptr){
                inOrder(node->left);
                std::cout << node->key << (node->color == Red ? "R " : "B ");
                inOrder(node->right); 
            }
        }

        void display();


}

int main(){

    return 0;
}
