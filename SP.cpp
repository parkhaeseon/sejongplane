#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include "blockModel.h"

#define PI 3.14159
#define GBOARD_WIDTH 45 //맵 가로 세로 크기
#define GBOARD_HEIGHT 27
#define START_CURPOS_X 30 //유저 시작 좌표
#define START_CURPOS_Y 20
#define BOSS_CURPOS_X 32 //보스 등장 좌표
#define BOSS_CURPOS_Y 2

typedef struct bullet //총알 구조체 (이중 연결리스트로 쓸꺼라 포인터로 따로 정의)
{
	float x; //좌표
	float y;
	struct bullet *next; //다음과 이전 총알
	struct bullet *prev;
	float dir_x; //방향
	float dir_y;
}Bullet, *pBullet;
typedef struct user //PC, 유저 구조체
{
	int x; //좌표
	int y;
	int Damage; //데미지
	int HP; //체력, 라이프
	pBullet bul; //총알
	int bomb; //폭탄, 필살기
	int speed;
}PC;
typedef struct boss //보스 구조체
{
	bool exist; //존재 유무
	int x; //좌표
	int y;
	int HP; //체력
	int Damage; //데미지
	pBullet bul; //총알
	int speed;
	int bul_speed;
}Boss;
typedef struct follower //쫄따구들
{
	bool exist;
	int HP;
	int Damage;
	int x;
	int y;
	pBullet bul;
	int speed;
	int bul_speed;
}Follower;
typedef struct item //아이템 구조체
{
	int x; //좌표
	int y;
	int id; //종류
}Item;

void SetCurrentCursorPos(int x, int y); //커서 좌표 설정하기
void RemoveCursor(void); //커서(밑줄 깜빡이) 끄기

int FirstScreenAfterExecution(); //첫 화면
void DrawGameBoard(); //맵 그리기
void DrawInfo(); //게임 화면 좌측에 점수 및 진행 상태 표시
int ShowCountBeforeStart(); // 3,2,1 보여주기

void ShiftRight(); //유저 상하좌우 이동
void ShiftLeft();
void ShiftUp();
void ShiftDown();

void Boss_ShiftLeft(); //보스 좌우 이동
void Boss_ShiftRight();
void Boss_Move(); //보스 행동 총괄(이후 총쏘는 것도 여기에 추가) 유저의 ProcessKeyInput과 같다고 생각하면 됨

void Follower_ShiftLeft(int id, int i);
void Follower_ShiftRight(int id, int i);
void Follower_ShiftDown(int id, int i);
void Follower_Shoot(int id, int i);
void Follower_Move();
void Follower_Create();

void PC_Bullet_Move(); //유저 총알 이동 총괄
void Boss_Bullet_Move(); //보스 총알 이동 총괄
void Follow_Bullet_Move();

void ShowPC(char blockInfo[4][4]); //유저 그리기 및 지우기
void DeletePC(char blockInfo[4][4]);
void ShowBoss(char blockInfo[4][4]); //보스 그리기 및 지우기
void DeleteBoss(char blockInfo[4][4]);
void ShowFollow(int id, int i); //부하 그리기 및 지우기
void DeleteFollow(int id, int i);

void ShowPCBullet(); //유저 총알 그리기 및 지우기
void DeletePCBullet();
void ShowBossBullet();
void DeleteBossBullet();
void ShowFollowBullet(int id, int i);
void DeleteFollowBullet(int id, int i);

int PC_DetectCollision(int posX, int posY, char blockModel[][4]); //유저 충돌 관련 총괄
int Boss_DetectCollision(int posX, int posY, char blockModel[][4]); //보스 충돌 관련 총괄
int Follower_DetectCollision(int id, int i, int posX, int posY);

int PC_Bullet_Detect(pBullet pb); //유저 총알 충돌 관련 총괄
int Boss_Bullet_Detect(pBullet pb); //보스 총알 충돌 관련 총괄
int Follow_Bullet_Detect(pBullet pb);

int MainStageClear();

void PC_Init(); //유저 정보 초기화
void Boss_Init(); //보스 정보 초기화
void Follow_Init(int id); //부하 정보 초기화
void Init(); //게임 초기화

void test(); //오류 날 시 테스트용

int MainStageClear(); //스테이지 및 게임 클리어 검사
void ItemCreate(); //아이템 생성
void ItemApply(int id); //아이템 적용

void ShowItem(); //아이템 그리기 및 지우기
void DeleteItem();

int show_gamewin(); //클리어나 게임오버 그리기
int show_gameover();

PC pc; //유저 구조체 생성
Boss boss[3]; //보스 구조체 배열 생성(3스테이지)
Follower f[3][8]; //한번에 8마리 보여줘야함
Item it[4]; //아이템 구조체 배열 생성(4가지)

int block_id = 1; //유저 블록 id
int Boss_block_id = 2; //보스 블록 id

int bomb_y = 100; //폭탄 위치 (안 쐈을땐 100)

int term; //부하 생성 관련 주기 변수
int sleep = 50; //슬립 변수 1000=1초(총알 속도 기준)
int gameBoardInfo[GBOARD_HEIGHT + 1][GBOARD_WIDTH + 2] = { 0, }; //맵 행렬

int main_stage; //메인 스테이지 (보스 클리어 관련)

bool clear = false; //게임 클리어 확인 변수
int stage_clear = 0; //스테이지 클리어 확인 변수(-1이 클리어)

bool reverseMode = false; //방향키 거꾸로 클리어 확인 변수

bool blockMode = false;
int blockTime = -1;

bool nodieMode = false;
int nodieTime = -1;

