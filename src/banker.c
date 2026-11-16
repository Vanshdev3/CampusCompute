#include "campuscompute.h"

#include <stdio.h>

#define P 5
#define R 3
static const int allocation[P][R] = {{0,1,0},{2,0,0},{3,0,2},{2,1,1},{0,0,2}};
static const int maximum[P][R] = {{7,5,3},{3,2,2},{9,0,2},{2,2,2},{4,3,3}};

int run_banker(int unsafe_case) {
    int available[R];
    if (!unsafe_case) { available[0]=3; available[1]=3; available[2]=2; puts("Banker's analysis: classic safe state"); }
    else { available[0]=0; available[1]=0; available[2]=0; puts("Banker's analysis: deliberately unsafe state"); }
    int need[P][R], finish[P] = {0}, sequence[P], count = 0;
    puts("Available:"); for (int r=0;r<R;++r) printf("%d%c",available[r],r+1==R?'\n':' ');
    for (int p=0;p<P;++p) for (int r=0;r<R;++r) { need[p][r]=maximum[p][r]-allocation[p][r]; if (need[p][r]<0) { fprintf(stderr,"Invalid state: allocation exceeds maximum.\n"); return 1; } }
    int progress;
    do {
        progress=0;
        for (int p=0;p<P;++p) if (!finish[p]) {
            int possible=1; for(int r=0;r<R;++r) if(need[p][r]>available[r]) possible=0;
            if(possible){for(int r=0;r<R;++r) available[r]+=allocation[p][r]; finish[p]=1; sequence[count++]=p; progress=1;}
        }
    } while(progress);
    if(count==P){printf("SAFE; safe sequence: ");for(int i=0;i<P;++i)printf("P%d%s",sequence[i],i+1==P?"\n":" -> ");return 0;}
    printf("UNSAFE; no complete safe sequence. Processes unable to finish:");for(int p=0;p<P;++p)if(!finish[p])printf(" P%d",p);putchar('\n');return 0;
}
