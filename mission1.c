// FF算法和NF算法的代码实现
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 定义内存块结构体
typedef struct Block {
    int start;  // 块起始地址
    int end; // 块结束地址
    int size; // 块大小
    int isAllocated; // 标识块是否已分配
} Block;

// 定义链表节点
typedef struct Node {
    Block* block;
    struct Node* next;
} Node;

// 创建内存块
Block* createBlock(int start, int end) {
    Block* block = (Block*)malloc(sizeof(Block));
    if (block == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    block->start = start;
    block->end = end;
    block->size = end - start + 1;
    block->isAllocated = 0;
    return block;
}

// 创建链表节点
Node* createNode(Block* block) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    newNode->block = block;
    newNode->next = NULL;
    return newNode;
}

//插入节点
void insertNode(Node* node, Node** head) {
    if (!*head) {
        *head = node;
        return;
    }
    Node* nodeSign = *head;
    Node* nodePrev = NULL;
    while (nodeSign && nodeSign->block->start < node->block->end) {
        nodePrev = nodeSign;
        nodeSign = nodeSign->next;
    }
    if (nodePrev == NULL) {
        node->next = *head;
        *head = node;
    }
    else {
        nodePrev->next = node;
        node->next = nodeSign;
    }
}

// 打印链表
void printList(Node* head) {
    Node* current = head;
    printf("分区状态：\n");
    while (current != NULL) {
        if (current->block->isAllocated) {
            printf("内存块[%d,%d] 大小%d 占用\n", current->block->start, current->block->end, current->block->size);
        }
        else {
            printf("内存块[%d,%d] 大小%d 空闲\n", current->block->start, current->block->end, current->block->size);
        }
        
        current = current->next;
    }
    printf("\n");
}

// 查找指定节点
Node* searchNode(Node* head, int start, int end) {
    Node* current = head;
    if (start != -1) {
        while (current != NULL) {
            if (current->block->start == start) {
                return current;
            }
            current = current->next;
        }
    }
    if (end != -1) {
        while (current != NULL) {
            if (current->block->end == end) {
                return current;
            }
            current = current->next;
        }
    }
    return NULL;
}

// 删除指定节点
void deleteNode(Node** head, int start) {
    Node* temp = *head;
    Node* prev = NULL;

    if (temp != NULL && temp->block->start == start) {
        *head = temp->next;
        free(temp);
        return;
    }

    while (temp != NULL && temp->block->start != start) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("未找到要删除的节点\n");
        return;
    }

    prev->next = temp->next;
    free(temp);
}


int randnum(int start, int end) {
    return start + rand() % (end - start + 1);
}

// 分配内存
void allocateMemory(Node** head, int allocated[], Node* node, int num, int size) {
    node->block->isAllocated = 1;
    
    int oldstart = node->block->start;
    int oldend = node->block->end;
    int newstart = randnum(oldstart, oldend - size + 1);
    int newend = newstart + size - 1;
    
    
    if (oldstart != newstart) {
        node->block->start = newstart;
        insertNode(createNode(createBlock(oldstart, newstart - 1)), head);
    }
    if (oldend != newend) {
        node->block->end = newend;
        insertNode(createNode(createBlock(newend + 1, oldend)), head);
    }
    node->block->size = newend - newstart + 1;
    allocated[num - 1] = newstart;
    printf("进程%d 大小%d 内存分配成功！ 地址为[%d,%d]\n", num, size, newstart, newend);
}

// 首次适应算法FF
void FF(Node** head, int allocated[], int num, int size) {
    Node* current = *head;
    while (current != NULL) {
        if (!current->block->isAllocated && size <= current->block->size) {
            allocateMemory(head, allocated, current, num, size);
            return;
        }
        current = current->next;
    }
    printf("进程%d 大小%d 内存分配失败！\n", num, size);
}

// 循环首次适应算法NF
void NF(Node** head, Node** pointer, int allocated[], int num, int size) {
    Node* current = *pointer;
    while (current != NULL) {
        if (!current->block->isAllocated && size <= current->block->size) {
            allocateMemory(head, allocated, current, num, size);
            *pointer = current->next;
            return;
        }
        current = current->next;
    }
    current = *head;
    while (current != *pointer) {
        if (!current->block->isAllocated && size <= current->block->size) {
            allocateMemory(head, allocated, current, num, size);
            *pointer = current->next;
            return;
        }
        current = current->next;
    }
    printf("进程%d 大小%d 内存分配失败！\n", num, size);
}

// 回收内存
void deallocateMemory(Node** head, int allocated[], int num) {
    int start = allocated[num - 1];
    allocated[num - 1] = -1;
    Node* recoverNode = searchNode(*head, start, -1);
    recoverNode->block->isAllocated = 0;
    printf("进程%d结束，返还地址为[%d,%d]的内存块。\n", num, recoverNode->block->start, recoverNode->block->end);
    Node* node = NULL;
    node = searchNode(*head, -1, start - 1);
    if (node && !node->block->isAllocated) {
        recoverNode->block->start = node->block->start;
        deleteNode(head, node->block->start);
    }
    node = searchNode(*head, recoverNode->block->end + 1, -1);
    if (node && !node->block->isAllocated) {
        recoverNode->block->end = node->block->end;
        deleteNode(head, node->block->start);
    }
    recoverNode->block->size = recoverNode->block->end - recoverNode->block->start + 1;
    
}


// 主函数
int main() {

    Node* head = NULL;     // 链表头结点
    int allocated[10] = {};     // 存储占用内存的起始地址
    for (int i = 0; i < 10; i++) {
        allocated[i] = -1;
    }
    Block* root = createBlock(0, 1023);
    insertNode(createNode(root), &head);
    printList(head);
    //FF算法分配
    printf("FF算法开始分配内存...\n");
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 10; i++) {
        FF(&head, allocated, i + 1, randnum(100, 200));
        printList(head);
    }


    printf("开始回收内存...\n");
    for (int i = 0; i < 10; i++) {
        if (allocated[i] == -1) {
            continue;
        }
        deallocateMemory(&head, allocated, i + 1);
        printList(head);
    }

    //NF算法分配
    Node* pointer = head; //指向上一次分配内存块的下一个内存块
    printf("NF算法开始分配内存...\n");
    for (int i = 0; i < 10; i++) {
        NF(&head, &pointer, allocated, i + 1, randnum(100, 200));
        printList(head);
    }

    printf("开始回收内存...\n");
    for (int i = 0; i < 10; i++) {
        if (allocated[i] == -1) {
            continue;
        }
        deallocateMemory(&head, allocated, i + 1);
        printList(head);
    }

    return 0;
}