void SetCurrentCursorPos(int x, int y)
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}
void RemoveCursor(void)
{
	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

int FirstScreenAfterExecution()
{
	int i, logo_delay = 5;

	SetCurrentCursorPos(4, 2);

	for (i = 4; i<76; i++)
	{
		printf("*");
		Sleep(logo_delay);
	}

	for (i = 3; i<8; i++)
	{
		SetCurrentCursorPos(75, i); printf("*");
		Sleep(logo_delay);
	}

	for (i = 75; i >= 4; i--)
	{
		SetCurrentCursorPos(i, 7);
		printf("*");
		Sleep(logo_delay);
	}

	for (i = 7; i >= 3; i--)
	{
		SetCurrentCursorPos(4, i); printf("*");
		Sleep(logo_delay);
	}

	Sleep(80);

	SetCurrentCursorPos(32, 4);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // 색 바꾸고
	printf("☆ Sejong Plane  ☆");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); // 다시 하얀색으로.



	Sleep(100);

	SetCurrentCursorPos(6, 6);

	printf("SW 설계 기초 1조");

	Sleep(100);

	SetCurrentCursorPos(44, 6);

	printf("박해선, 서준석, 윤병학, 강산하");

	SetCurrentCursorPos(37, 15);

	ShowPC(blockModel[block_id]); // 시작할 때 비행기 모양 보여주기.

	Sleep(500);

	SetCurrentCursorPos(30, 21);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10); // 색 바꾸고

	printf("Please Press Enter Key!");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); // 다시 원래대로

	return 0;
}
void DrawGameBoard()
{
	int x, y, i = 0;

	for (y = 0; y<GBOARD_HEIGHT; y++)
	{
		gameBoardInfo[y][0] = 1;
		gameBoardInfo[y][GBOARD_WIDTH + 1] = 1;
		gameBoardInfo[y][35] = 1;
	}

	for (x = 0; x<GBOARD_WIDTH + 2; x++)
	{
		gameBoardInfo[GBOARD_HEIGHT][x] = 1;
	}

	for (x = 0; x<GBOARD_WIDTH + 2; x++)
	{
		gameBoardInfo[0][x] = 1;

		if (x >= 35)
		{
			gameBoardInfo[6][x] = 1;
			gameBoardInfo[12][x] = 1;
			gameBoardInfo[18][x] = 1;
			gameBoardInfo[24][x] = 1;

		}

	}

	for (y = 0; y <= GBOARD_HEIGHT; y++)
	{
		for (x = 0; x<GBOARD_WIDTH + 2; x++)
		{
			if (gameBoardInfo[y][x] == 1)
			{
				SetCurrentCursorPos(x * 2, y);
				printf("■");
			}
		}
	}





}
void DrawInfo()
{
	int i = 0;

	SetCurrentCursorPos(73, 2);
	printf("    [Now Stage]\n");

	SetCurrentCursorPos(73, 4);
	printf("     %d Stage", main_stage);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14); // 색 바꾸고

	SetCurrentCursorPos(73, 8);
	printf("    [Boss HP]\n");

	SetCurrentCursorPos(73, 10);
	printf("     %3d", boss[main_stage - 1].HP);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //다시 원래 색으로

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // 색 바꾸고

	SetCurrentCursorPos(73, 14);
	printf("     [My HP]\n");

	if (nodieMode)
		{
			SetCurrentCursorPos(73, 16);
			printf("    무적 상태");

		}
	else
	{
		SetCurrentCursorPos(73, 16);
		printf("    ");
		for (i = 0; i<8; i++)
		{
			printf(" ");
		}
		SetCurrentCursorPos(73, 16);

		printf("                 ");
		SetCurrentCursorPos(77, 16);
		for (i = 0; i < pc.HP; i++)
		{
			printf("♥");
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //다시 원래 색으로


	SetCurrentCursorPos(73, 20);
	printf("   [Bomb Count]\n");

	SetCurrentCursorPos(73, 22);
	printf("    ");
	for (i = 0; i<5; i++)
	{
		printf(" ");
	}
	SetCurrentCursorPos(73, 22);
	printf("    ");
	for (i = 0; i < pc.bomb; i++)
	{
		printf("ⓑ");
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //다시 원래 색으로

	SetCurrentCursorPos(73, 25);
	printf("Space : Bullet");

	SetCurrentCursorPos(73, 26);
	printf("z : Bomb");
}
int ShowCountBeforeStart()
{
	int i, j, k;

	SetCurrentCursorPos(17, 5);
	printf("┏━━━━━━━━━━┓");
	SetCurrentCursorPos(17, 6);
	printf("┃       ■■■       ┃");
	SetCurrentCursorPos(17, 7);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 8);
	printf("┃             ■     ┃");
	SetCurrentCursorPos(17, 9);
	printf("┃             ■     ┃");
	SetCurrentCursorPos(17, 10);
	printf("┃         ■■       ┃");
	SetCurrentCursorPos(17, 11);
	printf("┃             ■     ┃");
	SetCurrentCursorPos(17, 12);
	printf("┃    ■        ■    ┃");
	SetCurrentCursorPos(17, 13);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 14);
	printf("┃       ■■■       ┃");
	SetCurrentCursorPos(17, 15);
	printf("┗━━━━━━━━━━┛");

	SetCurrentCursorPos(0, 23);
	Sleep(1000);

	SetCurrentCursorPos(17, 5);
	printf("┏━━━━━━━━━━┓");
	SetCurrentCursorPos(17, 6);
	printf("┃       ■■■       ┃");
	SetCurrentCursorPos(17, 7);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 8);
	printf("┃             ■     ┃");
	SetCurrentCursorPos(17, 9);
	printf("┃            ■      ┃");
	SetCurrentCursorPos(17, 10);
	printf("┃          ■        ┃");
	SetCurrentCursorPos(17, 11);
	printf("┃        ■          ┃");
	SetCurrentCursorPos(17, 12);
	printf("┃      ■            ┃");
	SetCurrentCursorPos(17, 13);
	printf("┃    ■              ┃");
	SetCurrentCursorPos(17, 14);
	printf("┃    ■■■■■■    ┃");
	SetCurrentCursorPos(17, 15);
	printf("┗━━━━━━━━━━┛");

	SetCurrentCursorPos(0, 23);
	Sleep(1000);

	SetCurrentCursorPos(17, 5);
	printf("┏━━━━━━━━━━┓");
	SetCurrentCursorPos(17, 6);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 7);
	printf("┃       ■■         ┃");
	SetCurrentCursorPos(17, 8);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 9);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 10);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 11);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 12);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 13);
	printf("┃         ■         ┃");
	SetCurrentCursorPos(17, 14);
	printf("┃       ■■■       ┃");
	SetCurrentCursorPos(17, 15);
	printf("┗━━━━━━━━━━┛");

	SetCurrentCursorPos(0, 23);
	Sleep(1000);

	SetCurrentCursorPos(17, 5);
	printf("┏━━━━━━━━━━┓");
	SetCurrentCursorPos(17, 6);
	printf("┃       ■■■       ┃");
	SetCurrentCursorPos(17, 7);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 8);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 9);
	printf("┃     ■    ■■     ┃");
	SetCurrentCursorPos(17, 10);
	printf("┃     ■  ■  ■     ┃");
	SetCurrentCursorPos(17, 11);
	printf("┃     ■■    ■     ┃");
	SetCurrentCursorPos(17, 12);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 13);
	printf("┃     ■      ■     ┃");
	SetCurrentCursorPos(17, 14);
	printf("┃       ■■■       ┃");
	SetCurrentCursorPos(17, 15);
	printf("┗━━━━━━━━━━┛");


	for (j = 17, k = 40; j <= 41; j++, k--)
	{
		for (i = 5; i<17; i += 2)
		{
			SetCurrentCursorPos(j - 1, i);

			printf("  ");

			if (j != 41)
			{
				printf("*");
			}

			SetCurrentCursorPos(k, i + 1);

			if (k != 16)
			{
				printf("*");
			}

			printf("  ");

		}

		Sleep(80);
	}

	return 0;
}

