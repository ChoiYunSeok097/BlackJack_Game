#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>

#define DECK 52
#define MAX_CARDS_PER_PLAYER 9
#define BEGIN_MONEY 100.0
#define BLACKJACK 21
#define DEALER_MINIMUM 17

enum suits { club = 0, diamond, heart, spade };
enum status { low = 0, blackjack, bust };
enum moves { hit = 0, stand, double_down, info };

typedef struct card {
        int number;
        enum suits suit;
        int color;
        int flag;  // 카드 사용중인지 
}CARD;

typedef struct player {
        double money;
        double bet;
        CARD cards[MAX_CARDS_PER_PLAYER];
        int card_num;
        enum status status;
}PLAYER;


//플레이어 세팅
void player_set(PLAYER *player)
{
	player->money = 10000;
	player->status = 0;
	player->card_num = 0;
}


//베팅 - 인원 수
void money_betting(PLAYER *player,int ns) {
	double num =0;
	char buf[BUFSIZ];
	
	send(ns, "money", 5, 0 );

	sprintf(buf, "%.1lf", player->money);

	send(ns, buf, strlen(buf), 0);
	
	strcpy(buf, "");

	while(1)
	{
		send(ns, "bet", 3, 0);
		recv(ns, buf, sizeof(buf), 0);

		num = (double)atoi(buf);
		if(num > player->money)
			continue;
		else if(num < 0)
			continue;
		break;
	}
	player->bet = num;
	player->card_num = 0;
}

// 덱 만들기, 초기화와 같은 역할
void deckMaker(CARD* deck) {
        int i;
        for (i = 0; i < 13; i++) { // 0 ~ 12 clubs 13 ~ 25  diamonds 26 ~38  hearts 39 ~ 51 spades
                deck[i].number = i + 1;
                deck[i].suit = club;
                deck[i].color = 0;
                deck[i].flag = 0;

                deck[i + 13].number = i + 1;
                deck[i + 13].suit = diamond;
                deck[i + 13].color = 1;
                deck[i + 13].flag = 0;

                deck[i + 26].number = i + 1;
                deck[i + 26].suit = heart;
                deck[i + 26].color = 1;
                deck[i + 26].flag = 0;

                deck[i + 39].number = i + 1;
                deck[i + 39].suit = spade;
                deck[i + 39].color = 0;
                deck[i + 39].flag = 0;
        }
}

//카드 나눠주기
void draw_card(PLAYER *PLAYERS ,CARD *deck, int num) {
        int i, j = 0;
	for(i = 0; i < num; i++) {
		j = 0;
	        while (PLAYERS[i].card_num < 2) {
                        int r = rand() % DECK;
		 	if (deck[r].flag == 0) { // flag : 카드 사용중
		 		if (deck[r].number > 10) deck[r].number = 10;
                               	       deck[r].flag = 1;
			     	       PLAYERS[i].cards[j] = deck[r];
			    	       j++;
                                       PLAYERS[i].card_num++;
			}
		}

		// bj check
		if (PLAYERS[i].cards[0].number == 1) {
                        if (11 + PLAYERS[i].cards[1].number <= BLACKJACK) {
                                PLAYERS[i].cards[0].number = 11;
                        }
                }

                if (PLAYERS[i].cards[1].number == 1) {
                        if (11 + PLAYERS[i].cards[0].number <= BLACKJACK) {
                                PLAYERS[i].cards[1].number = 11;
                        }
                }

                if (PLAYERS[i].cards[0].number + PLAYERS[i].cards[1].number < BLACKJACK) 
                        PLAYERS[i].status = low;
                
                else if (PLAYERS[i].cards[0].number + PLAYERS[i].cards[1].number == BLACKJACK) 
                        PLAYERS[i].status = blackjack;
                else 
                	PLAYERS[i].status = bust;
	}
}

void send_msg(PLAYER *player, char *buf, int ns) {
	send(ns,buf, strlen(buf), 0);

	int i;
	for(i = 0 ; i < player->card_num; i++ ) {
		char str[256];
		char temp[256];
		//enum suits { club = 0, diamond, heart, spade };

		if(player->cards[i].suit == club) 
			strcat(str, "club" );
		else if(player->cards[i].suit == diamond)
                        strcat(str, "diamond" );
  		else if(player->cards[i].suit == heart)
                        strcat(str, "heart" );
		else if(player->cards[i].suit == spade)
                        strcat(str, "spade" );
		strcat(str, " ");
		
		// 11일 경우
		if(player->cards[i].number == 1 || player->cards[i].number == 11)
			strcat(str, "Ace");	
		else
			sprintf(temp, "%d", player->cards[i].number);
			
                strcat(str, temp );
		send(ns, str, strlen(str), 0);}
	send(ns, "done", 4, 0);// 일단 에러 나는지 나중에 확인
}



int addQuestion(PLAYER * player, int ns) {
	int i;
	char *recvBuf[256];
	send(ns, "addQ", 4, 0); // 

        for(i = 0 ; i < player->card_num; i++ ) {
                char str[256];
                char temp[256];
                //enum suits { club = 0, diamond, heart, spade };

                if(player->cards[i].suit == club)
                        strcat(str, "club" );
                else if(player->cards[i].suit == diamond)
                        strcat(str, "diamond" );
                else if(player->cards[i].suit == heart)
                        strcat(str, "heart" );
                else if(player->cards[i].suit == spade)
                        strcat(str, "spade" );
                strcat(str, " ");

                // 11일 경우
                if(player->cards[i].number == 1 || player->cards[i].number == 11)
                        strcat(str, "Ace");
                else
                	sprintf(temp, "%d", player->cards[i].number);       
 	        strcat(str, temp );
                send(ns, str, strlen(str), 0);
	}
        send(ns, "done", 4, 0);// 일단 에러 나는지 나중에 확인
	
	recv(ns , recvBuf, sizeof(recvBuf), 0);

	return (int)(atoi(*recvBuf));
}
	

