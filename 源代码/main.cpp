#include<stdio.h>
#include<conio.h>
#include<time.h>
#include<math.h>
#include<graphics.h>
#include<easyx.h>


#define FilePath1 "timeer.txt"
#define FilePath "counter.txt"
#define WIDTH 1024								// 屏幕宽
#define HEIGHT 576								// 屏幕高
#define MAPW (WIDTH*4)								// 地图宽
#define MAPH (HEIGHT*4)							// 地图高
#define AINUM 100 							    // AI 数量
#define FNUM 2000									// FOOD 数量
#define DISTANCE(x1,y1,x2,y2)	(sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)))		//计算距离

struct FOOD {
	bool eat;
	COLORREF color;								// 颜色
	int x, y;									// 坐标
	char type;
};

struct BALL {										//小球结构体
	bool life;									//生命
	COLORREF color;								//颜色
	int x, y;									//坐标
	float r;										//半径
};

FOOD food[FNUM];									//食物
BALL mover = { 1,RGB(0,0,0),0,0,0 };				//玩家
BALL ai[AINUM] = { 1,RGB(0,0,0),0,0,0 };			//AI

void ChooseSpeed();
void move(BALL* ball);								// 玩家移动
void draw();										// 绘图
void start();										// 游戏开始
void setall();									// 初始化
void AI();										// AI
void Food();										// 食物
void starttime();
int readCount();
void writeCount(int);
int readTime();
void writeTime(int);
void endtime();

clock_t start_t, end_t;
DWORD* pBuffer;									// 显存指针
int eaten = 0;									// 吃 AI 的数量
int ai_eaten = 0;									// AI 吃 AI的数量
float asp = 1;									// 缩放因子
float Time = 0;									// 时间
int total_t;
int speed;

int main() {
	starttime();

	initgraph(WIDTH, HEIGHT);
	start();
	ChooseSpeed();
	setall();
	BeginBatchDraw();

	while (true) {
		move(&mover);
		AI();
		Food();
		draw();
		FlushBatchDraw();							// 显示缓存的绘制内容
		Sleep(1);
	}
}

void ChooseSpeed() {
	switch (getch()) {
	case 1:speed = 4;
	case 2:speed = 3;
	case 3:speed = 2;
	default:speed = 4;
	}
}

int readCount() {
	FILE *fp;
	int count;
	if ((fp = fopen(FilePath, "r")) == NULL) {
		return 0;
	}
	else
		fscanf(fp, "%d", &count);
	fclose(fp);
	return count;
}

void writeCount(int count) {
	FILE *fp;
	if ((fp = fopen(FilePath, "w")) == NULL) {
		printf("无法创建数据文件:counter.txt。\n");
		return;
	}
	else
		fprintf(fp, "%d", count);
	fclose(fp);
}

int readTime() {
	FILE *fp1;
	int time;
	if ((fp1 = fopen(FilePath1, "r")) == NULL) {
		return 0;
	}
	else
		fscanf(fp1, "%d", &time);
	fclose(fp1);
	return time;
}

void writeTime(int time) {
	FILE *fp1;
	if ((fp1 = fopen(FilePath1, "w")) == NULL) {
		printf("无法创建数据文件：timeer.txt。\n");
		return;
	}
	else
		fprintf(fp1, "%d", time);
	fclose(fp1);
}

void starttime() {
	start_t = clock();
	writeCount(readCount() + 1);
}

void endtime() {
	closegraph();
	initgraph(WIDTH, HEIGHT);
	cleardevice();
	BeginBatchDraw();
	setbkcolor(WHITE);							// 白色背景
	cleardevice();								// 初始化背景
	settextcolor(BLACK);							// 改字体
	setbkmode(TRANSPARENT);
	end_t = clock();
	total_t = (end_t - start_t);
	IMAGE image;
	loadimage(&image, _T("../resourse/start.jpg"), WIDTH, HEIGHT);
	putimage(0, 0, &image);
	settextstyle(50, 0, _T("宋体"));
	setlinestyle(PS_NULL);
	TCHAR str[64];
	swprintf_s(str, _T("本次游戏时间：%d分%d秒"), total_t / 60000, total_t / 1000 - total_t / 60000 * 60);
	settextcolor(WHITE);							// 改字体
	outtextxy(20, 100, str);
	total_t = readTime() + total_t;
	writeTime(total_t);
	TCHAR str1[64];
	swprintf_s(str1, _T("您已累计游戏%d分%d秒"), total_t / 60000, total_t / 1000 - total_t / 60000 * 60);
	settextcolor(WHITE);							// 改字体
	outtextxy(20, 20, str1);
	settextstyle(20, 0, _T("宋体"));
	outtextxy(384, 550, _T("按任意键退出游戏"));
	FlushBatchDraw();
	getchar();
	closegraph();
	exit(1);		//考虑增加重新开始游戏
}



