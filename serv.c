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
#include <time.h>
#include "blackjack.h"
#include "server.c"

#define PORTNUM 9003
#define MAXLINE 256
#define PLAYER_MAX 5

int main(int argc, char *argv[]) {
	//변수
	struct sockaddr_in sin, cli;
	int sd, ns, clientlen = sizeof(cli);
	int pid[5], *pid_num; //= 0;
      	char buf[MAXLINE+1];
      	int nbytes;
  
	// 랜덤인수 지정
	srand(time(NULL));

	//================게임 데이터 ======================//
	PLAYER players[PLAYER_MAX];
	
	CARD deck[DECK]; 

	int shmid[PLAYER_MAX], shmid_deck[DECK];

	//플레이어 공유 메모리 생성
	player_memory(shmid, players, PLAYER_MAX);
	
	//덱 공유 메모리 생성
	deck_memory( shmid_deck, deck);
  
	//게임시작 공유 메모리 생성
	int shgame;
	shgame = shmget(IPC_PRIVATE, 20, IPC_CREAT|0644);     //공유메모리 생성

	if(shgame == -1) {	
	      	perror("shmget");
      		exit(1);
	}	
	
	pid_num = (int *)shmat(shgame, NULL, 0); //공유메모리 연결
	*pid_num = 1;        

	int state;
	state = shmget(IPC_PRIVATE, 20, IPC_CREAT|0644);     //공유메모리 생성

	if(state == -1) {
        	perror("shmget");
	        exit(1);
	}
	
	int *ready = (int *)shmat(state, NULL, 0); //공유메모리 연결
	*ready = 0;		// 초기값 0

	// 게임 진행중 확인 공유 메모리
	int ingame;
	ingame = shmget(IPC_PRIVATE, 20, IPC_CREAT|0644);     //공유메모리 생성
	
	if(ingame == -1) {
		perror("shmget");
	        exit(1);
	}
	
	int *game_state  = (int *)shmat(ingame, NULL, 0); //공유메모리 연결
	*game_state = 0;
	
      	//==============서버==================
	//명령인자 - 도움말, 소켓연결
			
	
	if((sd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket");
		exit(0);
	}

	memset((char *)&sin, '\0',sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(PORTNUM);
	sin.sin_addr.s_addr = inet_addr("10.0.2.15");

	if(bind(sd,(struct sockaddr *)&sin, sizeof(sin))) {
		perror("bind");
		exit(0);
	}

	if(listen(sd, 5)) {
		perror("listen");
		exit(1);
	}

	//연결받기

	if((ns = accept(sd, (struct sockaddr *)&cli, &clientlen)) == -1) {
		perror("accept");
		exit(1);
	}

	//===================================================
	//자식 프로세스 생성
	if((pid[(*pid_num)] = fork()) == -1) {
	      	close(ns);
      		perror("fork()");
      		exit(0);
        }	

	//자식 프로세스
       	if(pid[*pid_num] == 0) {   // check
		int id = *pid_num++;
		PLAYER *player = &players[id];
		player_set(player);			//초기화		
		
		while(1) {				// 게임 반복문
			if(*game_state == 1) { 		// 게임중 
				sleep(1);
				continue;
			}
			sleep(1);
			ready++;
			money_betting(player,ns);	// 베팅
		
		
			// 일단 남겨두는 덱 초기화
		
			//카드 나눠주기-서버가 할 일
	
			while(1) {				// 카드뽑기 반복문
				if(player->status == blackjack) {
					 send_msg(player, "BJ", ns);
					 // 사용자에게 메세지 보내기
					 break;
				}

				// 더뽑을 것인지 아닌지 받아야함	
				if(addQuestion(player, ns) == 0)
					add_card(player, deck);
				else
					break;
				
				if(player->status == bust) {
	                                send_msg(player, "BUST", ns); // 사용자에게 메세지 보내기
					break;
                                }   
			}// 안쪽의 while
		
			// 결과 전달
			game_result(players[0], player,ns);	 
		 
			// ben
			ben(player, ns, *pid_num);
					
			ready -= 1;	
   		}// 바깥쪽의 while
	} //자식 프로세스 영역

	// 부모프로세스
  	if (pid[*pid_num] > 0) {
		while(1) {	
			sleep(1);
		
			while(*ready != *pid_num) {
				sleep(5);
			}	
	
			*game_state = 1;

			deckMaker(deck); 		   // 덱 초기화

			PLAYER *player = &players[0];
			player_set(player);

			draw_card(players, deck, *pid_num);// 카드 나누기, 전부 2장 뽑음
	
			dealer_draw(&(players[0]), deck);	//딜러 드로우	
				
			while(*ready != 0) 
				sleep(1);
		}// while
	} // 부모 프로세스 영역 끝

	return 0;		
}
