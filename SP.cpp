#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include "blockModel.h"

#define PI 3.14159
#define GBOARD_WIDTH 45 //�� ���� ���� ũ��
#define GBOARD_HEIGHT 27
#define START_CURPOS_X 30 //���� ���� ��ǥ
#define START_CURPOS_Y 20
#define BOSS_CURPOS_X 32 //���� ���� ��ǥ
#define BOSS_CURPOS_Y 2

typedef struct bullet //�Ѿ� ����ü (���� ���Ḯ��Ʈ�� ������ �����ͷ� ���� ����)
{
	float x; //��ǥ
	float y;
	struct bullet *next; //������ ���� �Ѿ�
	struct bullet *prev;
	float dir_x; //����
	float dir_y;
}Bullet, *pBullet;
typedef struct user //PC, ���� ����ü
{
	int x; //��ǥ
	int y;
	int Damage; //������
	int HP; //ü��, ������
	pBullet bul; //�Ѿ�
	int bomb; //��ź, �ʻ��
	int speed;
}PC;
typedef struct boss //���� ����ü
{
	bool exist; //���� ����
	int x; //��ǥ
	int y;
	int HP; //ü��
	int Damage; //������
	pBullet bul; //�Ѿ�
	int speed;
	int bul_speed;
}Boss;
typedef struct follower //�̵�����
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
typedef struct item //������ ����ü
{
	int x; //��ǥ
	int y;
	int id; //����
}Item;

void SetCurrentCursorPos(int x, int y); //Ŀ�� ��ǥ �����ϱ�
void RemoveCursor(void); //Ŀ��(���� ������) ����

int FirstScreenAfterExecution(); //ù ȭ��
void DrawGameBoard(); //�� �׸���
void DrawInfo(); //���� ȭ�� ������ ���� �� ���� ���� ǥ��
int ShowCountBeforeStart(); // 3,2,1 �����ֱ�

void ShiftRight(); //���� �����¿� �̵�
void ShiftLeft();
void ShiftUp();
void ShiftDown();

void Boss_ShiftLeft(); //���� �¿� �̵�
void Boss_ShiftRight();
void Boss_Move(); //���� �ൿ �Ѱ�(���� �ѽ�� �͵� ���⿡ �߰�) ������ ProcessKeyInput�� ���ٰ� �����ϸ� ��

void Follower_ShiftLeft(int id, int i);
void Follower_ShiftRight(int id, int i);
void Follower_ShiftDown(int id, int i);
void Follower_Shoot(int id, int i);
void Follower_Move();
void Follower_Create();

void PC_Bullet_Move(); //���� �Ѿ� �̵� �Ѱ�
void Boss_Bullet_Move(); //���� �Ѿ� �̵� �Ѱ�
void Follow_Bullet_Move();

void ShowPC(char blockInfo[4][4]); //���� �׸��� �� �����
void DeletePC(char blockInfo[4][4]);
void ShowBoss(char blockInfo[4][4]); //���� �׸��� �� �����
void DeleteBoss(char blockInfo[4][4]);
void ShowFollow(int id, int i); //���� �׸��� �� �����
void DeleteFollow(int id, int i);

void ShowPCBullet(); //���� �Ѿ� �׸��� �� �����
void DeletePCBullet();
void ShowBossBullet();
void DeleteBossBullet();
void ShowFollowBullet(int id, int i);
void DeleteFollowBullet(int id, int i);

int PC_DetectCollision(int posX, int posY, char blockModel[][4]); //���� �浹 ���� �Ѱ�
int Boss_DetectCollision(int posX, int posY, char blockModel[][4]); //���� �浹 ���� �Ѱ�
int Follower_DetectCollision(int id, int i, int posX, int posY);

int PC_Bullet_Detect(pBullet pb); //���� �Ѿ� �浹 ���� �Ѱ�
int Boss_Bullet_Detect(pBullet pb); //���� �Ѿ� �浹 ���� �Ѱ�
int Follow_Bullet_Detect(pBullet pb);