void move(BALL* ball) {
	if (ball->r <= 0)
		ball->life = false;
	if (ball->life == false) {						// 判定游戏是否接束
		HWND hwnd = GetHWnd();
		MessageBox(hwnd, _T("你被吃了"), _T("游戏结束"), MB_ICONEXCLAMATION);
		endtime();

	}

	if (eaten + ai_eaten == AINUM)					// 是否吃掉所有 AI
	{
		HWND hwnd = GetHWnd();
		MessageBox(hwnd, _T("恭喜过关"), _T("游戏结束"), MB_OK | MB_ICONEXCLAMATION);	// 结束
		endtime();
	}

	for (int i = 0; i < AINUM; i++) {				// 玩家吃 AI 判定
		if (ball->r >= ai[i].r) {
			if (ai[i].life == 0)	continue;
			if (DISTANCE(ball->x, ball->y, ai[i].x, ai[i].y) < (4 / 5.0 * (ball->r + ai[i].r))) {
				ai[i].life = 0;					//AI被吃
				ball->r = sqrt(ai[i].r*ai[i].r + ball->r*ball->r);
				eaten++;
			}
		}
	}

	for (int n = 0; n < FNUM; n++) {				// 玩家吃食物
		if (food[n].eat == 0)	continue;
		if (DISTANCE(ball->x, ball->y, food[n].x, food[n].y) < ball->r) {
			ball->r += 4 / ball->r;				// 增加面积
			food[n].eat = 0;						// 食物被吃
		}
	}

	static int mx = 0, my = 0;						// 记录偏移量

	if (GetAsyncKeyState(VK_UP) && (ball->y - ball->r > 0 && ball->y <= (MAPH - ball->r + 10))) {
		ball->y -= speed;
		my += speed;
	}
	if (GetAsyncKeyState(VK_DOWN) && (ball->y - ball->r >= -10 && ball->y < (MAPH - ball->r))) {
		ball->y += speed;
		my -= speed;
	}
	if (GetAsyncKeyState(VK_LEFT) && ball->x - ball->r > 0 && (ball->x <= (MAPW - ball->r + 10))) {
		ball->x -= speed;
		mx += speed;
	}
	if (GetAsyncKeyState(VK_RIGHT) && ball->x - ball->r >= -10 && (ball->x < (MAPW - ball->r))) {
		ball->x += speed;
		mx -= speed;
	}


	if (GetAsyncKeyState(VK_SPACE)) {
		settextcolor(WHITE);
		settextstyle(32, 0, _T("宋体"));
		outtextxy(384 - mx, 350 - my, _T("游戏已暂停！"));
		outtextxy(20 - mx, 500 - my, _T("（ESC）退出"));
		outtextxy(780 - mx, 500 - my, _T("（回车键）继续"));
		FlushBatchDraw();
		getch();
		if (GetAsyncKeyState(VK_ESCAPE))
			exit(0);
		else
			getch();
	}

	setorigin(mx, my);							//坐标修正
}


void AI() {
	for (int i = 0; i < AINUM; i++) {				// AI 吃玩家
		if (ai[i].r > mover.r) {
			if (DISTANCE(mover.x, mover.y, ai[i].x, ai[i].y) < (ai[i].r + mover.r)) {
				ai[i].r = sqrt(ai[i].r*ai[i].r + mover.r*mover.r);
				mover.life = 0;
				mover.r = 0;
			}
		}
		for (int j = 0; j < AINUM; j++) {			// AI 吃 AI
			if (ai[i].r > ai[j].r) {
				if (ai[j].life == 0) continue;
				if (DISTANCE(ai[i].x, ai[i].y, ai[j].x, ai[j].y) < (ai[i].r + ai[j].r)) {
					ai[i].r = sqrt(ai[i].r*ai[i].r + ai[j].r*ai[j].r);
					ai[j].life = 0;
					ai[j].r = 0;
					ai_eaten++;
				}
			}
		}

		double min_DISTANCE = 100000;
		int min = -1;
		for (int k = 0; k < AINUM; k++) {			// AI 靠近 AI
			if (ai[i].r > ai[k].r&&ai[k].life != 0) {
				if (DISTANCE(ai[i].x, ai[i].y, ai[k].x, ai[k].y) < min_DISTANCE) {
					min_DISTANCE = DISTANCE(ai[i].x, ai[i].y, ai[k].x, ai[k].y);
					min = k;
				}
			}
		}
		if ((min != -1) && (rand() % 2 == 1)) {
			if (rand() % 2) {
				if (ai[i].x < ai[min].x)
					ai[i].x++;
				else
					ai[i].x--;
			}
			else {
				if (ai[i].y < ai[min].y)
					ai[i].y++;
				else
					ai[i].y--;
			}
		}
		for (int n = 0; n < FNUM; n++) {			// AI 吃食物
			if (food[n].eat == 0) continue;
			if (DISTANCE(ai[i].x, ai[i].y, food[n].x, food[n].y) < ai[i].r) {
				ai[i].r += 4 / ai[i].r;
				food[n].eat = 0;
			}
		}
	}
}

