#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SYNC_GAP 1000000ULL
#define SHM_NAME "/smem"
#define SHM_SIZE 30
#define CYCLES_PER_BIT 500000ULL  // Tune this to match your system

unsigned long long rdtsc() {
    unsigned long long a, d;
    asm volatile ("mfence");
    asm volatile ("rdtsc" : "=a" (a), "=d" (d));
    a = (d << 32) | a;
    asm volatile ("mfence");
    return a;
}

void maccess(void* p) {
    asm volatile ("movq (%0), %%rax\n"
        :
        : "c" (p)
        : "rax");
}

void flush(void* p) {
    asm volatile ("clflush 0(%0)\n"
        :
        : "c" (p)
        : "rax");
}

int main() {
    long long startu=rdtsc();
    shm_unlink(SHM_NAME);
    const char* shname=SHM_NAME;
    int fd;
    fd=shm_open(shname,O_RDWR|O_CREAT,0666);
    ftruncate(fd,SHM_SIZE);
    char* shm;
    shm=(char*)mmap(0,SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
    // ********** DO NOT MODIFY THIS SECTION **********
    FILE *fp = fopen(MSG_FILE, "r");
    if(fp == NULL){
        printf("Error opening file\n");
        return 1;
    }

    char msg[MAX_MSG_SIZE];
    int msg_size = 0;
    char c;
    while((c = fgetc(fp)) != EOF){
        msg[msg_size++] = c;
    }
    fclose(fp);

    clock_t start = clock();
    // **********************************************
    // ********** YOUR CODE STARTS HERE **********

    // Synchronize with receiver
    while (rdtsc() < startu + SYNC_GAP * 100000){};
    printf("Synchronization complete. Starting transmission...\n");
    sprintf(shm,"%d",1);
    // Notify receiver (e.g., by writing '1' to shared mem)
    // Start transmission loop
    int msgrem=msg_size;
    int i=1;
    for(int i=0;i<msg_size;i++)
    {
        while(rdtsc()<startu+(SYNC_GAP*i)*100000){};
        if(msg[i]=='1'){maccess(shm);}
        else{}
    }
    
    // ********** YOUR CODE ENDS HERE **********
    // ********** DO NOT MODIFY THIS SECTION **********
    clock_t end = clock();
    double time_taken = ((double)end - start) / CLOCKS_PER_SEC;
    printf("Message sent successfully\n");
    printf("Time taken to send the message: %f\n", time_taken);
    printf("Message size: %d\n", msg_size);
    printf("Bits per second: %f\n", msg_size * 8 / time_taken);
    // **********************************************

    munmap(shm, SHM_SIZE);
    close(fd);
    shm_unlink(SHM_NAME);
}