int MainStageClear();

void PC_Init(); //���� ���� �ʱ�ȭ
void Boss_Init(); //���� ���� �ʱ�ȭ
void Follow_Init(int id); //���� ���� �ʱ�ȭ
void Init(); //���� �ʱ�ȭ

void test(); //���� �� �� �׽�Ʈ��

int MainStageClear(); //�������� �� ���� Ŭ���� �˻�
void ItemCreate(); //������ ����
void ItemApply(int id); //������ ����

void ShowItem(); //������ �׸��� �� �����
void DeleteItem();

int show_gamewin(); //Ŭ��� ���ӿ��� �׸���
int show_gameover();

PC pc; //���� ����ü ����
Boss boss[3]; //���� ����ü �迭 ����(3��������)
Follower f[3][8]; //�ѹ��� 8���� ���������
Item it[4]; //������ ����ü �迭 ����(4����)

int block_id = 1; //���� ��� id
int Boss_block_id = 2; //���� ��� id

int bomb_y = 100; //��ź ��ġ (�� ������ 100)

int term; //���� ���� ���� �ֱ� ����
int sleep = 50; //���� ���� 1000=1��(�Ѿ� �ӵ� ����)
int gameBoardInfo[GBOARD_HEIGHT + 1][GBOARD_WIDTH + 2] = { 0, }; //�� ���

int main_stage; //���� �������� (���� Ŭ���� ����)

bool clear = false; //���� Ŭ���� Ȯ�� ����
int stage_clear = 0; //�������� Ŭ���� Ȯ�� ����(-1�� Ŭ����)