int PC_DetectCollision(int posX, int posY, char blockModel[][4])
{
	int x, y;

	for (x = 0; x<4; x++)
	{
		for (y = 0; y < 4; y++)
		{
			if (blockModel[y][x] != 0 && gameBoardInfo[posY + y][posX / 2 + x] == 1)
				return 0; //유저가 맵이랑 박았을 때

			else if (stage_clear == -1) //아이템이랑 박았을 때
			{
				for (int i = 0; i<4; i++)
				{
					if (blockModel[y][x] != 0 && posX + 2 * x == it[i].x && posY + y == it[i].y)
					{
						ItemApply(i); //해당 아이템 적용하고 위치만 초기화
						DeletePC(blockModel);
						pc.x = START_CURPOS_X;
						pc.y = START_CURPOS_Y;
						ShowPC(blockModel);

						main_stage++; term = 0;

						return 0;
					}
				}
			}
		}
	}

	return 1;
}
int Boss_DetectCollision(int posX, int posY, char blockModel[][4]) //아직 ㄴㄴ
{
	int x, y;

	for (x = 0; x <4; x++)
	{
		for (y = 0; y <4; y++)
		{
			if (blockModel[y][x] != 0 && gameBoardInfo[posY + y][posX / 2 + x] == 1)
				return 0; //보스가 맵이랑 박았을 때
			if (blockModel[y][x] != 0 && (posX - 6 < pc.x && pc.x < posX + 9 && pc.y<posY + 3))
			{
				pc.HP = 0;
			//	if (reverseMode)
			//		reverseMode = false;
				return 0; //보스가 유저랑 박았을때
			}
		}
	}

	return 1;
}
int Follower_DetectCollision(int id, int i, int posX, int posY)
{
	if (gameBoardInfo[posY][posX / 2] == 1)
		return 0; //부하가 맵이랑 박았을 때
	for (int x = 0; x<4; x++) //유저랑 박을 때
	{
		for (int y = 0; y<4; y++)
		{
			if (blockModel[block_id][y][x] != 0 && pc.x + 2 * x == f[id][i].x && pc.y + y == f[id][i].y)
			{
				if (!nodieMode)
					pc.HP -= f[id][i].Damage;
				f[id][i].exist = false;
				DeleteFollow(id, i);
				return 0;
			}
		}
	}

	return 1;
}

void Boss_ShiftLeft()
{
	if (!Boss_DetectCollision(boss[main_stage - 1].x - 4, boss[main_stage - 1].y, blockModel[Boss_block_id]))
		return;

	DeleteBoss(blockModel[Boss_block_id]);
	boss[main_stage - 1].x -= 4;

	ShowBoss(blockModel[Boss_block_id]);
}
void Boss_ShiftRight()
{
	if (!Boss_DetectCollision(boss[main_stage - 1].x + 4, boss[main_stage - 1].y, blockModel[Boss_block_id]))
		return;

	DeleteBoss(blockModel[Boss_block_id]);
	boss[main_stage - 1].x += 4;

	ShowBoss(blockModel[Boss_block_id]);
}
void Boss_ShiftUp()
{
	if (!Boss_DetectCollision(boss[main_stage - 1].x, boss[main_stage - 1].y - 1, blockModel[Boss_block_id]))
		return;

	DeleteBoss(blockModel[Boss_block_id]);
	boss[main_stage - 1].y -= 1;

	ShowBoss(blockModel[Boss_block_id]);
}
void Boss_ShiftDown()
{
	if (!Boss_DetectCollision(boss[main_stage - 1].x, boss[main_stage - 1].y + 1, blockModel[Boss_block_id]))
		return;

	DeleteBoss(blockModel[Boss_block_id]);
	boss[main_stage - 1].y += 1;

	ShowBoss(blockModel[Boss_block_id]);
}
void Boss_Shoot()
{
	for (int i = 0; i<3; i++)
	{
		pBullet pb; //새로운 총알 생성(빈 노드 생성과 동일)
		pb = (pBullet)malloc(sizeof(Bullet));
		pb->x = boss[main_stage - 1].x + 2 + i;
		pb->y = boss[main_stage - 1].y + 3;
		pb->dir_x = 0;
		pb->dir_y = 1;
		pb->next = NULL;
		pb->prev = NULL;

		if (boss[main_stage - 1].bul == NULL) //존재하는 총알이 없을 시
			boss[main_stage - 1].bul = pb;
		else //있을 시
		{
			for (pBullet p = boss[main_stage - 1].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //마지막 다음으로 연결
				{
					p->next = pb;
					pb->prev = p;
					break;
				}
			}
		}
	}
}
void Boss_Shoot_Veer()
{
	int bul_num = 8; //총알 갯수
	double angle = 180 / (bul_num + 1); 
	for (int i = 0; i<bul_num; i++)
	{
		pBullet pb; //새로운 총알 생성(빈 노드 생성과 동일)
		pb = (pBullet)malloc(sizeof(Bullet));
		pb->x = boss[main_stage - 1].x + 2;
		pb->y = boss[main_stage - 1].y + 3;
		double rad = angle*(i + 1)*(PI / 180); // c언어에서는 cos, sin을 라디안으로 계산.
		pb->dir_x = (float)cos(rad);
		pb->dir_y = (float)sin(rad);
		pb->next = NULL;
		pb->prev = NULL;

		if (boss[main_stage - 1].bul == NULL) //존재하는 총알이 없을 시
			boss[main_stage - 1].bul = pb;
		else //있을 시
		{
			for (pBullet p = boss[main_stage - 1].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //마지막 다음으로 연결
				{
					p->next = pb;
					pb->prev = p;

					break;
				}
			}
		}
	}
}
void Boss_Move()
{
	if (boss[main_stage - 1].y + 3 == bomb_y) //폭탄 충돌 검사
		boss[main_stage - 1].HP -= 10;
	if (boss[main_stage - 1].speed == 201) //속도 초기화
		boss[main_stage - 1].speed = 0;
	boss[main_stage - 1].speed++;
	if (main_stage == 3 && 160<boss[main_stage - 1].speed && boss[main_stage - 1].speed <= 180) //돌진
		Boss_ShiftDown();
	else if (main_stage == 3 && 180<boss[main_stage - 1].speed && boss[main_stage - 1].speed <= 200) //돌진 후 돌아오기
		Boss_ShiftUp();
	else if (main_stage == 1 && boss[main_stage - 1].speed % 100 == 0) //부채꼴 발사 (스테이지 1이면 한번, 아니면 내가 지정해준데로)
		Boss_Shoot_Veer();
	else if (main_stage != 1 && 90<boss[main_stage - 1].speed && boss[main_stage - 1].speed <= 110)
		Boss_Shoot_Veer();
	else if (boss[main_stage - 1].speed % 20 == 0) // 총알 발사
		Boss_Shoot();
	else if (boss[main_stage - 1].speed % 5 == 0)
	{
		int BossShiftVar; //보스 좌우 랜덤
		BossShiftVar = rand() % 2;
		switch (BossShiftVar)
		{
		case 0:
			Boss_ShiftLeft(); break;
		case 1:
			Boss_ShiftRight(); break;
		}
	}
}

