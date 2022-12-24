#include <iostream>

struct Node {
    int data;
    Node* next;

    Node(int data) {
        this->data = data;
        next = 0;
    }
};

struct Linked_list {
    Node* front = 0;

    void print() {
        Node* next = front;

        while(next != 0) {
            std::cout << next->data << " ";
            next = next->next;
        }
    }

    void push_front(int data) {
        Node* temp = new Node(data);
        temp->next = front;
        front = temp;
    }

    void push_front(int* data, int length) {
        for(int i = length - 1; i >= 0; --i) {
            Node* temp = new Node(data[i]);
            temp->next = front;
            front = temp;
        }
    }

    void insert(int key, int data) {
        Node* ptr = front;
        for(int i = 0; i < key - 1; i++) {
            ptr = ptr->next;
        }

        Node* temp = ptr->next;
        Node* _new = new Node(data);
        _new->next = temp;
        ptr->next = _new;
    }

    void reverse() {
        Node* a = front;
        Node* b = 0, * c = 0;
        
        while(a != 0) {
            b = a->next;
            a->next = c;

            c = a;
            a = b;
        }

        front = c;
    }

    void remove(int key) {
        Node* ptr = front;
        for(int i = 0; i < key - 1; i++) {
            ptr = ptr->next;
        }

        Node* temp = ptr->next->next;
        delete ptr->next;
        ptr->next = temp;
    }

    ~Linked_list() {
        Node* next = front;
        Node* temp;
        while(next != 0) {
            temp = next->next;
            delete next;
            next = temp;
        }
    }
};

int main() {
    Linked_list list;
    int list_contents[7] = {6, 7, 8, 9, 20, 60, 2};
    list.push_front(list_contents, 7);
    list.remove(6);
    list.insert(2, 16);

    std::cout << "list before reverse: ";
    list.print();
    std::cout << "\n";

    list.reverse();

    std::cout << "list after reverse: ";
    list.print();
    std::cout << "\n";

    return 0;
}