void Food() {
	for (int i = 0; i < FNUM; i++) {				// 食物刷新
		if (food[i].eat == 0) {
			food[i].eat = 1;
			food[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);
			food[i].x = rand() % MAPW;
			food[i].y = rand() % MAPH;
			food[i].type = rand() % 10 + 1;
		}
	}
}

void draw() {
	clearcliprgn();
	IMAGE image;
	loadimage(&image, _T("../resourse/background.jpg"), WIDTH*4, HEIGHT*4);
	putimage(0, 0, &image);
	setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 20);		// 改变笔的颜色、状态
	setlinecolor(RGB(0, 100, 0));
	line(-20, MAPH + 20, -20, -20);				// 左竖
	line(-20, MAPH + 20, MAPW + 20, MAPH + 20);		// 上横
	line(-20, -20, MAPW + 20, -20);				// 下横
	line(MAPW + 20, -20, MAPW + 20, MAPH + 20);		// 右竖
	setfillcolor(GREEN);

	if (mover.x - 0.5 * WIDTH / asp < -20)
		floodfill(-20 - 11, mover.y, RGB(0, 100, 0));
	if (mover.x + 0.5 * WIDTH / asp > MAPW + 20)
		floodfill(MAPW + 20 + 11, mover.y, RGB(0, 100, 0));
	if (mover.y - 0.5 * HEIGHT / asp < -20)
		floodfill(mover.x, -20 - 11, RGB(0, 100, 0));
	if (mover.y + 0.5 * HEIGHT / asp > MAPH + 20)
		floodfill(mover.x, MAPH + 20 + 11, RGB(0, 100, 0));

	setlinecolor(WHITE);
	setlinestyle(PS_NULL);

	for (int i = 0; i < FNUM; i++) {				// 画出食物
		if (food[i].eat == 0) continue;
		setfillcolor(food[i].color);
		switch (food[i].type) {					// 形状
		case 1:		solidellipse(food[i].x, food[i].y, food[i].x + 2, food[i].y + 4); break;
		case 2:		solidellipse(food[i].x, food[i].y, food[i].x + 4, food[i].y + 2);	break;
		case 3:		solidrectangle(food[i].x, food[i].y, food[i].x + 4, food[i].y + 2); break;
		case 4:		solidrectangle(food[i].x, food[i].y, food[i].x + 2, food[i].y + 4); break;
		case 5:		solidroundrect(food[i].x, food[i].y, food[i].x + 2, food[i].y + 4, 2, 2); break;
		case 6:		solidroundrect(food[i].x, food[i].y, food[i].x + 4, food[i].y + 2, 2, 2); break;
		case 7:		solidroundrect(food[i].x, food[i].y, food[i].x + 4, food[i].y + 2, 4, 2); break;
		case 8:		solidroundrect(food[i].x, food[i].y, food[i].x + 4, food[i].y + 2, 2, 4); break;
		case 9:		solidroundrect(food[i].x, food[i].y, food[i].x + 4, food[i].y + 2, 1, 1); break;
		case 10:	fillcircle(food[i].x, food[i].y, 4); break;
		}
	}

	for (int i = 0; i < AINUM; i++) {				// 画 AI
		if (ai[i].life == 0) continue;
		setfillcolor(ai[i].color);
		fillcircle(ai[i].x, ai[i].y, int(ai[i].r + 0.5));
	}

	setfillcolor(mover.color);						// 画玩家
	fillcircle(mover.x, mover.y, int(mover.r + 0.5));

	IMAGE map(150, 100);							// 小地图
	SetWorkingImage(&map);
	setbkcolor(RGB(120, 165, 209));					// 浅灰色背景
	cleardevice();
	for (int i = 0; i < AINUM; i++)				// 画 AI（小地图）
	{
		if (ai[i].life == 0) continue;
		setfillcolor(ai[i].color);
		fillcircle(ai[i].x * 150 / WIDTH / 4, ai[i].y * 100 / HEIGHT / 4, int(ai[i].r / 28 + 1.5));
	}

	setfillcolor(mover.color);						// 画玩家（小地图）
	fillcircle(mover.x * 150 / WIDTH / 4, mover.y * 100 / HEIGHT / 4, int(mover.r / 28 + 3.5));
	setlinecolor(RGB(0, 100, 0));

	SetWorkingImage();							// 恢复绘图背景
	putimage(mover.x + int(0.5 * WIDTH) - 150, mover.y - int(0.5 * HEIGHT), 150, 100, &map, 0, 0);						// 画出小地图
	setlinecolor(LIGHTBLUE);
	setlinestyle(PS_SOLID | PS_JOIN_BEVEL, 4);
	line(mover.x + int(0.5 * WIDTH) - 151, mover.y - int(0.5 * HEIGHT), mover.x + int(0.5 * WIDTH) - 151, mover.y - int(0.5 * HEIGHT) + 99);	// 地图边框线
	line(mover.x + int(0.5 * WIDTH) - 151, mover.y - int(0.5 * HEIGHT) + 99, mover.x + int(0.5 * WIDTH), mover.y - int(0.5 * HEIGHT) + 99);	// 地图边框线

	setlinestyle(PS_NULL);							// 恢复笔
	TCHAR str[32];
	swprintf_s(str, _T("质量:%.1fg  击杀:%d"), mover.r, eaten);
	settextcolor(WHITE);							// 改字体
	outtextxy(mover.x - int(0.5 * WIDTH), mover.y - int(0.5 * HEIGHT), str);
	settextcolor(WHITE);
	outtextxy(mover.x-20, mover.y , _T("user"));
}