void Follower_ShiftLeft(int id, int i)
{
	if (!Follower_DetectCollision(id, i, f[id][i].x - 2, f[id][i].y))
		return;
	DeleteFollow(id, i);
	f[id][i].x -= 2;
	ShowFollow(id, i);
}
void Follower_ShiftRight(int id, int i)
{
	if (!Follower_DetectCollision(id, i, f[id][i].x + 2, f[id][i].y))
		return;
	DeleteFollow(id, i);
	f[id][i].x += 2;
	ShowFollow(id, i);
}
void Follower_ShiftDown(int id, int i)
{
	if (!Follower_DetectCollision(id, i, f[id][i].x, f[id][i].y + 1))
		return;
	DeleteFollow(id, i);
	f[id][i].y += 1;
	ShowFollow(id, i);
}
void Follower_Shoot(int id, int i)
{
	pBullet pb; //새로운 총알 생성(빈 노드 생성과 동일)
	pb = (pBullet)malloc(sizeof(Bullet));
	pb->next = NULL;
	pb->prev = NULL;
	switch (id)
	{
	case 0: // 부하1
	case 2: // 부하3
		pb->x = f[id][i].x;
		pb->y = f[id][i].y + 1;
		if (pb->y>25)
			return;
		pb->dir_x = 0;
		pb->dir_y = 1;

		if (f[id][i].bul == NULL) //존재하는 총알이 없을 시
			f[id][i].bul = pb;
		else //있을 시
		{
			for (pBullet p = f[id][i].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //마지막 다음으로 연결
				{
					p->next = pb;
					pb->prev = p;
					break;
				}
			}
		}
		break;
	case 1: // 부하2
		pb->x = f[id][i].x;
		pb->y = f[id][i].y;
		int diff_x = (pc.x + 2 - f[id][i].x) / 2;
		int diff_y = pc.y + 1 - f[id][i].y;
		int larger = (abs(diff_x)>abs(diff_y)) ? abs(diff_x) : abs(diff_y);
		pb->dir_x = (float)diff_x / larger;
		pb->dir_y = (float)diff_y / larger;

		if (f[id][i].bul == NULL) //존재하는 총알이 없을 시
			f[id][i].bul = pb;
		else //있을 시
		{
			for (pBullet p = f[id][i].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //마지막 다음으로 연결
				{
					p->next = pb;
					pb->prev = p;
					break;
				}
			}
		}
		break;
	}
}
void Follower_Move()
{
	for (int id = 0; id<3; id++)
	{
		for (int i = 0; i<8; i++)
		{
			if (!f[id][i].exist)
				continue;
			if (id != 2 && f[id][i].y >= bomb_y)
			{
				f[id][i].exist = false;
				DeleteFollow(id, i);
				continue;
			}

			f[id][i].speed++;
			if (id == 2) //부하3만(예외 케이스)
			{
				if (f[id][i].speed <= 10 && f[id][i].speed % 2 == 0)
					Follower_Shoot(id, i);
				else if (f[id][i].speed<20 && f[id][i].speed % 2 == 0)
					continue;
				else if (f[id][i].speed == 20)
				{
					DeleteFollow(id, i);
					f[id][i].x = pc.x;
					ShowFollow(id, i);
					f[id][i].speed = 0;
				}
				continue;
			}
			if (f[id][i].speed % 5 == 0) //부하1,2 
			{
				if (id == 0 && (i == 0 || i == 1 || i == 4 || i == 5))
				{
					if (f[id][i].x >= 70 || f[id][i].y >= 26)
					{
						f[id][i].exist = false;
						DeleteFollow(id, i);
						continue;
					}
					Follower_ShiftRight(id, i);
					Follower_ShiftDown(id, i);
				}
				else if (id == 0 && (i == 2 || i == 3 || i == 6 || i == 7))
				{
					if (f[id][i].x <= 2 || f[id][i].y >= 26)
					{
						f[id][i].exist = false;
						DeleteFollow(id, i);
						continue;
					}
					Follower_ShiftLeft(id, i);
					Follower_ShiftDown(id, i);
				}
				else if (id == 1)
				{
					if (f[id][i].y >= 26)
					{
						f[id][i].exist = false;
						DeleteFollow(id, i);
						continue;
					}
					Follower_ShiftDown(id, i);
				}
			}
			if (f[id][i].speed == 20)
			{
				Follower_Shoot(id, i);
				f[id][i].speed = 0;
			}
		}
	}
}
void Follower_Create()
{
	switch (main_stage)
	{
	case 1:
		if (term % 160 == 1)
			Follow_Init(4);
		else if (term % 80 == 1)
			Follow_Init(0);
		if (term % 200 == 0)
			Follow_Init(5);
		else if (term % 100 == 0)
			Follow_Init(1);
		break;
	case 2:
		if (term == 1)
			Follow_Init(2);
		if (term % 160 == 1)
			Follow_Init(4);
		else if (term % 80 == 1)
			Follow_Init(0);
		if (term % 200 == 0)
			Follow_Init(5);
		else if (term % 100 == 0)
			Follow_Init(1);
		break;
	case 3:
		if (term == 1)
			Follow_Init(2);
		if (term % 160 == 1)
			Follow_Init(4);
		else if (term % 80 == 1)
			Follow_Init(0);
		if (term % 160 == 0)
			Follow_Init(5);
		else if (term % 80 == 0)
			Follow_Init(1);
		break;
	}
}

