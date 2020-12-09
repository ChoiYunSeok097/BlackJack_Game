#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>

//플레이어 공유메모리 만들기			
void player_memory(int *shmid, PLAYER *players, int PLAYER_MAX){
	int i = 0;
	for( ; i < PLAYER_MAX; i++) {
		*shmid = (int)shmget(IPC_PRIVATE, 20, IPC_CREAT|0644);     //공유메모리 생성
		if(*shmid == -1) {
			perror("shmget"); exit(1);}
		players = (PLAYER *) shmat(*shmid, (PLAYER *)NULL,0); //공유메모리 연결
		players++;
		shmid++;
	}
}

//덱 공유메모리 만들기
void deck_memory(int *deck_shmid, CARD* deck) {
        int i = 0;
        for(; i < DECK; i++) {
                *deck_shmid = (int)shmget(IPC_PRIVATE, 20, IPC_CREAT|0644);     //공유메모리 생성
                if(*deck_shmid == -1) {
                        perror("shmget");
		       	exit(1);
		}

                deck = (CARD *)shmat(*deck_shmid, (CARD *)NULL, 0); //공유메모리 연결
		deck_shmid++;
		deck++;
        }
}

/*
void checkBJ_P(PLAYER *PLAYERS[], int pid_num) {
        int i;
        for (i = 0; i <= pid_num; i++) {
                if (PLAYERS[i]->status == blackjack) {
                        // 게임 끝                                   
                }
        }
}
*/
