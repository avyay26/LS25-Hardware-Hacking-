#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "utils.h"

#define SYNC_GAP 1000000ULL
#define SHM_NAME "/smem"
#define SHM_SIZE 30
#define CYCLES_PER_BIT 500000ULL  // Must match sender

#define MAX_BITS (MAX_MSG_SIZE * 8)
#define THRESHOLD 100  // Must be tuned per system

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

// Global for accuracy checker
char* received_msg = NULL;
int received_msg_size = 0;

int main() {
    long long st=rdtsc();
    char* pt=malloc(SHM_SIZE);
    int shm_fd=shm_open(SHM_NAME,O_RDWR,0666);
    char * shm=mmap(0,SHM_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
    int i=1;
    while(rdtsc()<st + (SYNC_GAP+0.1)*100000){}
    printf("Ready to attack!!");
    flush(stdout);
    while(*shm==1)
      {while(rdtsc()<st+(SYNC_GAP*i+0.1)*100000);
        long long a=rdtsc();
        maccess(shm);
        long long b=rdtsc();
        if(b-a < THRESHOLD){pt[i]='1';}
        else{pt[i]='0';}
        i++;
        flush(shm);
      }
    pt[i]='\0';
    received_msg = pt;
    received_msg_size = i;

    // DO NOT MODIFY THIS LINE
    printf("Accuracy (%%): %f\n", check_accuracy(received_msg, received_msg_size) * 100);

    // Cleanup
    munmap(shm, SHM_SIZE);
    close(shm_fd);
    free(received_msg);
}