void ShiftRight()
{
	if (!PC_DetectCollision(pc.x + 2, pc.y, blockModel[block_id]))
		return;

	DeletePC(blockModel[block_id]);
	pc.x += 2;

	ShowPC(blockModel[block_id]);
}
void ShiftLeft()
{
	if (!PC_DetectCollision(pc.x - 2, pc.y, blockModel[block_id]))
		return;

	DeletePC(blockModel[block_id]);
	pc.x -= 2;

	ShowPC(blockModel[block_id]);
}
void ShiftUp()
{
	if (!PC_DetectCollision(pc.x, pc.y - 1, blockModel[block_id]))
		return;

	DeletePC(blockModel[block_id]);
	pc.y -= 1;

	ShowPC(blockModel[block_id]);
}
void ShiftDown()
{
	if (!PC_DetectCollision(pc.x, pc.y + 1, blockModel[block_id]))
		return;

	DeletePC(blockModel[block_id]);
	pc.y += 1;

	ShowPC(blockModel[block_id]);
}

void PC_Shoot()
{
	pBullet pb; //새로운 총알 생성(빈 노드 생성과 동일)
	pb = (pBullet)malloc(sizeof(Bullet));
	pb->x = pc.x + 2;
	pb->y = pc.y;
	pb->next = NULL;
	pb->prev = NULL;

	if (pc.bul == NULL) //존재하는 총알이 없을 시
		pc.bul = pb;
	else //있을 시
	{
		for (pBullet p = pc.bul; p != NULL; p = p->next)
		{
			if (p->next == NULL) //마지막 다음으로 연결
			{
				p->next = pb;
				pb->prev = p;

				break;
			}
		}
	}
}
void PC_ShootBomb()
{
	if (pc.bomb == 0)
		return;
	pc.bomb--;
	bomb_y = pc.y;
}

void ShowPC(char blockInfo[4][4])
{
	int x, y;

	for (y = 0; y<4; y++)
	{
		for (x = 0; x<4; x++)
		{
			SetCurrentCursorPos(pc.x + (x * 2), pc.y + y);

			if (blockInfo[y][x] == 1) printf("◀");
			if (blockInfo[y][x] == 2) printf("■");
			if (blockInfo[y][x] == 3) printf("▶");
			if (blockInfo[y][x] == 4) printf("▲");
		}
	}
}
void DeletePC(char blockInfo[4][4])
{
	int x, y;

	for (y = 0; y<4; y++)
	{
		for (x = 0; x<4; x++)
		{
			SetCurrentCursorPos(pc.x + (x * 2), pc.y + y);

			if (blockInfo[y][x] != 0)
				printf(" ");
		}
	}
}
void ShowBoss(char blockInfo[4][4])
{
	int x, y;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			SetCurrentCursorPos(boss[main_stage - 1].x + (x * 2), boss[main_stage - 1].y + y);

			if (blockInfo[y][x] == 1) printf("▣");
			if (blockInfo[y][x] == 2) printf("◈");
		}
	}
}
void DeleteBoss(char blockInfo[4][4])
{
	int x, y;

	for (y = 0; y < 4; y++)
	{
		for (x = 0; x < 4; x++)
		{
			SetCurrentCursorPos(boss[main_stage - 1].x + (x * 2), boss[main_stage - 1].y + y);

			if (blockInfo[y][x] != 0)
				printf(" ");
		}
	}
}
void ShowFollow(int id, int i)
{
	switch (id)
	{
	case 0:
		SetCurrentCursorPos(f[id][i].x, f[id][i].y);
		printf("◎");
		break;
	case 1:
		SetCurrentCursorPos(f[id][i].x, f[id][i].y);
		printf("●");
		break;
	case 2:
		SetCurrentCursorPos(f[id][i].x, f[id][i].y);
		printf("○");
		break;
	}
}
void DeleteFollow(int id, int i)
{
	SetCurrentCursorPos(f[id][i].x, f[id][i].y);
	printf(" ");
}

void ShowPCBullet()
{
	for (pBullet b = pc.bul; b != NULL; b = b->next)
	{
		SetCurrentCursorPos(static_cast<int>(b->x), static_cast<int>(b->y));
		printf("|");
	}
}
void DeletePCBullet()
{
	for (pBullet b = pc.bul; b != NULL; b = b->next)
	{
		SetCurrentCursorPos(static_cast<int>(b->x), static_cast<int>(b->y));
		printf(" ");
	}
}
void ShowBomb()
{
	for (int x = 2; x<70; x += 2)
	{
		SetCurrentCursorPos(x, bomb_y);
		printf("▲");
	}
}
void DeleteBomb()
{
	for (int x = 2; x<70; x += 2)
	{
		SetCurrentCursorPos(x, bomb_y);
		printf(" ");
	}
}
void ShowBossBullet()
{
	for (pBullet pb = boss[main_stage - 1].bul; pb != NULL; pb = pb->next)
	{
		SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
		printf("o");
	}
}
void DeleteBossBullet()
{
	for (pBullet pb = boss[main_stage - 1].bul; pb != NULL; pb = pb->next)
	{
		SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
		printf(" ");
	}
}
void ShowFollowBullet(int id, int i)
{
	if (id == 1) //유도탄 이미지 따로
		for (pBullet pb = f[id][i].bul; pb != NULL; pb = pb->next)
		{
			SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
			printf("*");
		}
	else
		for (pBullet pb = f[id][i].bul; pb != NULL; pb = pb->next)
		{
			SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
			printf("ㅣ");
		}
}
void DeleteFollowBullet(int id, int i)
{
	for (pBullet pb = f[id][i].bul; pb != NULL; pb = pb->next)
	{
		SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
		printf(" ");
	}
}

