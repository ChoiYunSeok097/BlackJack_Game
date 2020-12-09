#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <ncurses.h>


#define WIDTH 25
#define HEIGHT 8 

#define DECK 52
#define BEGIN_MONEY 100
#define MAX_CARDS_PER_PLAYER 9
#define MINIMUM_BET 10
#define BLACKJACK 21
#define DEALER_MINIMUM 17

enum suits { club = 0, diamond, heart, spade };
enum status { live = 0, blackjack, bust };

typedef struct card {
        int number;
        enum suits suit;	//모양
        int color;		//색갈
        int flag;  		//
}CARD;

typedef struct player {
        int money;		//돈
        int bet;		//베팅
        CARD *cards[MAX_CARDS_PER_PLAYER];	//카드
        int card_num;				//카드의 갯수
        enum status status;			//상태
}PLAYER;

int startx = 0;
int starty = 0;
int mod = 1;

void print_box(WINDOW *win,char *buf,int i);
void player_set(PLAYER *p);
void player_unset(PLAYER *p);
void deckMaker(CARD *deck);
void game_start();
int money_bet();
void draw_card(PLAYER *player);
int print_game(int turn);
void game_result();
int add_card(PLAYER *);
void dealer_draw();
int ben();
int card_sum(PLAYER *p);
int check_bj(PLAYER *p);

PLAYER *player;
PLAYER *dealer;
CARD deck[DECK];

//start window
WINDOW *menu_win;
WINDOW *Dealer_win;
WINDOW *Player_win;

int main(void) {
	
	//setting
        initscr();
        clear();
        noecho();
        cbreak();

	//box location
        startx = (80 - WIDTH) / 2;
        starty = (24 - HEIGHT) / 2;

	//color
        start_color();
        init_pair(1,COLOR_WHITE,COLOR_BLACK);
        init_pair(2,COLOR_BLACK,COLOR_WHITE);
        wattron(menu_win,COLOR_PAIR(1));

	//call window
        menu_win = newwin(HEIGHT, WIDTH, starty, startx);
        Dealer_win = newwin(HEIGHT, WIDTH, 0, startx);
        Player_win = newwin(HEIGHT, WIDTH, starty+8, startx);

	refresh();

	//카드 정보
	player = (PLAYER*)malloc(sizeof(PLAYER));
	dealer = (PLAYER*)malloc(sizeof(PLAYER));
	
	system("clear");
	srand(time(NULL));
	
	printf("Welcome to Blackjack Station");

	//set keypad
        keypad(menu_win, TRUE);

	game_start();	

	//wgetch(menu_win);
	sleep(1);

	clrtoeol();
        refresh();
        endwin();

	free(player);
	free(dealer);
	player = NULL;
	dealer = NULL;

	return 0;
}

void player_set(PLAYER *p) {
       	int i = 0;
	for(i = 0; i < MAX_CARDS_PER_PLAYER; i++) {
		 p->cards[i] = (CARD *)malloc(sizeof(CARD));
	}
	p->status = 0;
        p->card_num = 0;
}

void player_unset(PLAYER *p) {
	int i = 0;
        for(i = 0; i < MAX_CARDS_PER_PLAYER; i++) {
                 free(p->cards[i]);
        }
}

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

void print_box(WINDOW *win,char *buf,int i)
{
        int x,y,card_num;

        //position
        x=3;
        y=3+i;
        box(win, 0, 0);

        mvwprintw(win, y, x, "%s",buf);
        //wrefresh(Player_win);
}

void game_start() {
	char buf[256];

	//처음 게임 돈
	player->money = BEGIN_MONEY;

	

	while(1){
		player_set(player);
		player_set(dealer);
	        deckMaker(deck);

		if((money_bet()) == -1) {
			printf("Bye!\n");
			break;
		}

		//처음 두장 드로우
		draw_card(player);
		draw_card(dealer);
		
		//블랙잭 나올 시 종료
		if(check_bj(player) || check_bj(dealer)) {		// 여기
			
			if(check_bj(player))
				printf("===you got BlackJack===\n\n\n");
			else
				printf("===Dealer got BlackJack===\n\n\n");
			
			print_game(0);
			game_result();
		}
			
		//카드 뽑기
		print_game(1);
		while(add_card(player) != 0) {
			print_game(1);
			check_bj(player);
			if(player->status != live)
				break;
		}
		
		//딜러 뽑기
		dealer_draw();	
		
		//출력
		print_game(0);			// 게임판 출력
		wgetch(menu_win);

		//game_result();			// 게임 결과 출력

		if(ben() == 1)
			break;
		
		//printf("press Enter");
		//getchar();
		//system("clear");
		
	}
}

