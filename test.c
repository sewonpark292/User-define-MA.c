#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct chunk {
    int start;
    int size;
    struct chunk* next;
} chunk;

chunk* head_chunk = NULL; // ���� ûũ (���⼭ �Ҵ��� �̷����)
chunk* allocated_chunk_head = NULL;

int myalloc(int request_size);
void myfree(int start_addr, int ret_size);
void print_freed_chunks();
void myfree_seq(int start_addr, int ret_size);
int search_exact_fit(int request_size);
int search_first_fit(int request_size);
int search_best_fit(int request_size);
void print_allocated_chunks(int total_size);

chunk* create_my_chunk(int chunk_size) {
    chunk* head_chunk = malloc(sizeof(chunk));
    if (head_chunk == NULL) return NULL;

    head_chunk->start = 0;
    head_chunk->size = chunk_size;
    head_chunk->next = NULL;

    return head_chunk;
}

int main(void) {

    head_chunk = create_my_chunk(100);
    printf("--- Initial State (size=100) ---\n");
    print_freed_chunks();

    // 1. �׽�Ʈ�� �� ���� ���� (10, 30, 20 ũ�� ������)
    int a = myalloc(10); // [0, 10)
    int b = myalloc(10); // [10, 20)
    int c = myalloc(30); // [20, 50)
    int d = myalloc(10); // [50, 60)
    int e = myalloc(20); // [60, 80)
    // ���� ����: (start=80, size=20)

    printf("--- After allocs, Free list ---\n");
    print_freed_chunks(); // (80, 20) �ϳ��� �־�� ��

    myfree(a, 10); // (0, 10) free
    myfree(c, 30); // (20, 30) free
    myfree(e, 20); // (60, 80) free

    // myfree�� �ùٸ��� �����ߴٸ�, �� ���� ����Ʈ�� �ּ� �������:
    // (0, 10) -> (20, 30) -> (60, 20) -> (80, 20)
    printf("--- Free list state (10, 30, 20, 20) ---\n");
    print_freed_chunks();

    // --- 2. Best-Fit �׽�Ʈ (Exact-Fit ����ȭ �׽�Ʈ) ---
    printf("--- Allocating 10 (Testing Exact-Fit optimization) ---\n");
    int f = myalloc(10); // (0, 10) ûũ�� �Ҵ��ؾ� �� (start=0)
    printf("Allocated at: %d\n", f);
    // ���� ����Ʈ: (20, 30) -> (60, 20) -> (80, 20)
    print_freed_chunks();

    // --- 3. Best-Fit �׽�Ʈ (Best-Fit Ž�� �׽�Ʈ) ---
    printf("--- Allocating 15 (Testing Best-Fit search) ---\n");
    int g = myalloc(15); // (60, 20) ûũ�� �Ҵ��ؾ� �� (start=60, diff=5)
    printf("Allocated at: %d\n", g);
    // (20, 30) -> (60, 5) -> (80, 20)
    print_freed_chunks();

    printf("--- Allocated chunks ---\n");
    print_allocated_chunks(100);

    return 0;
}

int myalloc(int request_size) {

    if (request_size <= 0) {
        printf("Invalid argument.\n");
        return -1;
    }
    
    //16����Ʈ ���� �Ҵ�
    if (request_size % 16 != 0) {
        int var = (request_size / 16) + 1;
        request_size = 16 * var;
    }

    int start_addr = 0;

    if ((start_addr = search_best_fit(request_size)) != -1) {
        return start_addr;
    }

    ////best-fit �켱 Ž��
    //if ((start_addr = search_exact_fit(request_size)) != -1) {
    //    return start_addr;
    //}
    ////first-fit Ž��
    //else if ((start_addr = search_first_fit(request_size)) != -1) {
    //    return start_addr;
    //}
    ////��� Ž������ ������ �� ã��.
    else {
        printf("space not enough.\n\n");
        return -1;
    }
}

