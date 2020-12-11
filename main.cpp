#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <time.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdbool.h> 
#define _BSD_SOURCE 
#include <sys/time.h> 
#include <stdio.h> 
  
#define BSIZE 8 // ������ ������
#define PWT 2 // ����������� ������� �������� �������������
#define CWT 10 // ����������� ������� �������� �����������
#define RT 10 // ����� ���������� ��������� � ��������  
int a1_id, a2_id, a3_id, a4_id; 
key_t k1 = 5491, k2 = 5812, k3 = 4327, k4 = 3213; 
bool* A1; 
int* A2; 
int* A3; 
  
int myrand(int n) { // ���������� ��������� ����� �� 1 �� n
    time_t t; 
    srand((unsigned)time(&t)); 
    return (rand() % n + 1); 
} 
  
int main() { 
    a1_id = a_get(k1, sizeof(bool) * 2, IPC_CREAT | 0660); // ���� 
    a2_id = a_get(k2, sizeof(int) * 1, IPC_CREAT | 0660); // ������� 
    a3_id = a_get(k3, sizeof(int) * BSIZE, IPC_CREAT | 0660); // ����� 
    a4_id = a_get(k4, sizeof(int) * 1, IPC_CREAT | 0660); // ����� 
  
    if (a1_id < 0 || a2_id < 0 || a3_id < 0 || a4_id < 0) { 
        perror("Main a_get error: "); 
        exit(1); 
    } 
    A3 = (int*)a(a3_id, NULL, 0); 
    int ix = 0; 
    while (ix < BSIZE) // ������������� ������
        A3[ix++] = 0; 
  
    struct timeval t; 
    time_t t1, t2; 
    gettimeofday(&t, NULL); 
    t1 = t.tv_sec; 
  
    int* state = (int*)a(a4_id, NULL, 0); 
    *state = 1; 
    int wait_time; 
  
    int i = 0; // �����������
    int j = 1; // �������������
  
    if (fork() == 0) { // ��� �������������
        A1 = (bool*)a(a1_id, NULL, 0); 
        A2 = (int*)a(a2_id, NULL, 0); 
        A3 = (int*)a(a3_id, NULL, 0); 
        if (A1 == (bool*)-1 || A2 == (int*)-1 || A3 == (int*)-1) { 
            perror("Producer a error: "); 
            exit(1); 
        } 
        bool* flag = A1; 
        int* turn = A2; 
        int* buf = A3; 
        int index = 0; 
        while (*state == 1) { 
            flag[j] = true; 
            printf("Producer is ready now.\n\n"); 
            *turn = i; 
            while (flag[i] == true && *turn == i);
            // ������ ������������ �������
            index = 0; 
            while (index < BSIZE) { 
                if (buf[index] == 0) { 
                    int tempo = myrand(BSIZE * 3); 
                    printf("Job %d has been produced\n", tempo); 
                    buf[index] = tempo; 
                    break; 
                } 
                index++; 
            } 
            if (index == BSIZE) 
                printf("Buffer is full, nothing can be produced!!!\n"); 
            printf("Buffer: "); 
            index = 0; 
            while (index < BSIZE) 
                printf("%d ", buf[index++]); 
            printf("\n"); 
            // ����� ����������� ������
  
            flag[j] = false; 
            if (*state == 0) 
                break; 
            wait_time = myrand(PWT); 
            printf("Producer will wait for %d seconds\n\n", wait_time); 
            sleep(wait_time); 
        } 
        exit(0); 
    } 
  
    if (fork() == 0) { // ��������������� ��� 
        A1 = (bool*)a(a1_id, NULL, 0); 
        A2 = (int*)a(a2_id, NULL, 0); 
        A3 = (int*)a(a3_id, NULL, 0); 
        if (A1 == (bool*)-1 || A2 == (int*)-1 || A3 == (int*)-1) { 
            perror("Consumer a error:"); 
            exit(1); 
        } 
  
        bool* flag = A1; 
        int* turn = A2; 
        int* buf = A3; 
        int index = 0; 
        flag[i] = false; 
        sleep(5); 
        while (*state == 1) { 
            flag[i] = true; 
            printf("Consumer is ready now.\n\n"); 
            *turn = j; 
            while (flag[j] == true && *turn == j);
  
            // ������ ������������ �������
            if (buf[0] != 0) { 
                printf("Job %d has been consumed\n", buf[0]); 
                buf[0] = 0; 
                index = 1; 
                while (index < BSIZE){ // ������� ���������� ������� ���� ������                
                    buf[index - 1] = buf[index]; 
                    index++; 
                } 
                buf[index - 1] = 0; 
            } else
                printf("Buffer is empty, nothing can be consumed!!!\n"); 
            printf("Buffer: "); 
            index = 0; 
            while (index < BSIZE) 
                printf("%d ", buf[index++]); 
            printf("\n"); 
            // ����� ����������� ������
            flag[i] = false; 
            if (*state == 0) 
                break; 
            wait_time = myrand(CWT); 
            printf("Consumer will sleep for %d seconds\n\n", wait_time); 
            sleep(wait_time); 
        } 
        exit(0); 
    } 
    while (1) { 
        gettimeofday(&t, NULL); 
        t2 = t.tv_sec; 
        if (t2 - t1 > RT) { 
            *state = 0; 
            break; 
        } 
    } 
    // �������� ������ ����� ���������
    wait(); 
    wait(); 
    printf("The clock ran out.\n"); 
    return 0; 
} 