void setall() {
	srand((unsigned)time(NULL));					// 随机数
	mover.color = RGB(rand() % 256, rand() % 256, rand() % 256);		// 随机颜色
	mover.life = 1;								// 赋初值1
	mover.x = int(WIDTH*0.5);
	mover.y = int(HEIGHT*0.5);
	mover.r = 20;

	for (int i = 0; i < AINUM; i++) {				// AI 的属性
		ai[i].life = 1;
		ai[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);
		ai[i].r = float(rand() % 10 + 10);
		ai[i].x = rand() % (MAPW - int(ai[i].r + 0.5)) + int(ai[i].r + 0.5);
		ai[i].y = rand() % (MAPH - int(ai[i].r + 0.5)) + int(ai[i].r + 0.5);
	}

	for (int i = 0; i < FNUM; i++) {				// 食物的属性
		food[i].eat = 1;
		food[i].color = RGB(rand() % 256, rand() % 256, rand() % 256);
		food[i].x = rand() % MAPW;
		food[i].y = rand() % MAPH;
		food[i].type = rand() % 10 + 1;
	}

	pBuffer = GetImageBuffer(NULL);					// 获取显存指针
	setbkcolor(WHITE);							// 白色背景
	cleardevice();								// 初始化背景
	settextcolor(LIGHTRED);						// 改字体
	setbkmode(TRANSPARENT);
	settextstyle(16, 0, _T("宋体"));

}

void start() {
	setbkcolor(WHITE);							// 白色背景
	cleardevice();								// 初始化背景
	settextcolor(BLACK);							// 改字体
	setbkmode(TRANSPARENT);
	IMAGE image;
	loadimage(&image, _T("../resourse/start.jpg"), WIDTH, HEIGHT);
	putimage(0, 0, &image);
	settextcolor(WHITE);
	settextstyle(22, 0, _T("黑体"));
	//outtextxy(580, 380, _T("请选择关卡："));
	outtextxy(50, 550, _T("1.小试牛刀"));
	outtextxy(200, 550, _T("2.炉火纯青"));
	outtextxy(350, 550, _T("3.登峰造极"));
	settextstyle(15, 0, _T("宋体"));
	outtextxy(600, 550, _T("注：按序号选择，默认选择关卡1;游戏中按空格键可以暂停。"));
	settextcolor(BLACK);
	settextstyle(22, 0, _T("黑体"));
	TCHAR s[5];
	_stprintf(s, _T("%d"), readCount());
	outtextxy(810, 200, _T("欢迎进入游戏！"));
	outtextxy(810, 240, _T("游戏次数："));
	outtextxy(910, 240, s);
}