#include <curses.h>
#include <locale.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h> 
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
struct Coordinate{
	int x;
	int y;
};
enum Food_Type
{
	T1=35,T2=42
};
struct Food{
	struct Coordinate c;
	enum Food_Type ft;
};
struct Snake_Node{
	struct Coordinate c;
	chtype fur;
	struct Snake_Node *next;
};
enum Direction{	//方向
	LEFT,
	RIGHT,
	UP,
	DOWN
};
struct Snake{
	struct Snake_Node *head;
	struct Snake_Node *tail;
	int length;			//长度
	enum Direction dir;	//方向
};
struct Food *food;
struct Snake *snake;
int *stop;
int *pause_game;
int *speed;
int cid;
int i=0;
void init_screen();
void draw(struct Coordinate co, chtype c);
void draw_Snake_Node(struct Snake_Node *sn);
void draw_snake();
void eat();
void init_snake();
void init_food();
void debug();
void snake_move();
void keybordhit();
void redraw();
void recycle();
void sigroutine(int s);
void eat();
void trans_tail_to_head();
void trans_tail_to_head()
{
	snake->head->next=snake->tail;	
	snake->tail=snake->tail->next;
	snake->head=snake->head->next;
	snake->head->next=NULL;
}
void eat(){
	struct Snake_Node *sn=malloc(sizeof(struct Snake_Node));
	sn->c.x=food->c.x;
	sn->c.y=food->c.y;
	sn->fur='@';
	sn->next=NULL;
	
	snake->head->next=sn;
	snake->head=sn;
	snake->length++;
}
//ctrl+c退出父进程时产生SIGINT信号，子进程也会退出不会挂到init
void sigroutine(int s) {
	int status;
	switch(s)
	{
		case SIGCHLD: //子进程退出或被kill时触发，父进程回收僵尸进程
			wait(&status);
			// printf("回收完毕:%d!\n",WEXITSTATUS(status));
			break;
	}
} 
void recycle()
{

	free(food);
	struct Snake_Node *p;
	for(p=snake->tail;p!=NULL;p=p->next)
	{
		free(p);
	}
	munmap(snake,sizeof(struct Snake)+sizeof(int)*3);
	endwin();
	kill(cid, SIGKILL);
	exit(0);
}
void debug(const char *s)
{
	//mvprintw(i++,1,"%d,%d",LINES,COLS);
	#ifdef DEBUG
		//sleep(1);
		if(DEBUG==1){
			mvaddstr(i++,1,s);
		}else{
			mvaddstr(0,1,s);
		}
		refresh();
	#endif
}
void init_screen()
{
	
	setlocale(LC_ALL, "zh_CN.utf8");
	initscr();
	curs_set(0);//隐藏光标
	box(stdscr,0,'_');//边框
	noecho();//不显示输入的字符
	debug("init_screen");
	init_food();
	init_snake();
}
void draw(struct Coordinate co,chtype c)
{
	debug("draw");
	mvaddch(co.y,co.x,c);//在指定位置显示字符
}
void draw_Snake_Node(struct Snake_Node *sn)
{
	debug("draw_Snake_Node");
	draw(sn->c,sn->fur);
}
void draw_snake()
{
	debug("draw_snake");
	mvprintw(0,0,"score is %d, speed is %d",snake->length,*speed);
	refresh();
	struct Snake_Node *p;
	for(p=snake->tail;p!=NULL;p=p->next)
	{
		draw_Snake_Node(p);
	}
}
void init_food()
{
	debug("init_food");
	//food=malloc(sizeof(struct Food));
	// food=mmap(0,sizeof(struct Food),PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
	food=malloc(sizeof(struct Food));
	food->c.x=20;
	food->c.y=20;
	food->ft=T1;
	draw(food->c,food->ft);
}
void init_snake()
{
	debug("init_snake");
	//snake=malloc(sizeof(struct Snake));
	snake=mmap(0,sizeof(struct Snake),PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
	stop=mmap(snake,sizeof(int),PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
	pause_game=mmap(stop,sizeof(int),PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
	speed=mmap(pause_game,sizeof(int),PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED,0,0);
	
	*stop=0;
	*pause_game=0;
	*speed=100000;
	//struct Snake_Node *sn=malloc(sizeof(struct Snake_Node));
	// struct Snake_Node *sn=mmap((char *)snake+sizeof(struct Snake),sizeof(struct Snake_Node),PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_SHARED,0,0);
	struct Snake_Node *sn=malloc(sizeof(struct Snake_Node));
	sn->c.x=10;
	sn->c.y=10;
	sn->fur='@';
	sn->next=NULL;
	snake->head=sn;
	
	
	snake->tail=sn;
	snake->length=1;
	snake->dir=RIGHT;
	draw_snake();
}
void keybordhit()		//监控键盘
{
	debug("keybordhit");
	while(1)
	{
		char ch=getch();
		switch(ch)
		{
			case 'W':
			case 'w':
				if(snake->dir!=DOWN)	//如果本来方向是下,而按相反方向无效
					snake->dir=UP;
				break;
			case 'A':
			case 'a':
				if(snake->dir!=RIGHT)	//如果本来方向是右,而按相反方向无效
					snake->dir=LEFT;
				break;
			case 'S':
			case 's':
				if(snake->dir!=UP)	//如果本来方向是上,而按相反方向无效
					snake->dir=DOWN;
				break;
			case 'D':
			case 'd':
				if(snake->dir!=LEFT)	//如果本来方向是左,而按相反方向无效
					snake->dir=RIGHT;
				break;
			case 'q':
				*pause_game=1;
				attron(A_UNDERLINE);
				mvprintw(9,40,"Are you sure?");
				attroff(A_UNDERLINE);
				refresh();
				char c =getch();
				if(c=='y'){
					recycle();
				}else
				{
					*pause_game=0;
				}
				break;
			case 'p':
				*pause_game=1;
				getch();
				*pause_game=0;
				break;
			case 't':
				*stop=1;
				break;
			case 'l':
				*speed+=1000;
				break;
			case 'k':
				*speed-=1000;
				break;
			default:break;
		}
	}
}
void snake_move()
{
	debug("snake_move");
	struct Coordinate c=snake->head->c;
	struct Coordinate fc=food->c;
	switch(snake->dir){
		case LEFT:
			if(fc.x==c.x-1&&fc.y==c.y){
				eat();
				food->c.x=13;
				food->c.y=16;
			}
			else if(c.x>1)
			{
				snake->tail->c.x=c.x-1;
				snake->tail->c.y=c.y;
				trans_tail_to_head();
			}
			break;
		case RIGHT:
			if(fc.x==c.x+1&&fc.y==c.y){
				eat();
				food->c.x=17;
				food->c.y=18;	
			}
			else if(c.x<COLS-2)
			{
				snake->tail->c.x=c.x+1;
				snake->tail->c.y=c.y;
				trans_tail_to_head();
			}
			break;
		case UP:
			if(fc.x==c.x&&fc.y==c.y-1){
				eat();
				food->c.x=19;
				food->c.y=10;
			}
			else if(c.y>1)
			{
				snake->tail->c.x=c.x;
				snake->tail->c.y=c.y-1;				
				trans_tail_to_head();
			}
			break;
		case DOWN:
			if(fc.x==c.x&&fc.y==c.y+1){
				eat();
				food->c.x=11;
				food->c.y=12;
			}
			else if(c.y<LINES-2)
			{
				snake->tail->c.x=c.x;
				snake->tail->c.y=c.y+1;
				trans_tail_to_head();
			}
			break;
		default:
			assert(0);
	}
}
void redraw()
{
	clear();
	border(0,0,'$','$','$','$','$','$');
	draw_snake();
	draw(food->c,food->ft);
	refresh();
}


int main()
{
	signal(SIGCHLD,sigroutine);
	init_screen();
	if(cid=fork())
	{
		keybordhit();
	}
	else
	{
		while(!*stop){
			if(*pause_game)
			{
				attron(A_UNDERLINE);
				mvprintw(8,40,"pause");
				refresh();
				attroff(A_UNDERLINE);
				mvprintw(8,40,"pause");
				refresh();
				continue;
			}
			snake_move();
			redraw();
			usleep(*speed);
		}		
	}
	return 0;
}
















