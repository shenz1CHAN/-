#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TOTAL_SIZE 1024
#define MAX_LEVEL 10
#define N 8


// 定义内存块结构体
typedef struct Block {
    int start;  // 块起始地址
    int end; // 块结束地址
    int level; // 块所在的层级
    int isAllocated; // 标识块是否已分配
    int isSplited; // 标识块是否被拆分
    struct Block* Buddy; // 伙伴指针
} Block;

// 定义链表节点
typedef struct Node {
    Block* block;
    struct Node* next;
} Node;

// 创建内存块
Block* createBlock(int start, int end, int level) {
    Block* block = (Block*)malloc(sizeof(Block));
    block->start = start;
    block->end = end;
    block->level = level;
    block->isAllocated = 0;
    block->isSplited = 0;
    block->Buddy = NULL;
    return block;
}

// 创建链表节点
Node* createNode(Block* block, Node* list[]) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("内存分配失败\n");
        return NULL;
    }
    newNode->block = block;
    newNode->next = NULL;

    Node* lastNode = list[block->level];
    while (lastNode->next != NULL) {
        lastNode = lastNode->next;
    }
    lastNode->next = newNode;
    return newNode;
}

// 删除链表节点
int deleteNode(Block* block, Node* list[]) {
    Node* nodeDelete = list[block->level];
    Node* node = NULL;
    while (nodeDelete->next) {
        if (nodeDelete->next->block == block) {
            node = nodeDelete->next->next;
            free(nodeDelete->next);
            nodeDelete->next = node;
            return 1;
        }
        nodeDelete = nodeDelete->next;
    }
    return 0;
}
// 分裂内存块
void splitBlock(Node* node, Node* list[]) {
    int mid = (node->block->start + node->block->end) / 2;
    int newLevel = node->block->level - 1;

    
    Block* blockLeft = createBlock(node->block->start, mid, newLevel);
    Block* blockRight = createBlock(mid + 1, node->block->end, newLevel);
    blockLeft->Buddy = blockRight;
    blockRight->Buddy = blockLeft;
    createNode(blockLeft, list);
    createNode(blockRight, list);
    node->block->isSplited = 1;

    printf("对地址为[%d,%d]的分区块进行划分，新分区块地址为[%d,%d],[%d,%d]。\n", node->block->start, node->block->end, blockLeft->start, blockLeft->end, blockRight->start, blockRight->end);

}


// 分配内存
int allocateMemory(Node* list[], Block* allocate[], int num, int size) {
    int requiredLevel = 0;
    int tempSize = size;
    printf("进程%d要求大小为%d的内存空间，", num, size);
    // 计算所需的层级
    while (tempSize > 1) {
        tempSize /= 2;
        requiredLevel++;
    }
    printf("准备级别为%d的内存块供其使用。\n", requiredLevel);
    int level;
    Node* node;
    bool splitSign = true;
    while (splitSign) {
        level = requiredLevel;
        node = list[level];
        // 在对应层级查找内存块
        while (node->next) {
            node = node->next;
            // 找到符合要求的内存块
            if (!node->block->isAllocated && !node->block->isSplited)
            {
                node->block->isAllocated = 1;
                allocate[num - 1] = node->block;
                printf("将地址为[%d,%d]的级别%d分区块供进程%d使用。\n", node->block->start, node->block->end, node->block->level, num);
                return NULL;
            }
        }

        splitSign = false;
        level++;
        while (level <= MAX_LEVEL) {
            node = list[level];
            while (node->next) {
                node = node->next;
                if (!node->block->isAllocated && !node->block->isSplited) {
                    splitBlock(node, list);
                    splitSign = true;
                    break;
                }
            }
            if (splitSign) {
                break;
            }
            level++;
        }
    }
    // 找不到符合要求的内存块
    printf("分配失败！");
    return NULL;
}

// 合并内存块
Block* mergeBlocks(Block* block, Node* list[]) {
    // 检查是否满足合并条件
    if (!block->isAllocated && !block->isSplited && !block->Buddy->isAllocated && !block->Buddy->isSplited) {
        int start = block->start < block->Buddy->start ? block->start : block->Buddy->start;
        int end = block->end > block->Buddy->end ? block->end : block->Buddy->end;
        int newlevel = block->level + 1;
        Node* node = list[newlevel];
        printf("将地址为[%d,%d]和[%d,%d]的内存块进行合并，新分区块的地址为[%d,%d]。\n", block->start, block->end, block->Buddy->start, block->Buddy->end, start, end);
        while (node->next) {
            node = node->next;
            if (node->block->start == start) {
                node->block->isSplited = 0;

                deleteNode(block, list);
                deleteNode(block->Buddy, list);
                free(block->Buddy);
                free(block);

                break;
            }
        }

        
        return node->block;
    }
    return NULL;
}

// 回收内存
void deallocateMemory(Block* block, Node* list[], int num) {
    block->isAllocated = 0;
    printf("进程%d结束，返还地址为[%d,%d]级别为%d的内存块。\n", num, block->start, block->end, block->level);
    int level = block->level;

    Block* current = block;
    while (current = mergeBlocks(current, list)) {
        if (current->level == MAX_LEVEL) {
            break;
        }
    }

}

// 打印内存块状态
void printBlockStatus(Node* list[]) {
    printf("分区状态: \n");
    for (int i = MAX_LEVEL; i > 0; i--) {
        Node* node = list[i];
        while (node->next) {
            node = node->next;
            if (node->block->isSplited) {
                continue;
            }
            else if(node->block->isAllocated){
                printf("内存块[%d,%d] 级别%d 占用\n", node->block->start, node->block->end, i);
            }
            else {
                printf("内存块[%d,%d] 级别%d 空闲\n", node->block->start, node->block->end, i);
            }
        }
    
    }
    printf("\n");
}

// 主函数
int main() {

    Node* list[MAX_LEVEL + 1] = {};     // 链表指针数组
    for (int i = 0; i < MAX_LEVEL + 1; i++) {
        list[i] = (Node*)malloc(sizeof(Node));
        list[i]->block = NULL;
        list[i]->next = NULL;
    }
    Block* allocate[N] = {};    // 存放占用的内存分区
    Block* root = createBlock(0, 1023, MAX_LEVEL);
    createNode(root, list);

    //问题一
    printf("开始分配内存...\n");
    allocateMemory(list, allocate, 1, 128);
    printBlockStatus(list);

    allocateMemory(list, allocate, 2, 16);
    printBlockStatus(list);

    allocateMemory(list, allocate, 3, 256);
    printBlockStatus(list);

    printf("开始回收内存...\n");
    deallocateMemory(allocate[0], list, 1);
    allocate[0] = NULL;
    printBlockStatus(list);

    deallocateMemory(allocate[1], list, 2);
    allocate[1] = NULL;
    printBlockStatus(list);

    deallocateMemory(allocate[2], list, 3);
    allocate[2] = NULL;
    printBlockStatus(list);

    //问题二
    printf("开始分配内存...\n");
    int requiredSize = 0;
    int exp = 0;
    srand((unsigned int)time(NULL));
    for (int i = 0; i < N; i++) {
        requiredSize = 1;
        exp = 3 + rand() % 6;
        for (int j = 0; j < exp; j++) {
            requiredSize *= 2;
        }
        allocateMemory(list, allocate, i + 1, requiredSize);
        printBlockStatus(list);
    }

    printf("开始回收内存...\n");
    for (int i = 0; i < N; i++) {
        deallocateMemory(allocate[i], list, i + 1);
        allocate[i] = NULL;
        printBlockStatus(list);
    }
    return 0;
}