//��Ȯ�� �´� ûũ Ž��
int search_exact_fit(int request_size) {
 
    chunk* temp_chunk = head_chunk;
    chunk* prev_chunk = NULL;

    while (temp_chunk != NULL) {

        if (request_size == temp_chunk->size) {
            int start_addr = temp_chunk->start;
            temp_chunk->size -= request_size;
            temp_chunk->start += request_size;

            if (temp_chunk->size == 0) {
                if (temp_chunk == head_chunk) {
                    head_chunk = temp_chunk->next; //null�̵� �ƴϵ� ����.
                    free(temp_chunk);
                }
                else {
                    prev_chunk->next = temp_chunk->next;
                    free(temp_chunk);
                }
            }

            return start_addr;
        }

        prev_chunk = temp_chunk;
        temp_chunk = temp_chunk->next;
    }
    //Ž�� �� �� ���
    return -1;
}

//����� ûũ Ž��
int search_first_fit(int request_size) {

    chunk* temp_chunk = head_chunk;
    chunk* prev_chunk = NULL;

    while (temp_chunk != NULL) {
        
        if (temp_chunk->size >= request_size) {
            int start_addr = temp_chunk->start;
            temp_chunk->size -= request_size;
            temp_chunk->start += request_size; //�� �κ��� ���ִϱ� �����ؾ� ��.

            if (temp_chunk->size == 0) {
                if (temp_chunk == head_chunk) {
                    head_chunk = temp_chunk->next;
                    free(temp_chunk);
                }
                else {
                    prev_chunk->next = temp_chunk->next;
                    free(temp_chunk);
                }
            }

            return start_addr;
        }
        else {
            prev_chunk = temp_chunk;
            temp_chunk = temp_chunk->next;
        }
    }
    //Ž�� �� �� ���
    return -1;
}

//���� �˸��� ûũ Ž��
int search_best_fit(int request_size) {
    
    chunk* best_fit = NULL;
    int best = 1000000;
    
    chunk* temp_chunk = head_chunk;
    chunk* prev_chunk = NULL;

    //exact-fit �Լ��� ȣ���ϱ⿡�� �ݺ��� ������.
    while (temp_chunk != NULL) {

        int diff = temp_chunk->size - request_size;

        if (diff >= 0) {
            int start_addr = temp_chunk->start;

            if (diff == 0) {
                temp_chunk->size -= request_size;
                temp_chunk->start += request_size;
                if (temp_chunk == head_chunk) {
                    head_chunk = temp_chunk->next;
                    free(temp_chunk);
                }
                else {
                    prev_chunk->next = temp_chunk->next;
                    free(temp_chunk);
                }
                return start_addr;
            }
            else if (diff > 0) {
                
                if (best > diff) {
                    best = diff;
                    best_fit = temp_chunk;
                }
            }
        }

        prev_chunk = temp_chunk;
        temp_chunk = temp_chunk->next;
    }

    if (best_fit != NULL) {
        int start_addr = best_fit->start;
        best_fit->start += request_size;
        best_fit->size -= request_size;
        return start_addr;
    }
    else {
        printf("space not enough.\n");
        return -1;
    }
}