void PC_Bullet_Move() //유저의 Shift들과 같다고 보면됨
{
	DeletePCBullet();
	for (pBullet pb = pc.bul; pb != NULL; pb = pb->next)
	{
		pb->y--;
		switch (PC_Bullet_Detect(pb))
		{
		case 0:
			continue;
		case 1: //총알이 벽에 충돌할때
			if (pb->prev == NULL && pb->next == NULL) //총알이 한발일때
				pc.bul = NULL;
			else if (pb->prev == NULL) //첫번째 총알일때
			{
				pc.bul = pb->next;
				pb->next->prev = NULL;
			}
			//벽에는 당연히 첫번째 총알이 충돌하므로 다른 경우 없음
			break;
		case 2://총알이 보스나 부하한테 충돌할 때
		case 3:
			if (pb->prev == NULL && pb->next == NULL) //총알이 한발일때
				pc.bul = NULL;
			else if (pb->prev == NULL) //첫번째 총알일때
			{
				pc.bul = pb->next;
				pb->next->prev = NULL;
			}
			else if (pb->next == NULL) //마지막 총알일때
			{
				pb->prev->next = NULL;
			}
			else //중간 총알일때
			{
				pb->next->prev = pb->prev;
				pb->prev->next = pb->next;
				pb = pb->prev;
			}
			break;
		}
	}
	ShowPCBullet();
	if (bomb_y == 1)
	{
		DeleteBomb();
		bomb_y = 100;
	}
	else if (bomb_y != 100)
	{
		DeleteBomb();
		bomb_y--;
		ShowBomb();
	}
}
void Boss_Bullet_Move()
{
	boss[main_stage - 1].bul_speed++;
	if (boss[main_stage - 1].bul_speed == 2)
	{
		DeleteBossBullet();
		for (pBullet pb = boss[main_stage - 1].bul; pb != NULL; pb = pb->next)
		{
			pb->x += pb->dir_x * 2;
			pb->y += pb->dir_y;
			switch (Boss_Bullet_Detect(pb))
			{
			case 0:
				break;
			case 2://총알이 유저한테 충돌할 때
				if (!nodieMode)
					pc.HP -= boss[main_stage - 1].Damage;
				if (reverseMode)
					reverseMode = false;
			case 1: //총알이 벽에 충돌할때
				if (pb->prev == NULL && pb->next == NULL) //총알이 한발일때
					boss[main_stage - 1].bul = NULL;
				else if (pb->prev == NULL) //첫번째 총알일때
				{
					boss[main_stage - 1].bul = pb->next;
					pb->next->prev = NULL;
				}
				else if (pb->next == NULL) //마지막 총알일때
				{
					pb->prev->next = NULL;
				}
				else //중간 총알일때
				{
					pb->next->prev = pb->prev;
					pb->prev->next = pb->next;
					pb = pb->prev;
				}
				break;
			}
			if (static_cast<int>(pb->y) >= bomb_y) // 단순 형변환
			{
				if (pb->prev == NULL && pb->next == NULL) //총알이 한발일때
					boss[main_stage - 1].bul = NULL;
				else if (pb->prev == NULL) //첫번째 총알일때
				{
					boss[main_stage - 1].bul = pb->next;
					pb->next->prev = NULL;
				}
				else if (pb->next == NULL) //마지막 총알일때
				{
					pb->prev->next = NULL;
				}
				else //중간 총알일때
				{
					pb->next->prev = pb->prev;
					pb->prev->next = pb->next;
					pb = pb->prev;
				}
			}
		}
		ShowBossBullet();
		boss[main_stage - 1].bul_speed = 0;
	}
}
void Follow_Bullet_Move()
{
	for (int id = 0; id<3; id++)
	{
		for (int i = 0; i<8; i++)
		{
			f[id][i].bul_speed++;
			if (f[id][i].bul_speed == 2)
			{
				DeleteFollowBullet(id, i);
				for (pBullet pb = f[id][i].bul; pb != NULL; pb = pb->next)
				{
					pb->x += pb->dir_x * 2;
					pb->y += pb->dir_y;
					switch (Follow_Bullet_Detect(pb))
					{
					case 0:
						break;
					case 2://총알이 유저한테 충돌할 때
						if (!nodieMode)
							pc.HP -= f[id][i].Damage;
						if (reverseMode)
							reverseMode = false;
					case 1: //총알이 벽에 충돌할때
						if (pb->prev == NULL && pb->next == NULL) //총알이 한발일때
							f[id][i].bul = NULL;
						else if (pb->prev == NULL) //첫번째 총알일때
						{
							f[id][i].bul = pb->next;
							pb->next->prev = NULL;
						}
						else if (pb->next == NULL) //마지막 총알일때
						{
							pb->prev->next = NULL;
						}
						else //중간 총알일때
						{
							pb->next->prev = pb->prev;
							pb->prev->next = pb->next;
							pb = pb->prev;
						}
						break;
					}
					if (static_cast<int>(pb->y) >= bomb_y)
					{
						if (pb->prev == NULL && pb->next == NULL) //총알이 한발일때
							f[id][i].bul = NULL;
						else if (pb->prev == NULL) //첫번째 총알일때
						{
							f[id][i].bul = pb->next;
							pb->next->prev = NULL;
						}
						else if (pb->next == NULL) //마지막 총알일때
						{
							pb->prev->next = NULL;
						}
						else //중간 총알일때
						{
							pb->next->prev = pb->prev;
							pb->prev->next = pb->next;
							pb = pb->prev;
						}
					}
				}
				ShowFollowBullet(id, i);
				f[id][i].bul_speed = 0;
			}
		}
	}

}

int PC_Bullet_Detect(pBullet pb) // 유저 총알 충돌 확인 함수
{
	if (pb->y == 0) // 벽에 충돌할때 1
		return 1;

	for (int id = 0; id<3; id++) //부하랑 충돌할때 3 
	{
		for (int i = 0; i<8; i++)
		{
			if (!f[id][i].exist)
				continue;
			if (f[id][i].x == pb->x && f[id][i].y == pb->y)
			{
				f[id][i].HP -= pc.Damage; //유저의 데미지 만큼 부하 체력 감소
				if (f[id][i].HP <= 0)
				{
					f[id][i].exist = false;
					DeleteFollow(id, i);
				}
				return 3;
			}
		}
	}

	if (boss[main_stage - 1].exist)
	{
		for (int x = 0; x<4; x++) //보스랑 충돌할때 2
		{
			for (int y = 0; y<4; y++)
			{
				if (boss[main_stage - 1].x + 2 * x == pb->x && pb->y == boss[main_stage - 1].y + y && blockModel[Boss_block_id][y][x] != 0)
				{
					boss[main_stage - 1].HP -= pc.Damage; //유저의 데미지 만큼 보스 체력 감소
					return 2;
				}
			}
		}
	}

	return 0;
}
int Boss_Bullet_Detect(pBullet pb)
{
	if (static_cast<int>(pb->x)<2 || static_cast<int>(pb->x)>68 || static_cast<int>(pb->y)<2 || static_cast<int>(pb->y)>26) // 벽에 충돌할때 1
		return 1;

	for (int x = 0; x<4; x++) //유저랑 박을 때 2
	{
		for (int y = 0; y<4; y++)
		{
			if ((blockModel[block_id][y][x] != 0 && pc.x + 2 * x == static_cast<int>(pb->x) && pc.y + y == static_cast<int>(pb->y)) ||
				(blockModel[block_id][y][x] != 0 && pc.x + 2 * x + 1 == static_cast<int>(pb->x) && pc.y + y == static_cast<int>(pb->y)))
				return 2;
		}
	}

	return 0;
}
int Follow_Bullet_Detect(pBullet pb)
{
	if (static_cast<int>(pb->x)<2 || static_cast<int>(pb->x)>68 || static_cast<int>(pb->y)<2 || static_cast<int>(pb->y)>26) // 벽에 충돌할때 1
		return 1;

	for (int x = 0; x<4; x++) //유저랑 박을 때
	{
		for (int y = 0; y<4; y++)
		{
			if ((blockModel[block_id][y][x] != 0 && pc.x + 2 * x == static_cast<int>(pb->x) && pc.y + y == static_cast<int>(pb->y)) ||
				(blockModel[block_id][y][x] != 0 && pc.x + 2 * x + 1 == static_cast<int>(pb->x) && pc.y + y == static_cast<int>(pb->y)))
				return 2;
		}
	}

	return 0;
}

