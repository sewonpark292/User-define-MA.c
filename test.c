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
int insert_allocated_chunk(int start_addr, int request_size);
chunk* search_chunk(int start_addr);
void myfree_addr(int start_addr);
void print_allocated_chunks_with_head(int total_size);
int free_from_alloc(chunk* be_freed);

chunk* create_my_chunk(int chunk_size) {
    chunk* head_chunk = malloc(sizeof(chunk));
    if (head_chunk == NULL) return NULL;

    head_chunk->start = 0;
    head_chunk->size = chunk_size;
    head_chunk->next = NULL;

    return head_chunk;
}

int main(void) {

    head_chunk = create_my_chunk(1600);
    int a = myalloc(10);  // 16바이트 할당 (start=0)
    int b = myalloc(20);  // 32바이트 할당 (start=16)
    int c = myalloc(10);  // 16바이트 할당 (start=48)
    int d = myalloc(40);  // 48바이트 할당 (start=64)
    int e = myalloc(20);  // 32바이트 할당 (start=112)
    print_freed_chunks();
    print_allocated_chunks_with_head(1600);


    myfree_addr(a);   
    print_freed_chunks();
    print_allocated_chunks_with_head(1600);

    myfree_addr(b);   
    print_freed_chunks();
    print_allocated_chunks_with_head(1600);

    myfree_addr(c); 
    print_freed_chunks();
    print_allocated_chunks_with_head(1600);

    myfree_addr(d);   
    print_freed_chunks();
    print_allocated_chunks_with_head(1600);

    myfree_addr(e);   
    print_freed_chunks();
    print_allocated_chunks_with_head(1600);

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

    //best-fit 우선 탐색
    if ((start_addr = search_best_fit(request_size)) != -1) {
        if (insert_allocated_chunk(start_addr, request_size) == -1) {
            printf("Failed insert allocated chunk to list.\n");
            return -1;
        }
        return start_addr;
    }

    ////////exact-fit 우선 탐색
    //if ((start_addr = search_exact_fit(request_size)) != -1) {
    //    return start_addr;
    //}
    ////////first-fit 탐색
    //else if ((start_addr = search_first_fit(request_size)) != -1) {
    //    return start_addr;
    //}
    //모든 탐색에서 공간을 못 찾음.
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

//해제된 청크를 병합
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
    printf("====Freed====\n");
    while (temp_chunk != NULL) {
        printf("%d chunk: (start: %d, size: %d)\n", i++, temp_chunk->start, temp_chunk->size);
        temp_chunk = temp_chunk->next;
    }
    printf("\n");
}

//TODO: 뒤 할당(기존 앞) ["양방향 연결 리스트"]
//TODO: ["이전(다음: 필요할 경우) 주소, 청크 자체에 사이즈 저장 공간 마련(실제 메모리 or 리스트?)", "실제 데이터 저장 가능(pointer?)"]

//보류: 마지막 노드와 병합이 이루어지지 않는 것 같음
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

//할당된 청크를 관리하기 위한 head 생성 이후 함수 정의
//할당된 청크를 관리하는 연결리스트 생성
int insert_allocated_chunk(int start_addr, int request_size) {

    chunk* allocated_chunk = malloc(sizeof(chunk));
    if (allocated_chunk == NULL) return -1;

    //맨 앞에 넣는 경우
    allocated_chunk->next = allocated_chunk_head; //처음은 NULL
    allocated_chunk->start = start_addr; //고유한 주소임.
    allocated_chunk->size = request_size;
    allocated_chunk_head = allocated_chunk;
    return 0;

    ////////맨 뒤에 넣는 경우
    //allocated_chunk->start = start_addr;
    //allocated_chunk->size = request_size;
    //allocated_chunk->next = NULL; //고정

    //chunk* prev_chunk = NULL;
    //chunk* curr_chunk = allocated_chunk_head;

    //while (curr_chunk != NULL) {
    //    prev_chunk = curr_chunk;
    //    curr_chunk = curr_chunk->next;
    //}

    ////아무것도 없는 경우
    //if (prev_chunk == NULL) {
    //    allocated_chunk_head = allocated_chunk;
    //    return 0;
    //}
    //else {
    //    prev_chunk->next = allocated_chunk;
    //    return 0;
    //}
}

//고유한 start_addr 값으로 사이즈 없이 청크 탐색 후 주소 반환
chunk* search_chunk(int start_addr) {

    chunk* curr_chunk = allocated_chunk_head;
    while (curr_chunk != NULL) {
        
        //1. 청크 포인터를 반환하는 방법 (Ret: chunk*)
        //2. 사이즈를 반환하는 방법 (Ret: int)
        if (curr_chunk->start == start_addr) {
            return curr_chunk;
            //return curr_chunk->size;
        }

        curr_chunk = curr_chunk->next;
    }

    printf("The chunk has not exists.\n");
    return NULL;
}

//사이즈 없이 알아서 찾게(search_chunk , myfree 호출)
void myfree_addr(int start_addr) {
    
    chunk* be_freed = search_chunk(start_addr);
    if (be_freed == NULL) {
        printf("be_freed NULL pointer exception.\n");
        return;
    }
    //int size = search_chunk(start_addr);

    myfree(be_freed->start, be_freed->size);
    //myfree(start_addr, size);

    free_from_alloc(be_freed);
}

//할당된 청크 출력(할당 청크 head 있는 경우)
void print_allocated_chunks_with_head(int total_size) {

    chunk* temp_chunk = allocated_chunk_head;
    int count = 0;
    printf("====Allocated====\n");
    while (temp_chunk != NULL) {
        printf("%d used: %d ~ %d\n", count, temp_chunk->start, temp_chunk->start + temp_chunk->size);
        temp_chunk = temp_chunk->next;
        count++;
    }
    printf("\n");
}

//search_chunk를 int 반환형 함수로 만들고 본 함수를 삭제해도 좋으나 원론적으로..
int free_from_alloc(chunk* be_freed) {
    
    //be_freed가 이전 주소와 다음 주소를 가지고 있으면 좋을 듯함
    chunk* prev_chunk = NULL;
    chunk* curr_chunk = allocated_chunk_head;

    while (curr_chunk != NULL && curr_chunk->start != be_freed->start) {
        prev_chunk = curr_chunk;
        curr_chunk = curr_chunk->next;
    }
    if (curr_chunk == NULL) {
        printf("free from alloc error.\n");
        return -1;
    }
    else if (prev_chunk == NULL) {
        allocated_chunk_head = curr_chunk->next;
        free(curr_chunk);
        return 0;
    }
    else {
        prev_chunk->next = curr_chunk->next;
        free(curr_chunk);
        return 0;
    }
}