int money_bet() {
	int num = 10;

	wrefresh(menu_win);
        //printf("\nyour money: %d\n", player->money);
	char str[20];
	char temp[10];

	while(1) {
		wrefresh(menu_win);
		sprintf(temp,"%d",player->money);
        	strcpy(str,"your money : ");
        	strcat(str,temp);

        	print_box(menu_win,str,0);


		//printf("\ninput bet: ");
		strcpy(temp,"");
		sprintf(temp,"%d",num);
		strcpy(str,"");
		strcpy(str,"now bet : ");
                strcat(str,temp);

		print_box(menu_win,str,1);
		print_box(menu_win,"double",2);
		print_box(menu_win,"pass",3);

		//wgetch(menu_win);

		//scanf("%d", &num);
		//strcpy(str,"");
		//getstr(str);
		//int c = mvscanw(3, 3, "%d", &num);        	
		int c = wgetch(menu_win);

		switch(c)
                {
			case '0':
				return -1;
				break;
                        case '1':
				num = num*2;

				break;
                        case '2':
                                break;
			refresh();
		}
		if(c == '1'){
		       	if(num  > player->money) {
                        	// 돈보다 많은 배팅
                        	num /=num;
			}
			continue;
		} else if(num < 0) {
			// 잘못된 배팅
			continue;
		} else if(num == 0) {
			return -1;
		}else if(num >= MINIMUM_BET) {
			player->bet = num;
			break;
		}
	}
        player->bet = num;
        player->card_num = 0;

	return 0;
}

//처음 카드 두장 뽑기
void draw_card(PLAYER *p) {
	int i = 0;

        while (p->card_num < 2) {
		i = p->card_num;
		int r = rand() % DECK;       
	 	if (deck[r].flag == 0) { // flag : 카드 사용중		
			deck[r].flag = 1;
			p->cards[i] = &deck[r];
			p->card_num++;
     		}
       	}
}

//게임 정보 출력(카드정보, 딜러정보, 플레이어 정보)
int print_game(int turn) {
	int i, result = 0;	
	char str[256] = "";
        char temp[256]= "";

	refresh();

	//내 카드 출력
	//printf("\nyour cards:\n\t");
	for(i = 0 ; i < player->card_num; i++ ) {
                //enum suits { club = 0, diamond, heart, spade };
                if(player->cards[i]->suit == club)
                        strcat(str, "club" );
                else if(player->cards[i]->suit == diamond)
                        strcat(str, "diamond" );
                else if(player->cards[i]->suit == heart)
                        strcat(str, "heart" );
                else if(player->cards[i]->suit == spade)
                        strcat(str, "spade" );
                strcat(str, " ");

                if(player->cards[i]->number == 1)
			strcat(str, "Ace ");
		else if(player->cards[i]->number == 11)
			strcat(str, "Jack ");
		else if(player->cards[i]->number == 12)
                        strcat(str, "Queen ");
		else if(player->cards[i]->number == 13)
                        strcat(str, "King ");
                else {
                        sprintf(temp, "%d", player->cards[i]->number);
			strcat(str, temp );
			strcat(str, " ");
			strcpy(temp,"");
		}
	}
        //printf("%s \n", str);
	print_box(Player_win,str,1);
	strcpy(str ,"");
        
	//딜러카드 출력
	//printf("\ndealer's card:\n\t");

	if(turn == 1) {
		printf("X \t");
		i = 1;
	} else 
		i = 0;

	for( ; i < dealer->card_num; i++ ) {
                //enum suits { club = 0, diamond, heart, spade };
                if(dealer->cards[i]->suit == club)
                        strcat(str, "club" );
                else if(dealer->cards[i]->suit == diamond)
                        strcat(str, "diamond" );
                else if(dealer->cards[i]->suit == heart)
                        strcat(str, "heart" );
                else if(dealer->cards[i]->suit == spade)
                        strcat(str, "spade" );
                strcat(str, " ");

                if(dealer->cards[i]->number == 1)
                        strcat(str, "Ace ");
                else if(dealer->cards[i]->number == 11)
                        strcat(str, "Jack ");
                else if(dealer->cards[i]->number == 12)
                        strcat(str, "Queen ");
                else if(dealer->cards[i]->number == 13)
                        strcat(str, "King ");
		else {
			sprintf(temp, "%d", dealer->cards[i]->number);
                        strcat(str, temp );
                        strcat(str, " ");
			strcpy(temp,"");
                }
        }

	//printf("%s \n", str);
	print_box(Dealer_win,str,1);
        strcpy(str ,"");
	int c = wgetch(Player_win);

	check_bj(player);
	check_bj(dealer);

	//printf("\nyour's status : ");
        if(dealer->status == live);
                //printf("low");
        else if(dealer->status == blackjack) {
                //printf("BlackJack!!");
                result = 1;
        }
	else {
                //printf("bust");
		result = 1;
	}

	//printf("\tdealer's status : ");
        if(dealer->status == live);
                //printf("low\n");
        else if(dealer->status == blackjack) {
                //printf("BlackJack!!\n");
                result = 1;
        }
        else {
                //printf("bust\n");
                result = 1;
        }

	return result;
}