void myfree(int start_addr, int ret_size) {
    if (ret_size <= 0) {
        printf("Invalid argument.\n");
        return;
    }
    chunk* temp_chunk = head_chunk;
    chunk* free_chunk = malloc(sizeof(chunk));
    chunk* prev_chunk = NULL; // �������鼭 ������ ûũ�� ���� �� ���

    if (free_chunk == NULL) return;

    free_chunk->start = start_addr;
    free_chunk->size = ret_size;

    while (temp_chunk != NULL && temp_chunk->start < free_chunk->start) {
        prev_chunk = temp_chunk;
        temp_chunk = temp_chunk->next;
    } //free_chunk�� �� ��ġ�� ����. prev_chunk�� free_chunk �ڸ�

    //temp_chunk �� head_chunk�� ���
    if (temp_chunk == head_chunk && prev_chunk == NULL) {
        head_chunk = free_chunk;
        free_chunk->next = temp_chunk;
    }

    //�׳� ���� ������ ��� 
    else if (temp_chunk == NULL && prev_chunk != NULL) {
        prev_chunk->next = free_chunk;
        free_chunk->next = temp_chunk; //NULL
    }

    else {
        prev_chunk->next = free_chunk;
        free_chunk->next = temp_chunk;
    }

    //? ���� �߿� (�ݴ�� �ϸ� �������� �� ����)
    //free ���� ûũ ���� ����
    if (temp_chunk != NULL && (free_chunk->start + free_chunk->size) == temp_chunk->start) {
        free_chunk->size += temp_chunk->size;
        free_chunk->next = temp_chunk->next;
        free(temp_chunk);
    }
    //free ���� ûũ ���� ����
    if (prev_chunk != NULL && (prev_chunk->start + prev_chunk->size) == free_chunk->start) {
        prev_chunk->size += free_chunk->size;
        prev_chunk->next = free_chunk->next;
        free(free_chunk);
    }
}

void print_freed_chunks() {
    chunk* temp_chunk = head_chunk;
    int i = 0;

    while (temp_chunk != NULL) {
        printf("%d chunk: (start: %d, size: %d)\n", i++, temp_chunk->start, temp_chunk->size);
        temp_chunk = temp_chunk->next;
    }
    printf("\n");
}

//TODO: �� �Ҵ�(���� ��) ["size ���� free", "����� ���� ����Ʈ"]
//TODO: ["����(����: �ʿ��� ���) �ּ�, ������ ���� ���� ����", "���� ������ ���� ����"]

//����
void myfree_seq(int start_addr, int ret_size) {
    if (ret_size <= 0) {
        printf("Invalid argument.\n");
        return;
    }
    chunk* curr_chunk = head_chunk;
    chunk* free_chunk = malloc(sizeof(chunk));
    chunk* prev_chunk = NULL; // �������鼭 ������ ûũ�� ���� �� ���

    if (free_chunk == NULL) return;

    free_chunk->start = start_addr;
    free_chunk->size = ret_size;

    while (curr_chunk != NULL && curr_chunk->start < free_chunk->start) {
        prev_chunk = curr_chunk;
        curr_chunk = curr_chunk->next;
    } //free_chunk�� �� ��ġ�� ����. prev_chunk�� free_chunk �ڸ�

    //curr�� head�� ���
    if (prev_chunk == NULL && curr_chunk == head_chunk) {
        head_chunk = free_chunk;
        free_chunk->next = curr_chunk;
        prev_chunk = free_chunk;
    }
    else {
        prev_chunk->next = free_chunk;
        free_chunk->next = curr_chunk;
    }

    //sequential merge logic.
    chunk* temp_chunk = free_chunk;

    while (temp_chunk != NULL && (prev_chunk->start + prev_chunk->size) == temp_chunk->start) {
        prev_chunk->size += temp_chunk->size;
        prev_chunk->next = temp_chunk->next;
        free(temp_chunk);
        temp_chunk = prev_chunk->next; //���� �ݺ��� ���� ����
    }
}

//�Ҵ�� ûũ ���(�Ҵ� ûũ head�� ���� ���)
void print_allocated_chunks(int total_size) {
    
    int count = 0;
    int curr_pos = 0;

    chunk* curr_chunk = head_chunk;

    if (curr_chunk == NULL) { //��� ������ ����.
        printf("%d used: 0 ~ %d \n", count, total_size);
        return;
    }
    
    while (curr_chunk != NULL) {
        
        if (curr_pos < curr_chunk->start) {
            printf("%d used: %d ~ %d\n", count, curr_pos, curr_chunk->start);
        }
        count++;
        curr_pos = curr_chunk->start + curr_chunk->size;
        curr_chunk = curr_chunk->next;
    }

    if (curr_pos < total_size) {
        printf("%d used: %d ~ %d", count + 1, curr_pos, total_size);
    }
    //count==0�̸� 1�� �Ǵϱ�
    if (!count) printf("There isn't allocated chunks.\n");
}

