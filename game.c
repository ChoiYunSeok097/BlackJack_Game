#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORTNUM 9003

#define WIDTH 25
#define HEIGHT 8 
#define CARDMAX 9

//variable
int startx = 0;
int starty = 0;
int money =0;
int ns;
struct sockaddr_in son;
char card_info[CARDMAX][10],dealer_info[CARDMAX][10],BOB[10];

WINDOW *menu_win;
WINDOW *Dealer_win;
WINDOW *Player_win;

//method
void print_menu(WINDOW *menu_win, char * BOB);
void print_Player(WINDOW *,char ch[][10]);
void card_set(char card[][10]);
int recv_msg();


int main()
{

        //start window
        int choice = 0;
	int c;
	
	//소켓 연결
	if ((ns = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	
	memset((char *)&son, '\0', sizeof(son));
	son.sin_family = AF_INET;
	son.sin_port = htons(PORTNUM);
	son.sin_addr.s_addr = inet_addr("10.0.2.15"); // server ip address

	
	if (connect(ns, (struct sockaddr *)&son, sizeof(son))) {
		perror("connect");
		exit(1);
	 }

	
        //setting
        initscr();
        clear();
        noecho();
        cbreak();       // Line buffering disabled. pass on everything 

	
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

	
        //set keypad
        //keypad(menu_win, TRUE);
	
        //mvprintw(0, 0, "choose your choice");
        refresh();

	
        //print
	card_set(card_info);
	card_set(dealer_info);
	
	
	while(1){

		strcpy(BOB,"");
        	print_menu(menu_win,"");
        	print_Player(Dealer_win,card_info);
        	print_Player(Player_win,dealer_info);

		if(recv_msg() == 0)
		break;
		sleep(0.5);
	}
	
        clrtoeol();
        refresh();
        endwin();
        close(ns);		//소켓닫기
		
	return 0;

}

void print_menu(WINDOW *menu_win,char *buf)
{
        int x, y, i;

        //position
        x = 2;
        y = 2;
        box(menu_win, 0, 0);

        //menu print
	
        mvwprintw(menu_win, y, x, "%s", "your money : ", money);
        y++;
        mvwprintw(menu_win, y, x, "%s",buf);
	
	if(strcmp(BOB,"") !=0)
		mvwprintw(menu_win, y, x, "your state : %s",BOB);

        wrefresh(menu_win);
}


void print_Player(WINDOW *Player_win,char card_info[][10])
{
        int x,y,i;

        //position
        x=3;
        y=3;
        box(Player_win, 0, 0);

        for(i = 0; i < CARDMAX; ++i)
        {
		if(strcmp(card_info[i],"") != 0){
                	mvwprintw(Player_win, y, x, "%s",*card_info[i]);
                	y++;
		}
        }
        wrefresh(Player_win);
}

void card_set(char card[][10])
{
	int i=0;

	for( ; i<CARDMAX; i++){
		printf("%d",i);
		strcpy(card[i],"");
	}
}

int recv_msg()
{
	char buf[50];
	recv(ns, buf, sizeof(buf), 0);

	//돈의 값을 받아야 할 떄
	if(strcmp("money",buf) == 0)
	{	
		printf("%s",buf);
		strcpy(buf,"");
		recv(ns, buf, sizeof(buf),0);	//돈 정보 받기
		money = atoi(buf);

	}

	//배팅하기
	else if(strcmp("bet",buf) == 0)
	{
		printf("%s",buf);
		print_menu(menu_win,"배팅할 금액을 입력하시오\n");
		strcpy(buf,"");
		//money = atoi(getstr(buf));
		getstr(buf);

		send(ns, buf, sizeof(buf), 0);		//배팅 금액 전달
	}

	//블랙잭 일 때
	else if(strcmp("BJ",buf) == 0)
        {
		int k=0;
       		
		printf("%s",buf);
		while(1)
		{
			strcpy(buf,"");
			recv(ns, buf, sizeof(buf), 0);	//카드 정보 복사
			if(strcmp(buf,"done") == 0) break;
			strcpy(card_info[k++], buf);
		}
	}

	//패가 터졋을 때
        else if(strcmp("BUST",buf) == 0)
        {
                printf("%s",buf);
		int k=0;
                strcpy(BOB,"BUST");
		
		while(1)
		{
			strcpy(buf,"");
			recv(ns, buf, sizeof(buf), 0);  //카드 정보 복사
			if(strcmp(buf,"done") == 0) break;
			strcpy(card_info[k++], buf);
		}
        }
	
	//패를 더 뽑을 것인지 질문
	else if(strcmp("addQ",buf) == 0)
        {
                int k=0;
 
		printf("%s",buf);
		while(1)
                {
                        strcpy(buf,"");
                        recv(ns, buf, sizeof(buf), 0);  //카드 정보 복사
                        if(strcmp(buf,"done") == 0) break;
                        strcpy(card_info[k++], buf);
                }

		print_menu(menu_win,"카드를 더 받으시겠습니까? 더받기 : 0, 아니요 : 다른 키\n");
		strcpy(buf,"");
		getstr(buf);

		send(ns, buf, sizeof(buf), 0);
         }

	//게임 결과를 받기(플레이어 정보, 딜러정보)
	else if(strcmp("result",buf) == 0)
        {
                int k=0;
		printf("%s",buf);
		//딜러 정보
                while(1)
                {
                        strcpy(buf,"");
                        recv(ns, buf, sizeof(buf), 0);  //카드 정보 복사
                        if(strcmp(buf,"done") == 0) break;
                        strcpy(dealer_info[k++], buf);
                }

		while(1)
                {
                        strcpy(buf,"");
                        recv(ns, buf, sizeof(buf), 0);  //카드 정보 복사
                        if(strcmp(buf,"done") == 0) break;
                        strcpy(card_info[k++], buf);
                }
	
		//결과후 카드 정보 리셋	
		card_set(card_info);
	        card_set(dealer_info);
         }

	//탈락 일 때
        else if(strcmp("ben",buf) == 0)
        {
		printf("%s",buf);
                int k=0;
		return 0;
        }

	return 1;
}