bool reverseMode = false; //����Ű �Ųٷ� Ŭ���� Ȯ�� ����

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
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // �� �ٲٰ�
	printf("�� Sejong Plane  ��");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); // �ٽ� �Ͼ������.



	Sleep(100);

	SetCurrentCursorPos(6, 6);

	printf("SW ���� ���� 1��");

	Sleep(100);

	SetCurrentCursorPos(44, 6);

	printf("���ؼ�, ���ؼ�, ������, ������");

	SetCurrentCursorPos(37, 15);

	ShowPC(blockModel[block_id]); // ������ �� ����� ��� �����ֱ�.

	Sleep(500);

	SetCurrentCursorPos(30, 21);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 10); // �� �ٲٰ�

	printf("Please Press Enter Key!");
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); // �ٽ� �������

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
				printf("��");
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

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 14); // �� �ٲٰ�

	SetCurrentCursorPos(73, 8);
	printf("    [Boss HP]\n");

	SetCurrentCursorPos(73, 10);
	printf("     %3d", boss[main_stage - 1].HP);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //�ٽ� ���� ������

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); // �� �ٲٰ�

	SetCurrentCursorPos(73, 14);
	printf("     [My HP]\n");

	if (nodieMode)
		{
			SetCurrentCursorPos(73, 16);
			printf("    ���� ����");

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
			printf("��");
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //�ٽ� ���� ������


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
		printf("��");
	}

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); //�ٽ� ���� ������

	SetCurrentCursorPos(73, 25);
	printf("Space : Bullet");

	SetCurrentCursorPos(73, 26);
	printf("z : Bomb");
}
int ShowCountBeforeStart()
{
	int i, j, k;

	SetCurrentCursorPos(17, 5);
	printf("������������������������");
	SetCurrentCursorPos(17, 6);
	printf("��       ����       ��");
	SetCurrentCursorPos(17, 7);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 8);
	printf("��             ��     ��");
	SetCurrentCursorPos(17, 9);
	printf("��             ��     ��");
	SetCurrentCursorPos(17, 10);
	printf("��         ���       ��");
	SetCurrentCursorPos(17, 11);
	printf("��             ��     ��");
	SetCurrentCursorPos(17, 12);
	printf("��    ��        ��    ��");
	SetCurrentCursorPos(17, 13);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 14);
	printf("��       ����       ��");
	SetCurrentCursorPos(17, 15);
	printf("������������������������");

	SetCurrentCursorPos(0, 23);
	Sleep(1000);

	SetCurrentCursorPos(17, 5);
	printf("������������������������");
	SetCurrentCursorPos(17, 6);
	printf("��       ����       ��");
	SetCurrentCursorPos(17, 7);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 8);
	printf("��             ��     ��");
	SetCurrentCursorPos(17, 9);
	printf("��            ��      ��");
	SetCurrentCursorPos(17, 10);
	printf("��          ��        ��");
	SetCurrentCursorPos(17, 11);
	printf("��        ��          ��");
	SetCurrentCursorPos(17, 12);
	printf("��      ��            ��");
	SetCurrentCursorPos(17, 13);
	printf("��    ��              ��");
	SetCurrentCursorPos(17, 14);
	printf("��    �������    ��");
	SetCurrentCursorPos(17, 15);
	printf("������������������������");

	SetCurrentCursorPos(0, 23);
	Sleep(1000);

	SetCurrentCursorPos(17, 5);
	printf("������������������������");
	SetCurrentCursorPos(17, 6);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 7);
	printf("��       ���         ��");
	SetCurrentCursorPos(17, 8);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 9);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 10);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 11);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 12);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 13);
	printf("��         ��         ��");
	SetCurrentCursorPos(17, 14);
	printf("��       ����       ��");
	SetCurrentCursorPos(17, 15);
	printf("������������������������");

	SetCurrentCursorPos(0, 23);
	Sleep(1000);

	SetCurrentCursorPos(17, 5);
	printf("������������������������");
	SetCurrentCursorPos(17, 6);
	printf("��       ����       ��");
	SetCurrentCursorPos(17, 7);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 8);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 9);
	printf("��     ��    ���     ��");
	SetCurrentCursorPos(17, 10);
	printf("��     ��  ��  ��     ��");
	SetCurrentCursorPos(17, 11);
	printf("��     ���    ��     ��");
	SetCurrentCursorPos(17, 12);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 13);
	printf("��     ��      ��     ��");
	SetCurrentCursorPos(17, 14);
	printf("��       ����       ��");
	SetCurrentCursorPos(17, 15);
	printf("������������������������");


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
				return 0; //������ ���̶� �ھ��� ��

			else if (stage_clear == -1) //�������̶� �ھ��� ��
			{
				for (int i = 0; i<4; i++)
				{
					if (blockModel[y][x] != 0 && posX + 2 * x == it[i].x && posY + y == it[i].y)
					{
						ItemApply(i); //�ش� ������ �����ϰ� ��ġ�� �ʱ�ȭ
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
int Boss_DetectCollision(int posX, int posY, char blockModel[][4]) //���� ����
{
	int x, y;

	for (x = 0; x <4; x++)
	{
		for (y = 0; y <4; y++)
		{
			if (blockModel[y][x] != 0 && gameBoardInfo[posY + y][posX / 2 + x] == 1)
				return 0; //������ ���̶� �ھ��� ��
			if (blockModel[y][x] != 0 && (posX - 6 < pc.x && pc.x < posX + 9 && pc.y<posY + 3))
			{
				pc.HP = 0;
			//	if (reverseMode)
			//		reverseMode = false;
				return 0; //������ ������ �ھ�����
			}
		}
	}

	return 1;
}
int Follower_DetectCollision(int id, int i, int posX, int posY)
{
	if (gameBoardInfo[posY][posX / 2] == 1)
		return 0; //���ϰ� ���̶� �ھ��� ��
	for (int x = 0; x<4; x++) //������ ���� ��
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
		pBullet pb; //���ο� �Ѿ� ����(�� ��� ������ ����)
		pb = (pBullet)malloc(sizeof(Bullet));
		pb->x = boss[main_stage - 1].x + 2 + i;
		pb->y = boss[main_stage - 1].y + 3;
		pb->dir_x = 0;
		pb->dir_y = 1;
		pb->next = NULL;
		pb->prev = NULL;

		if (boss[main_stage - 1].bul == NULL) //�����ϴ� �Ѿ��� ���� ��
			boss[main_stage - 1].bul = pb;
		else //���� ��
		{
			for (pBullet p = boss[main_stage - 1].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //������ �������� ����
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
	int bul_num = 8; //�Ѿ� ����
	double angle = 180 / (bul_num + 1); 
	for (int i = 0; i<bul_num; i++)
	{
		pBullet pb; //���ο� �Ѿ� ����(�� ��� ������ ����)
		pb = (pBullet)malloc(sizeof(Bullet));
		pb->x = boss[main_stage - 1].x + 2;
		pb->y = boss[main_stage - 1].y + 3;
		double rad = angle*(i + 1)*(PI / 180); // c������ cos, sin�� �������� ���.
		pb->dir_x = (float)cos(rad);
		pb->dir_y = (float)sin(rad);
		pb->next = NULL;
		pb->prev = NULL;

		if (boss[main_stage - 1].bul == NULL) //�����ϴ� �Ѿ��� ���� ��
			boss[main_stage - 1].bul = pb;
		else //���� ��
		{
			for (pBullet p = boss[main_stage - 1].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //������ �������� ����
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
	if (boss[main_stage - 1].y + 3 == bomb_y) //��ź �浹 �˻�
		boss[main_stage - 1].HP -= 10;
	if (boss[main_stage - 1].speed == 201) //�ӵ� �ʱ�ȭ
		boss[main_stage - 1].speed = 0;
	boss[main_stage - 1].speed++;
	if (main_stage == 3 && 160<boss[main_stage - 1].speed && boss[main_stage - 1].speed <= 180) //����
		Boss_ShiftDown();
	else if (main_stage == 3 && 180<boss[main_stage - 1].speed && boss[main_stage - 1].speed <= 200) //���� �� ���ƿ���
		Boss_ShiftUp();
	else if (main_stage == 1 && boss[main_stage - 1].speed % 100 == 0) //��ä�� �߻� (�������� 1�̸� �ѹ�, �ƴϸ� ���� �������ص���)
		Boss_Shoot_Veer();
	else if (main_stage != 1 && 90<boss[main_stage - 1].speed && boss[main_stage - 1].speed <= 110)
		Boss_Shoot_Veer();
	else if (boss[main_stage - 1].speed % 20 == 0) // �Ѿ� �߻�
		Boss_Shoot();
	else if (boss[main_stage - 1].speed % 5 == 0)
	{
		int BossShiftVar; //���� �¿� ����
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
	pBullet pb; //���ο� �Ѿ� ����(�� ��� ������ ����)
	pb = (pBullet)malloc(sizeof(Bullet));
	pb->next = NULL;
	pb->prev = NULL;
	switch (id)
	{
	case 0: // ����1
	case 2: // ����3
		pb->x = f[id][i].x;
		pb->y = f[id][i].y + 1;
		if (pb->y>25)
			return;
		pb->dir_x = 0;
		pb->dir_y = 1;

		if (f[id][i].bul == NULL) //�����ϴ� �Ѿ��� ���� ��
			f[id][i].bul = pb;
		else //���� ��
		{
			for (pBullet p = f[id][i].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //������ �������� ����
				{
					p->next = pb;
					pb->prev = p;
					break;
				}
			}
		}
		break;
	case 1: // ����2
		pb->x = f[id][i].x;
		pb->y = f[id][i].y;
		int diff_x = (pc.x + 2 - f[id][i].x) / 2;
		int diff_y = pc.y + 1 - f[id][i].y;
		int larger = (abs(diff_x)>abs(diff_y)) ? abs(diff_x) : abs(diff_y);
		pb->dir_x = (float)diff_x / larger;
		pb->dir_y = (float)diff_y / larger;

		if (f[id][i].bul == NULL) //�����ϴ� �Ѿ��� ���� ��
			f[id][i].bul = pb;
		else //���� ��
		{
			for (pBullet p = f[id][i].bul; p != NULL; p = p->next)
			{
				if (p->next == NULL) //������ �������� ����
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
			if (id == 2) //����3��(���� ���̽�)
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
			if (f[id][i].speed % 5 == 0) //����1,2 
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
	pBullet pb; //���ο� �Ѿ� ����(�� ��� ������ ����)
	pb = (pBullet)malloc(sizeof(Bullet));
	pb->x = pc.x + 2;
	pb->y = pc.y;
	pb->next = NULL;
	pb->prev = NULL;

	if (pc.bul == NULL) //�����ϴ� �Ѿ��� ���� ��
		pc.bul = pb;
	else //���� ��
	{
		for (pBullet p = pc.bul; p != NULL; p = p->next)
		{
			if (p->next == NULL) //������ �������� ����
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

			if (blockInfo[y][x] == 1) printf("��");
			if (blockInfo[y][x] == 2) printf("��");
			if (blockInfo[y][x] == 3) printf("��");
			if (blockInfo[y][x] == 4) printf("��");
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

			if (blockInfo[y][x] == 1) printf("��");
			if (blockInfo[y][x] == 2) printf("��");
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
		printf("��");
		break;
	case 1:
		SetCurrentCursorPos(f[id][i].x, f[id][i].y);
		printf("��");
		break;
	case 2:
		SetCurrentCursorPos(f[id][i].x, f[id][i].y);
		printf("��");
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
		printf("��");
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
	if (id == 1) //����ź �̹��� ����
		for (pBullet pb = f[id][i].bul; pb != NULL; pb = pb->next)
		{
			SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
			printf("*");
		}
	else
		for (pBullet pb = f[id][i].bul; pb != NULL; pb = pb->next)
		{
			SetCurrentCursorPos(static_cast<int>(pb->x), static_cast<int>(pb->y));
			printf("��");
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

void PC_Bullet_Move() //������ Shift��� ���ٰ� �����
{
	DeletePCBullet();
	for (pBullet pb = pc.bul; pb != NULL; pb = pb->next)
	{
		pb->y--;
		switch (PC_Bullet_Detect(pb))
		{
		case 0:
			continue;
		case 1: //�Ѿ��� ���� �浹�Ҷ�
			if (pb->prev == NULL && pb->next == NULL) //�Ѿ��� �ѹ��϶�
				pc.bul = NULL;
			else if (pb->prev == NULL) //ù��° �Ѿ��϶�
			{
				pc.bul = pb->next;
				pb->next->prev = NULL;
			}
			//������ �翬�� ù��° �Ѿ��� �浹�ϹǷ� �ٸ� ��� ����
			break;
		case 2://�Ѿ��� ������ �������� �浹�� ��
		case 3:
			if (pb->prev == NULL && pb->next == NULL) //�Ѿ��� �ѹ��϶�
				pc.bul = NULL;
			else if (pb->prev == NULL) //ù��° �Ѿ��϶�
			{
				pc.bul = pb->next;
				pb->next->prev = NULL;
			}
			else if (pb->next == NULL) //������ �Ѿ��϶�
			{
				pb->prev->next = NULL;
			}
			else //�߰� �Ѿ��϶�
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
			case 2://�Ѿ��� �������� �浹�� ��
				if (!nodieMode)
					pc.HP -= boss[main_stage - 1].Damage;
				if (reverseMode)
					reverseMode = false;
			case 1: //�Ѿ��� ���� �浹�Ҷ�
				if (pb->prev == NULL && pb->next == NULL) //�Ѿ��� �ѹ��϶�
					boss[main_stage - 1].bul = NULL;
				else if (pb->prev == NULL) //ù��° �Ѿ��϶�
				{
					boss[main_stage - 1].bul = pb->next;
					pb->next->prev = NULL;
				}
				else if (pb->next == NULL) //������ �Ѿ��϶�
				{
					pb->prev->next = NULL;
				}
				else //�߰� �Ѿ��϶�
				{
					pb->next->prev = pb->prev;
					pb->prev->next = pb->next;
					pb = pb->prev;
				}
				break;
			}
			if (static_cast<int>(pb->y) >= bomb_y) // �ܼ� ����ȯ
			{
				if (pb->prev == NULL && pb->next == NULL) //�Ѿ��� �ѹ��϶�
					boss[main_stage - 1].bul = NULL;
				else if (pb->prev == NULL) //ù��° �Ѿ��϶�
				{
					boss[main_stage - 1].bul = pb->next;
					pb->next->prev = NULL;
				}
				else if (pb->next == NULL) //������ �Ѿ��϶�
				{
					pb->prev->next = NULL;
				}
				else //�߰� �Ѿ��϶�
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
					case 2://�Ѿ��� �������� �浹�� ��
						if (!nodieMode)
							pc.HP -= f[id][i].Damage;
						if (reverseMode)
							reverseMode = false;
					case 1: //�Ѿ��� ���� �浹�Ҷ�
						if (pb->prev == NULL && pb->next == NULL) //�Ѿ��� �ѹ��϶�
							f[id][i].bul = NULL;
						else if (pb->prev == NULL) //ù��° �Ѿ��϶�
						{
							f[id][i].bul = pb->next;
							pb->next->prev = NULL;
						}
						else if (pb->next == NULL) //������ �Ѿ��϶�
						{
							pb->prev->next = NULL;
						}
						else //�߰� �Ѿ��϶�
						{
							pb->next->prev = pb->prev;
							pb->prev->next = pb->next;
							pb = pb->prev;
						}
						break;
					}
					if (static_cast<int>(pb->y) >= bomb_y)
					{
						if (pb->prev == NULL && pb->next == NULL) //�Ѿ��� �ѹ��϶�
							f[id][i].bul = NULL;
						else if (pb->prev == NULL) //ù��° �Ѿ��϶�
						{
							f[id][i].bul = pb->next;
							pb->next->prev = NULL;
						}
						else if (pb->next == NULL) //������ �Ѿ��϶�
						{
							pb->prev->next = NULL;
						}
						else //�߰� �Ѿ��϶�
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

int PC_Bullet_Detect(pBullet pb) // ���� �Ѿ� �浹 Ȯ�� �Լ�
{
	if (pb->y == 0) // ���� �浹�Ҷ� 1
		return 1;

	for (int id = 0; id<3; id++) //���϶� �浹�Ҷ� 3 
	{
		for (int i = 0; i<8; i++)
		{
			if (!f[id][i].exist)
				continue;
			if (f[id][i].x == pb->x && f[id][i].y == pb->y)
			{
				f[id][i].HP -= pc.Damage; //������ ������ ��ŭ ���� ü�� ����
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
		for (int x = 0; x<4; x++) //������ �浹�Ҷ� 2
		{
			for (int y = 0; y<4; y++)
			{
				if (boss[main_stage - 1].x + 2 * x == pb->x && pb->y == boss[main_stage - 1].y + y && blockModel[Boss_block_id][y][x] != 0)
				{
					boss[main_stage - 1].HP -= pc.Damage; //������ ������ ��ŭ ���� ü�� ����
					return 2;
				}
			}
		}
	}

	return 0;
}
int Boss_Bullet_Detect(pBullet pb)
{
	if (static_cast<int>(pb->x)<2 || static_cast<int>(pb->x)>68 || static_cast<int>(pb->y)<2 || static_cast<int>(pb->y)>26) // ���� �浹�Ҷ� 1
		return 1;

	for (int x = 0; x<4; x++) //������ ���� �� 2
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
	if (static_cast<int>(pb->x)<2 || static_cast<int>(pb->x)>68 || static_cast<int>(pb->y)<2 || static_cast<int>(pb->y)>26) // ���� �浹�Ҷ� 1
		return 1;

	for (int x = 0; x<4; x++) //������ ���� ��
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
		if (blockTime == 50) // 50*50*2 = 5000 5��
			blockMode = false;
		if (nodieMode)
			nodieTime++;
		if (nodieTime == 100) // 50*100*2 = 10000 10��
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

int MainStageClear() //��� �������� Ŭ�����ϸ� 1, �� �� ���� ������� ItemCreate()
{
	if (main_stage == 3 && boss[main_stage - 1].exist == true && boss[main_stage - 1].HP <= 0) //���� Ŭ����
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
	else if (boss[main_stage - 1].exist == true && boss[main_stage - 1].HP <= 0) //���� Ŭ����
	{
		term = -1;
		boss[main_stage - 1].exist = false;
		stage_clear = -1;   //Ŭ���� ���� on�ϰ� ���� ����� ������ ����
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
	if (stage_clear == -1) //Ŭ������� �Ա� ������ ��� �׸�
		ShowItem();

	return 0;
}

void ItemCreate()
{
	//���� ��ġ ����
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
	case 0: //ü�� ����
		pc.HP += 1; break;
	case 1: //������ ����
		pc.Damage += 1; break;
	case 2: //��ź ���� ����
		if (pc.bomb == 3) pc.bomb = 3;
		else pc.bomb += 1; break;
	case 3: //���� ������
		r = rand() % 5;

		switch (r)
		{
		case 0: //ü�� ����
			pc.HP = 1; break;
		case 1: //Ǯ��
			pc.HP = 5; break;
		case 2: //�ݴ��� ����
			reverseMode = true; break;
		case 3: //������ 1����
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
	printf("��");
	SetCurrentCursorPos(it[1].x, it[1].y);
	printf("��");
	SetCurrentCursorPos(it[2].x, it[2].y);
	printf("��");
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
	printf("������������������������������������");
	SetCurrentCursorPos(15, 12);
	printf("��********************************��");
	SetCurrentCursorPos(15, 13);
	printf("��*           GAME OVER          *��");
	SetCurrentCursorPos(15, 14);
	printf("��********************************��");
	SetCurrentCursorPos(15, 15);
	printf("������������������������������������");
	return 0;
}
int show_gamewin()
{
	SetCurrentCursorPos(15, 11);
	printf("������������������������������������");
	SetCurrentCursorPos(15, 12);
	printf("��********************************��");
	SetCurrentCursorPos(15, 13);
	printf("��*        �� GAME WIN ��        *��");
	SetCurrentCursorPos(15, 14);
	printf("��********************************��");
	SetCurrentCursorPos(15, 15);
	printf("������������������������������������");
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
	case 0: // ���� 1
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
	case 4: // ���� 1
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
	case 1: // ���� 2
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
		} while (f[1][1].x == f[1][0].x); // x��ǥ�� ���� ������ Ż��
		f[1][1].y = 1;
		f[1][1].bul = NULL;
		f[1][1].Damage = 1;
		f[1][1].speed = 0;
		f[1][1].bul_speed = 0;
		ShowFollow(1, 1);
		break;
	case 5: // ���� 2
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
	case 2: // ���� 3
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
void Init() //���� �����ϰ� ��
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
	system("title 2017�� 2�б� SW ���� ���� 1�� Sejong Plane");
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

		DrawInfo(); //���� ���
		if (term > -1)
			term++;

		if (term == 300) //15�� �� ���� ����
		{
			if (main_stage>1)
			{
				f[2][0].exist = false;
				DeleteFollow(2, 0);
			}
			Boss_Init();
		}

		Follower_Create();
		
		PC_Bullet_Move(); //���� �Ѿ� ������ �׻� ����
		Boss_Bullet_Move();
		Follow_Bullet_Move();

		if (MainStageClear()) //���� Ŭ����� Ż��
		{
			Sleep(1000);
			break;
		}

		if (pc.HP <= 0)
		{
		Sleep(1000);
		break;
		}

		ProcessKeyInput(); //���� ����(�� �ȿ� ��������)
		if (boss[main_stage - 1].exist) //���� ������ ������
			Boss_Move();
		Follower_Move(); //���� ������

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