void ProcessKeyInput(void)
{
	pc.speed++;
	if (pc.speed == 2)
	{
		if (blockMode)
			blockTime++;
		if (blockTime == 50) // 50*50*2 = 5000 5초
			blockMode = false;
		if (nodieMode)
			nodieTime++;
		if (nodieTime == 100) // 50*100*2 = 10000 10초
			nodieMode = false;
		if (GetAsyncKeyState(VK_LEFT))
		{
			if (reverseMode)
				ShiftRight();
			else
				ShiftLeft();
		}
		if (GetAsyncKeyState(VK_RIGHT))
		{
			if (reverseMode)
				ShiftLeft();
			else
				ShiftRight();
		}
		if (GetAsyncKeyState(VK_DOWN))
		{
			if (reverseMode)
				ShiftUp();
			else
				ShiftDown();
		}
		if (GetAsyncKeyState(VK_UP))
		{
			if (reverseMode)
				ShiftDown();
			else
				ShiftUp();
		}
		if (GetAsyncKeyState(VK_SPACE))
		{
			if (!blockMode && pc.y>2)
				PC_Shoot();
		}
		if (GetAsyncKeyState(0x5A))
		{
			if (!blockMode && pc.y>2)
				PC_ShootBomb();
		}

		pc.speed = 0;
	}
}

int MainStageClear() //모든 스테이지 클리어하면 1, 그 전 보스 잡았으면 ItemCreate()
{
	if (main_stage == 3 && boss[main_stage - 1].exist == true && boss[main_stage - 1].HP <= 0) //게임 클리어
	{
		clear = true;
		term = -1;
		boss[main_stage - 1].exist = false;
		DeleteBoss(blockModel[Boss_block_id]);
		boss[main_stage - 1].x = -1;
		boss[main_stage - 1].y = -1;
		for (int id = 0; id<3; id++)
		{
			for (int i = 0; i<8; i++)
			{
				if (!f[id][i].exist)
					continue;
				f[id][i].exist = false;
				DeleteFollow(id, i);
			}
		}
		return 1;
	}
	else if (boss[main_stage - 1].exist == true && boss[main_stage - 1].HP <= 0) //보스 클리어
	{
		term = -1;
		boss[main_stage - 1].exist = false;
		stage_clear = -1;   //클리어 변수 on하고 보스 지우고 아이템 생성
		DeleteBoss(blockModel[Boss_block_id]);
		boss[main_stage - 1].x = -1;
		boss[main_stage - 1].y = -1;
		for (int id = 0; id<3; id++)
		{
			for (int i = 0; i<8; i++)
			{
				if (!f[id][i].exist)
					continue;
				f[id][i].exist = false;
				DeleteFollow(id, i);
			}
		}
		ItemCreate();
	}
	if (stage_clear == -1) //클리어부터 먹기 전까지 계속 그림
		ShowItem();

	return 0;
}

void ItemCreate()
{
	//각각 위치 설정
	it[0].x = 18; it[0].y = 10; it[0].id = 0;
	it[1].x = 28; it[1].y = 10; it[1].id = 1;
	it[2].x = 38; it[2].y = 10; it[2].id = 2;
	it[3].x = 48; it[3].y = 10; it[3].id = 3;
}
void ItemApply(int id)
{
	int r;

	stage_clear = 0;
	switch (id)
	{
	case 0: //체력 증가
		pc.HP += 1; break;
	case 1: //데미지 증가
		pc.Damage += 1; break;
	case 2: //폭탄 개수 증가
		if (pc.bomb == 3) pc.bomb = 3;
		else pc.bomb += 1; break;
	case 3: //랜덤 아이템
		r = rand() % 5;

		switch (r)
		{
		case 0: //체력 감소
			pc.HP = 1; break;
		case 1: //풀피
			pc.HP = 5; break;
		case 2: //반대모드 켜짐
			reverseMode = true; break;
		case 3: //데미지 1감소
			blockMode = true; blockTime = -1; break;
		case 4:
			nodieMode = true; nodieTime = -1; break;
		}
	}
	DeleteItem();
}
void ShowItem()
{
	SetCurrentCursorPos(it[0].x, it[0].y);
	printf("♥");
	SetCurrentCursorPos(it[1].x, it[1].y);
	printf("ⓟ");
	SetCurrentCursorPos(it[2].x, it[2].y);
	printf("ⓑ");
	SetCurrentCursorPos(it[3].x, it[3].y);
	printf("?");
}
void DeleteItem()
{
	for (int i = 0; i<4; i++)
	{
		SetCurrentCursorPos(it[i].x, it[i].y);
		printf(" ");
	}
}

int show_gameover()
{
	SetCurrentCursorPos(15, 11);
	printf("┏━━━━━━━━━━━━━━━━┓");
	SetCurrentCursorPos(15, 12);
	printf("┃********************************┃");
	SetCurrentCursorPos(15, 13);
	printf("┃*           GAME OVER          *┃");
	SetCurrentCursorPos(15, 14);
	printf("┃********************************┃");
	SetCurrentCursorPos(15, 15);
	printf("┗━━━━━━━━━━━━━━━━┛");
	return 0;
}
int show_gamewin()
{
	SetCurrentCursorPos(15, 11);
	printf("┏━━━━━━━━━━━━━━━━┓");
	SetCurrentCursorPos(15, 12);
	printf("┃********************************┃");
	SetCurrentCursorPos(15, 13);
	printf("┃*        ★ GAME WIN ★        *┃");
	SetCurrentCursorPos(15, 14);
	printf("┃********************************┃");
	SetCurrentCursorPos(15, 15);
	printf("┗━━━━━━━━━━━━━━━━┛");
	return 0;
}

