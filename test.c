#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct chunk {
    int start;
    int size;
    struct chunk* next;
} chunk;

chunk* head_chunk = NULL; // 메인 청크 (여기서 할당이 이루어짐)
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

    // 1. 테스트용 빈 공간 생성 (10, 30, 20 크기 순서로)
    int a = myalloc(10); // [0, 10)
    int b = myalloc(10); // [10, 20)
    int c = myalloc(30); // [20, 50)
    int d = myalloc(10); // [50, 60)
    int e = myalloc(20); // [60, 80)
    // 남은 공간: (start=80, size=20)

    printf("--- After allocs, Free list ---\n");
    print_freed_chunks(); // (80, 20) 하나만 있어야 함

    myfree(a, 10); // (0, 10) free
    myfree(c, 30); // (20, 30) free
    myfree(e, 20); // (60, 80) free

    // myfree가 올바르게 동작했다면, 빈 공간 리스트는 주소 순서대로:
    // (0, 10) -> (20, 30) -> (60, 20) -> (80, 20)
    printf("--- Free list state (10, 30, 20, 20) ---\n");
    print_freed_chunks();

    // --- 2. Best-Fit 테스트 (Exact-Fit 최적화 테스트) ---
    printf("--- Allocating 10 (Testing Exact-Fit optimization) ---\n");
    int f = myalloc(10); // (0, 10) 청크를 할당해야 함 (start=0)
    printf("Allocated at: %d\n", f);
    // 남은 리스트: (20, 30) -> (60, 20) -> (80, 20)
    print_freed_chunks();

    // --- 3. Best-Fit 테스트 (Best-Fit 탐색 테스트) ---
    printf("--- Allocating 15 (Testing Best-Fit search) ---\n");
    int g = myalloc(15); // (60, 20) 청크를 할당해야 함 (start=60, diff=5)
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
    
    //16바이트 단위 할당
    if (request_size % 16 != 0) {
        int var = (request_size / 16) + 1;
        request_size = 16 * var;
    }

    int start_addr = 0;

    if ((start_addr = search_best_fit(request_size)) != -1) {
        return start_addr;
    }

    ////best-fit 우선 탐색
    //if ((start_addr = search_exact_fit(request_size)) != -1) {
    //    return start_addr;
    //}
    ////first-fit 탐색
    //else if ((start_addr = search_first_fit(request_size)) != -1) {
    //    return start_addr;
    //}
    ////모든 탐색에서 공간을 못 찾음.
    else {
        printf("space not enough.\n\n");
        return -1;
    }
}

//정확히 맞는 청크 탐색
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
                    head_chunk = temp_chunk->next; //null이든 아니든 같음.
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
    //탐색 못 한 경우
    return -1;
}

//충분한 청크 탐색
int search_first_fit(int request_size) {

    chunk* temp_chunk = head_chunk;
    chunk* prev_chunk = NULL;

    while (temp_chunk != NULL) {
        
        if (temp_chunk->size >= request_size) {
            int start_addr = temp_chunk->start;
            temp_chunk->size -= request_size;
            temp_chunk->start += request_size; //앞 부분을 떼주니까 갱신해야 함.

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
    //탐색 못 한 경우
    return -1;
}

//가장 알맞은 청크 탐색
int search_best_fit(int request_size) {
    
    chunk* best_fit = NULL;
    int best = 1000000;
    
    chunk* temp_chunk = head_chunk;
    chunk* prev_chunk = NULL;

    //exact-fit 함수를 호출하기에는 반복이 많아짐.
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
    chunk* prev_chunk = NULL; // 지나가면서 병합할 청크가 있을 때 사용

    if (free_chunk == NULL) return;

    free_chunk->start = start_addr;
    free_chunk->size = ret_size;

    while (temp_chunk != NULL && temp_chunk->start < free_chunk->start) {
        prev_chunk = temp_chunk;
        temp_chunk = temp_chunk->next;
    } //free_chunk가 들어갈 위치에 멈춤. prev_chunk가 free_chunk 자리

    //temp_chunk 가 head_chunk인 경우
    if (temp_chunk == head_chunk && prev_chunk == NULL) {
        head_chunk = free_chunk;
        free_chunk->next = temp_chunk;
    }

    //그냥 끝에 도달한 경우 
    else if (temp_chunk == NULL && prev_chunk != NULL) {
        prev_chunk->next = free_chunk;
        free_chunk->next = temp_chunk; //NULL
    }

    else {
        prev_chunk->next = free_chunk;
        free_chunk->next = temp_chunk;
    }

    //? 순서 중요 (반대로 하면 논리적으로 안 맞음)
    //free 다음 청크 병합 수행
    if (temp_chunk != NULL && (free_chunk->start + free_chunk->size) == temp_chunk->start) {
        free_chunk->size += temp_chunk->size;
        free_chunk->next = temp_chunk->next;
        free(temp_chunk);
    }
    //free 이전 청크 병합 수행
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

//TODO: 뒤 할당(기존 앞) ["size 없이 free", "양방향 연결 리스트"]
//TODO: ["이전(다음: 필요할 경우) 주소, 사이즈 저장 공간 마련", "실제 데이터 저장 가능"]

//보류
void myfree_seq(int start_addr, int ret_size) {
    if (ret_size <= 0) {
        printf("Invalid argument.\n");
        return;
    }
    chunk* curr_chunk = head_chunk;
    chunk* free_chunk = malloc(sizeof(chunk));
    chunk* prev_chunk = NULL; // 지나가면서 병합할 청크가 있을 때 사용

    if (free_chunk == NULL) return;

    free_chunk->start = start_addr;
    free_chunk->size = ret_size;

    while (curr_chunk != NULL && curr_chunk->start < free_chunk->start) {
        prev_chunk = curr_chunk;
        curr_chunk = curr_chunk->next;
    } //free_chunk가 들어갈 위치에 멈춤. prev_chunk가 free_chunk 자리

    //curr이 head인 경우
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
        temp_chunk = prev_chunk->next; //다음 반복을 위해 갱신
    }
}

//할당된 청크 출력(할당 청크 head가 없는 경우)
void print_allocated_chunks(int total_size) {
    
    int count = 0;
    int curr_pos = 0;

    chunk* curr_chunk = head_chunk;

    if (curr_chunk == NULL) { //모든 공간이 사용됨.
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
    //count==0이면 1이 되니까
    if (!count) printf("There isn't allocated chunks.\n");
}