//카드 추가
int add_card(PLAYER *p) {
	int i, j, r;
        int sum = 0;
	char str[100];		
	
	wrefresh(menu_win);
	//카드 뽑을지 질문
	//printf("-----------------------");
	//printf("\nadd card? y or n :");
	print_box(menu_win,"add card? 1 or not 2 :",0);

	//fgets(str,sizeof(str), stdin);	
	
	int c = wgetch(menu_win);

	if(c != '2') {	// 뽑지 않으면 종료
		check_bj(p);
		return 0;
	}	
	
	//뽑을시
	while(c == '1'){
		i = p->card_num;
	        r = rand() % DECK;
	
		//안뽑은 카드일 때
        	if(deck[r].flag == 0) {
        	        deck[r].flag = 1;
        	        p->cards[i] = &deck[r];
        	        p->card_num++;
			break;
        	}
	}
	return 1;
}
  
//게임 결과
void game_result() {
	char buf[20] = "";

	 if(dealer->status == blackjack) {
                if(player->status == blackjack) {		//비김
			player->bet = 0;
		       	strcpy(buf,"draw");
		}
                else if(player->status == live || player->status == bust) {//플레이어 패배
			player->money -= player->bet;
		       	strcpy(buf,"player lose");
		}
        }
        else if(dealer->status == live) {
                if(player->status == blackjack) {		//플레이어 블랙잭 승리
			player->money += player->bet; 
			strcpy(buf,"player win");
		}
                else if(player->status == bust) {	//플레이어 패배
			player->money -= player->bet; 
			strcpy(buf,"player lose");
		}
                else {					//둘다 살아있을 때
		       	int dealerSum = card_sum(dealer);
			int sum = card_sum(player);
                        
                        if(sum < dealerSum)		//승패 비교
				{player->money -= player->bet; strcpy(buf,"player lose");}
                        else if(sum > dealerSum)
				{player->money += player->bet; strcpy(buf,"player win");}
                }
        }
        else {
		if(player->status == bust)
			player->money -= player->bet;
		else
			player->money += player->bet;
	}

	//printf("%s\n",buf);
	print_box(menu_win,buf,1);
}

void dealer_draw() {
        int i =0 , r = 0;
        
	if(card_sum(dealer) < DEALER_MINIMUM) {
		while(1) {
			i = dealer->card_num;
			r = rand() % DECK;
			if(deck[r].flag == 0) {
				deck[r].flag == 1;
				dealer->cards[i] = &deck[r];
				dealer->card_num++;
			}
			if(card_sum(dealer) < DEALER_MINIMUM) 
				continue;
		}
	}
	check_bj(dealer);	
}
             
int ben() {
	if(player->money == 0) {
		printf("youer money : 0\n");
		return 1;
	}
	return 0;
}

//카드들의 합
int card_sum(PLAYER *p) {
	int i, sum = 0, k = 0, cnt = 0;

	for(i = 0; i < p->card_num; i++) {
		k = p->cards[i]->number;
		if(k == 11 || k == 12 || k == 13)
			sum += 10;
		else if(k == 1 && cnt == 0) {
			sum += 11;
			cnt++;
		}
		else
			sum += k;
	}

	if(sum > BLACKJACK) {
		sum = 0;
		if(cnt > 0 ) {
			for(i = 0; i < p->card_num; i++) {
				k = p->cards[i]->number;
				if(k == 11 || k == 12 || k == 13)
					sum += 10;
				else
					sum += k;
			}
		}	
	}
	return sum;
}

//블랙잭인지 확인하고 status 바꿔주기
int check_bj(PLAYER* p) {
	int i, k = 0;
		
	if(card_sum(p) == BLACKJACK) {		// 블랙잭	
		p->status = blackjack;
		return 1;
	}
	else if(card_sum(p) > BLACKJACK) {
		p->status = bust;
		return 2;
	}
	else
		p->status = live;
	
	return 0;
}