void PC_Init()
{
	pc.x = START_CURPOS_X;
	pc.y = START_CURPOS_Y;
	pc.HP = 3;
	pc.Damage = 1;
	pc.bomb = 3;
	pc.bul = NULL;
}
void Boss_Init()
{
	boss[main_stage - 1].exist = true;
	boss[main_stage - 1].x = BOSS_CURPOS_X;
	boss[main_stage - 1].y = BOSS_CURPOS_Y;
	boss[main_stage - 1].HP = 10 * main_stage + 10;
	boss[main_stage - 1].Damage = 2;
	boss[main_stage - 1].bul = NULL;
	boss[main_stage - 1].speed = 0;
}
void Follow_Init(int id)
{
	int left_x = rand() % 15 * 2;
	int right_x = rand() % 15 * 2 + 38;
	switch (id)
	{
	case 0: // 부하 1
		for (int i = 0; i<2; i++)
		{
			f[0][i].exist = true;
			f[0][i].HP = 1;
			f[0][i].x = left_x + i * 4;
			f[0][i].y = 1;
			f[0][i].bul = NULL;
			f[0][i].Damage = 1;
			f[0][i].speed = 0;
			f[0][i].bul_speed = 0;
			ShowFollow(0, i);
		}
		for (int i = 2; i<4; i++)
		{
			f[0][i].exist = true;
			f[0][i].HP = 1;
			f[0][i].x = right_x + (2 - i) * 4;
			f[0][i].y = 1;
			f[0][i].bul = NULL;
			f[0][i].Damage = 1;
			f[0][i].speed = 0;
			f[0][i].bul_speed = 0;
			ShowFollow(0, i);
		}
		break;
	case 4: // 부하 1
		for (int i = 4; i<6; i++)
		{
			f[0][i].exist = true;
			f[0][i].HP = 1;
			f[0][i].x = left_x + (i - 4) * 4;
			f[0][i].y = 1;
			f[0][i].bul = NULL;
			f[0][i].Damage = 1;
			f[0][i].speed = 0;
			f[0][i].bul_speed = 0;
			ShowFollow(0, i);
		}
		for (int i = 6; i<8; i++)
		{
			f[0][i].exist = true;
			f[0][i].HP = 1;
			f[0][i].x = right_x + (6 - i) * 4;
			f[0][i].y = 1;
			f[0][i].bul = NULL;
			f[0][i].Damage = 1;
			f[0][i].speed = 0;
			f[0][i].bul_speed = 0;
			ShowFollow(0, i);
		}
		break;
	case 1: // 부하 2
		f[1][0].exist = true;
		f[1][0].HP = 1;
		f[1][0].x = (rand() % 34 + 1) * 2;
		f[1][0].y = 1;
		f[1][0].bul = NULL;
		f[1][0].Damage = 1;
		f[1][0].speed = 0;
		f[1][0].bul_speed = 0;
		ShowFollow(1, 0);
		f[1][1].exist = true;
		f[1][1].HP = 1;
		do {
			f[1][1].x = (rand() % 34 + 1) * 2;
		} while (f[1][1].x == f[1][0].x); // x좌표가 같지 않으면 탈출
		f[1][1].y = 1;
		f[1][1].bul = NULL;
		f[1][1].Damage = 1;
		f[1][1].speed = 0;
		f[1][1].bul_speed = 0;
		ShowFollow(1, 1);
		break;
	case 5: // 부하 2
		f[1][2].exist = true;
		f[1][2].HP = 1;
		f[1][2].x = (rand() % 34 + 1) * 2;
		f[1][2].y = 1;
		f[1][2].bul = NULL;
		f[1][2].Damage = 1;
		f[1][2].speed = 0;
		f[1][2].bul_speed = 0;
		ShowFollow(1, 2);
		f[1][3].exist = true;
		f[1][3].HP = 1;
		do {
			f[1][3].x = (rand() % 34 + 1) * 2;
		} while (f[1][3].x == f[1][2].x);
		f[1][3].y = 1;
		f[1][3].bul = NULL;
		f[1][3].Damage = 1;
		f[1][3].speed = 0;
		f[1][3].bul_speed = 0;
		ShowFollow(1, 3);
		break;
	case 2: // 부하 3
		f[2][0].exist = true;
		f[2][0].HP = 10000;
		f[2][0].x = pc.x + 2;
		f[2][0].y = 1;
		f[2][0].bul = NULL;
		f[2][0].Damage = 1;
		f[2][0].speed = 0;
		f[2][0].bul_speed = 0;
		ShowFollow(2, 0);
		break;
	}
}
void Init() //이후 수정하게 됨
{
	main_stage = 1;

	PC_Init();
	ShowPC(blockModel[block_id]);

	boss[main_stage - 1].exist = false;
	for (int id = 0; id<3; id++)
		for (int i = 0; i<8; i++)
			f[id][i].exist = false;

	term = 0;
}

void test()
{
	SetCurrentCursorPos(73, 21);
	printf("Test\n");
}

int main(void)
{
	system("mode con:cols=95 lines=29");
	system("title 2017년 2학기 SW 설계 기초 1조 Sejong Plane");
	RemoveCursor();
	srand((unsigned int)time(NULL));

	pc.x = 38;
	pc.y = 15;

	FirstScreenAfterExecution();
	getchar();
	system("cls");

	DrawGameBoard();
	ShowCountBeforeStart();
	Init();

	while (1) 
	{
		if( term % 20 == 0) DrawGameBoard();

		DrawInfo(); //정보 출력
		if (term > -1)
			term++;

		if (term == 300) //15초 뒤 보스 생성
		{
			if (main_stage>1)
			{
				f[2][0].exist = false;
				DeleteFollow(2, 0);
			}
			Boss_Init();
		}

		Follower_Create();
		
		PC_Bullet_Move(); //각각 총알 움직임 항상 감시
		Boss_Bullet_Move();
		Follow_Bullet_Move();

		if (MainStageClear()) //게임 클리어시 탈출
		{
			Sleep(1000);
			break;
		}

		if (pc.HP <= 0)
		{
		Sleep(1000);
		break;
		}

		ProcessKeyInput(); //유저 조작(이 안에 슬립있음)
		if (boss[main_stage - 1].exist) //보스 있으면 움직임
			Boss_Move();
		Follower_Move(); //부하 움직임

		Sleep(sleep);
	}

	system("cls");
	if (clear)
		show_gamewin();
	else
		show_gameover();

	getchar();

	return 0;
}