void game_result(PLAYER dealer, PLAYER *player, int ns) {
        int i;
	char str[256];
        char temp[256];

	send(ns, "result", 6, 0);

	// 딜러
	for( i = 0 ; i < dealer.card_num; i++) {
		if(dealer.cards[i].suit == club)
                        strcat(str, "club" );
                else if(dealer.cards[i].suit == diamond)
                        strcat(str, "diamond" );
                else if(dealer.cards[i].suit == heart)
                        strcat(str, "heart" );
                else if(dealer.cards[i].suit == spade)
                        strcat(str, "spade" );
                strcat(str, " ");

                // 11일 경우
                if(dealer.cards[i].number == 1 || dealer.cards[i].number == 11)
                        strcat(str, "Ace");
                else
                       	sprintf(temp, "%d", dealer.cards[i].number);
                strcat(str, temp );
                send(ns, str, strlen(str), 0);
        }
	
        send(ns, "done", 4, 0);// 일단 에러 나는지 나중에 확인

	strcpy(str, "");
	strcpy(temp, "");

	send(ns, "player", 6, 0);
	for(i = 0 ; i < player->card_num; i++ ) {
                //enum suits { club = 0, diamond, heart, spade };

                if(player->cards[i].suit == club)
                        strcat(str, "club" );
                else if(player->cards[i].suit == diamond)
                        strcat(str, "diamond" );
                else if(player->cards[i].suit == heart)
                        strcat(str, "heart" );
                else if(player->cards[i].suit == spade)
                        strcat(str, "spade" );
                strcat(str, " ");

                // 11일 경우
                if(player->cards[i].number == 1 || player->cards[i].number == 11)
                        strcat(str, "Ace");
                else
                        sprintf(temp, "%d", player->cards[i].number);

                strcat(str, temp );
                send(ns, str, strlen(str), 0);
	}
        send(ns, "done", 4, 0);// 일단 에러 나는지 나중에 확인
	
	if(dealer.status == blackjack) {
		if(player->status == blackjack)
			player->bet = 0;
		else if(player->status == low || player->status == bust)
			player->money -= player->bet;
	}	
	else if(dealer.status == low) {
		if(player->status == blackjack) 
			player->money += player->bet;
                else if(player->status ==  bust)
                        player->money -= player->bet;
		else {
			int i, dealerSum = 0, sum = 0;
                	for (i = 0; i < dealer.card_num; i++)  
				dealerSum += dealer.cards[i].number;
			for (i = 0; i < player->card_num; i++)
                                sum += player->cards[i].number;
 	      		
			if(sum < dealerSum) 
				player->money -= player->bet;
			else if(sum > dealerSum)
				player->money += player->bet;
		}
	}
	else
		player->money += player->bet;
}


//카드 추가
void add_card(PLAYER *player, CARD *deck) {
        int i, j, r;
        int sum = 0;
        
	for (i = 0; i < player->card_num; i++)
		sum += player->cards[i].number;       	
	      
	r = rand() % DECK;
	if (deck[r].flag == 0) {
		if (deck[r].number > 10)
			deck[r].number = 10;                                           
		deck[r].flag = 1;
		player->cards[i] = deck[r];
		player->card_num++;
	}
	
	sum += player->cards[i].number;
	
	if (sum >= BLACKJACK) {     
		for(j = 0; j < player->card_num; j++) {
			if(player->cards[j].number == 11) {
				player->cards[j].number = 1;
				sum -= 10;
			}
			break;
		}                             
 	}       
}


void ben(PLAYER *player, int ns , int pid_num) {
	
	if(player->money <= 0) {
		pid_num--;
		send(ns, "ben", 3, 0);
		close(ns);
	}
}


void dealer_draw(PLAYER* dealer, CARD *deck) {
	int i, j, r;
        int sum = 0; 

	if (dealer->status == low) {  
		for (i = 0; i < dealer->card_num; i++)  
			sum += dealer->cards[i].number;		
	}
	else if(dealer->status == blackjack) 
		sum = BLACKJACK;
	
	

	if (sum < DEALER_MINIMUM) {
		while (1) {                
			r = rand() % DECK;

			if (deck[r].flag == 0) {
				if (deck[r].number > 10)
					deck[r].number = 10;

				dealer->cards[i] = deck[r];		

				dealer->card_num += 1;

				sum += dealer->cards[i].number;

				i++;

				if (sum < DEALER_MINIMUM)
					continue;
				else if(sum >= DEALER_MINIMUM) {
					for(j = 0; j < dealer->card_num; j++) {
						if(dealer->cards[j].number == 11){
			                                dealer->cards[j].number = 1;
                        			        sum -= 10;
                        				break;
						}
					}// for
					if(sum >= DEALER_MINIMUM)
						break;
					else
						continue;
                		}// else if
			}// if
		}// while	
	}//if

	if(sum > BLACKJACK)
		dealer->status = bust;
	else if( sum == BLACKJACK)
		dealer->status = blackjack;
	else
		dealer->status = low;
}

