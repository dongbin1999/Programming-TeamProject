#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <stdbool.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#define GunSound ".\\�ѼҸ� 12.wav"
#define ReloadSound ".\\������ 7.wav"

#define EASY 1 //���̵�
#define HARD 2 //���̵�
#define XLIMIT 35 //���α���
#define YLIMIT 23//���α���
#define FRAME 5 //���� ������
#define UNIT 20 //��ü �ִ� ����
#define EDGE 5 //Ŀ�� ������

//�۾� ���� ���,
#define DARKGRAY 8
#define LIGHTCYAN 11
#define YELLOW 14
#define LIGHTGREEN 10
#define LIGHTMAGENTA 13
#define RED 4

//��ü�� �������� �����ϴ� ���.
const int ZOMBIE = 1;
const int BULLET = 2;
const int PLAYER = 3;

typedef struct result {
    int score; //ȹ�� ����.
    clock_t runtime; //���� �ð�.
}result;

typedef struct element {
    int x, y; //x,y��ǥ.
    int id; //��ü�� Ư��.(����/�Ѿ�/ĳ����)
    int speed; //��ü�� �������� �ӵ� = FRAME * speed(ms).
    int period; //period == 0�� ��, object�� �� ĭ ������. (speed���� �ֱ��)
    bool create; //��ü�� ���� ����.
    char direct; //��ü�� �����̴� ����.
}element;

int start(int highest); //���� ���� �� ���� ȭ��.
result running_game(int difficulty); //���� ���� ȭ��.
bool Gameover(result R); //���â

void gotoxy(int x, int y); //Ŀ�� ��ġ �̵�.
void show(element object); //��ü ���
void erase(element object); //��ü �����
void outline(); //���� ȭ���� Ʋ �׸���.
void textColor(int color); //�۾� �� ����.
bool is_collide(element drop, element player); //ĳ���Ϳ� ��ü�� �浹 ����.
element create_zombie(element zombie, int speed[2]); //���� ����.
element create_bullet(element bullet, char direct, element player); //�Ѿ� ����.
void move_player(element* player, char move); //ĳ������ ������ ����.
void move_zombie(element* zombie, element player); //���� ������ ����.
void move_object(element* object); //����,�Ѿ�,ĳ���� ������ ����.
void sound_gun(void); //�� �Ҹ� ����
void sound_reload(void); //���� �Ҹ� ����

int main() {
    int highest = 0; //�ְ����� ���.
    while (true) {
        int difficulty = start(highest); //���̵��� ����ȭ�鿡�� ������.
        srand((unsigned)time(NULL));
        result R = running_game(difficulty);
        highest = max(highest, R.score);
        if (Gameover(R) == false)
            break;
    }
    gotoxy(EDGE, YLIMIT + EDGE + 1); //���� �޽��� ��� ��ġ.
    return 0;
}

void gotoxy(int x, int y) {
    COORD Pos = { 2 * x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}

int start(int highest) {
    textColor(YELLOW);
    system("cls");
    outline();
    {gotoxy(EDGE, 1);
    printf("�ٿ�ε� : https://github.com/dongbin1999/Programming-TeamProject");
    gotoxy(EDGE, 2);
    printf("����ó(github) : https://dongbin1999.github.io/jekyll/update/Programming-TeamProject/");
    gotoxy(EDGE, 3);
    printf("����ó(email) : dongbin1999@inu.ac.kr");
    gotoxy(17, 7);
    printf("[�̴� ������� Ver.1.0]\n\n");
    gotoxy(17, 9);
    printf("���� �ְ����� : %5d��", highest);
    gotoxy(20, 11);
    textColor(LIGHTCYAN);
    printf("[���� ���]\n");
    gotoxy(7, 13);
    printf("1) ����Ű�� ĳ���͸� ������ ����κ��� ���մϴ�.\n");
    gotoxy(7, 14);
    printf("2) W,A,S,DŰ�� �Ѿ��� �߻��մϴ�.\n");
    gotoxy(7, 15);
    printf("3) ���� ������ 10����, �����ð� 1�ʸ��� 5���� ȹ���մϴ�.\n");
    gotoxy(7, 16);
    printf("4) �������� ��� ������ �й��մϴ�.\n");
    gotoxy(12, 20);
    textColor(YELLOW);
    printf("���̵��� �����ϼ��� (���� : 1, ����� : 2)"); }

    int difficulty = 0;
    char command = 0;
    while (true) {
        command = getch();
        if ((command != '1') && (command != '2'))
            continue;
        else {
            difficulty = command - '0';
            break;
        }
    }

    gotoxy(12, 21);
    printf("���̵� : %s�� �����ϼ̽��ϴ�.", (difficulty == EASY) ? "����" : "�����");
    {Sleep(500);
    gotoxy(12, 22);
    printf("�ƹ�Ű�� �Է��Ͽ� ������ �����ϼ���!");
    getch();

    for (int i = 3; i > 0; i--) {
        system("cls");
        printf("%d�� �� ������ ���۵˴ϴ�...\n", i);
        Sleep(1000);
    }
    system("cls"); }
    return difficulty;
}

bool is_collide(element object, element player) {
    if (abs(player.x - object.x) <= 1 && abs(player.y - object.y) <= 1)
        return true;
    else
        return false;
}

result running_game(int difficulty) {
    element zombie[UNIT] = { {0} }, bullet[UNIT] = { 0 }, player = { XLIMIT / 2 + EDGE, YLIMIT / 2 + EDGE, PLAYER }, temp = { 0 };
    int i = 0, j = 0, remains = UNIT, delay = 1; //remains : ���� �Ѿ� ��, delay : �� ������ �ð�.
    char command = 0;

    //probability : ��ü�� ������ Ȯ��.(1/probability), speed[2] : ��ü�� �������� �ӵ��� �ִ�/�ּ� ����.
    int probability = 0, speed[2] = { 0,0 }, life = 0;
    result R = { 0,0 };

    if (difficulty == EASY) {
        probability = 50; speed[0] = 40; speed[1] = 80; life = 5;
    }
    else if (difficulty == HARD) {
        probability = 10; speed[0] = 20; speed[1] = 40; life = 2;
    }

    textColor(LIGHTGREEN);
    system("cls");
    outline();
    textColor(YELLOW);
    gotoxy(45, 7);
    printf("��  ������ :   %d  ��", life);
    gotoxy(45, 5);
    printf("��  ���� : %5d  ��", R.score);

    //�ʱ�ȭ ��, �ð����� ����.
    clock_t startTime, currentTime;
    startTime = clock();

    while (1) {
        //���� �Ѹ��� ����.
        if (rand() % probability == 0)
            for (i = 0; i < UNIT; i++)
                if (zombie[i].create == false) {
                    zombie[i] = create_zombie(zombie[i], speed);
                    break;
                }

        Sleep(FRAME);
        show(player);

        //�� �߻�/������, ĳ���� ������ ����,
        if (kbhit()) {
            command = getch();
            switch (command)
            {
            case 'w':case 'a':case 's':case 'd': //�Ѿ��� �߻��� ���
                if (delay < 0) { //������ �ð��� ��������,
                    remains--; //�Ѿ� �� �� ����.
                    if (remains >= 0) {
                        bullet[remains] = create_bullet(bullet[remains], command, player);
                        sound_gun();
                    }
                    gotoxy(15, 3);
                    textColor(YELLOW);
                    printf("���� �Ѿ� : %2d/%2d", max(0, remains), UNIT);
                }
                break;
            case 72:case 75:case 77:case 80: //ĳ���͸� ������ ���
                erase(player);
                move_player(&player, command);
                show(player);
                break;
            case 'r': //�Ѿ� �������ϴ� ���
                if (delay < 0) {
                    remains = UNIT; //�Ѿ� ����.
                    sound_reload();
                    delay = 300; //������ �ð� : 1.5��.
                    gotoxy(15, 3);
                    textColor(YELLOW);
                    printf("�Ѿ� ��������....");
                }
                break;
            default:
                break;
            }
        }

        //����� ĳ���� �浹 ���� Ȯ��.
        for (i = 0; i < UNIT; i++) {
            if (zombie[i].create == true) {
                zombie[i].period--;
                if (zombie[i].period <= 0) {
                    zombie[i].period = zombie[i].speed;//�ٽ� ������ ���ʸ� ��ٸ�.

                    erase(zombie[i]);
                    move_zombie(&zombie[i], player);
                    show(zombie[i]);

                    if (is_collide(zombie[i], player)) {
                        life--;
                        gotoxy(45, 7);
                        if (life <= 1)
                            textColor(LIGHTMAGENTA);
                        else
                            textColor(YELLOW);
                        printf("��  ������ :   %d  ��", life);
                        if (life == 0) {
                            gotoxy(EDGE, 30);
                            textColor(YELLOW);
                            currentTime = clock();
                            system("pause");
                            R.runtime = currentTime - startTime;
                            R.score += (int)(R.runtime / 1000) * 5;
                            return R;
                        }
                        //ĳ���Ϳ� �ε����� ���� ����.
                        erase(zombie[i]);
                        zombie[i].create = false;
                    }
                }
            }
        }

        //�Ѿ��� �����̰�, �� �� ����� �浹�ߴ��� Ȯ���ϴ� �ݺ���.
        for (j = 0; j < UNIT; j++) {
            if (bullet[j].create == true) {
                bullet[j].period--;
                if (bullet[j].period <= 0) {
                    bullet[j].period = bullet[j].speed;
                    erase(bullet[j]);
                    temp = bullet[j];
                    move_object(&bullet[j]);

                    //�Ѿ��� ���� ������(�������� ��������) �����.
                    if (bullet[j].x == temp.x && bullet[j].y == temp.y)
                    {
                        erase(bullet[j]);
                        bullet[j].create = false;
                        continue;
                    }
                    show(bullet[j]);

                    for (i = 0; i < UNIT; i++) {
                        if (zombie[i].create == true) {
                            if (is_collide(bullet[j], zombie[i])) {
                                R.score += 10;
                                gotoxy(45, 5);
                                textColor(YELLOW);
                                printf("��  ���� : %5d  ��", R.score + (int)(R.runtime / 1000) * 5);
                                //�Ѿ˰� �ε����� ����, �Ѿ� ����,
                                erase(zombie[i]);
                                erase(bullet[j]);
                                zombie[i].create = false;
                                bullet[j].create = false;
                                break;
                            }
                        }
                    }
                }
            }
        }

        //���� �ð��� ���� ���� �߰�.
        currentTime = clock();
        R.runtime = currentTime - startTime;
        gotoxy(45, 5);
        textColor(YELLOW);
        printf("��  ���� : %5d  ��", R.score + (int)(R.runtime / 1000) * 5);

        //������ �ð� ī��Ʈ.
        delay--;
        if (delay == 0)
        {
            gotoxy(15, 3);
            textColor(YELLOW);
            printf("���� �Ѿ� : %2d/%2d", UNIT, UNIT);
        }
    }
}

bool Gameover(result R) {
    textColor(YELLOW);
    system("cls");
    Sleep(1000);
    outline();
    gotoxy(17, 7);
    printf("[GameOver]");
    gotoxy(12, 13);
    printf("ȹ�� ���� : %d��\n", R.score);
    gotoxy(12, 15);
    printf("���� �ð� : %d.%d��\n\n", (int)(R.runtime / 1000), (int)R.runtime % 1000);
    Sleep(1000);
    gotoxy(10, 20);
    printf("�ٽ� �����Ϸ��� r, �����Ϸ��� e�� �����ּ���.");
    char command = 0;
    while (true) {
        command = getch();
        if ((command != 'r') && (command != 'e'))
            continue;
        else if (command == 'r')
            return true;
        else
            return false;
    }
}

void show(element object) {
    gotoxy(object.x, object.y);
    if (object.id == ZOMBIE)
    {
        textColor(RED);
        printf("��");
    }
    else if (object.id == BULLET)
    {
        textColor(DARKGRAY);
        printf("��");
    }
    else if (object.id == PLAYER)
    {
        textColor(LIGHTCYAN);
        printf("��");
    }
    else;
}

void erase(element object) {
    gotoxy(object.x, object.y);
    printf("  ");
}

void outline() {
    int i = 0;
    gotoxy(EDGE, EDGE);
    for (i = 0; i < 2 * XLIMIT; i++)
        printf("��");

    gotoxy(EDGE, EDGE + YLIMIT);
    for (i = 0; i < 2 * XLIMIT; i++)
        printf("��");

    for (i = EDGE; i < EDGE + YLIMIT; i++) {
        gotoxy(EDGE, i);
        printf("��");
        gotoxy(EDGE + XLIMIT, i);
        printf("��");
    }
    gotoxy(EDGE, EDGE); printf("��");
    gotoxy(EDGE + XLIMIT, EDGE); printf("��");
    gotoxy(EDGE, EDGE + YLIMIT); printf("��");
    gotoxy(EDGE + XLIMIT, EDGE + YLIMIT); printf("��");
}

void move_player(element* player, char move) {
    switch (move) {
    case 72:// ��
        player->direct = 'w';
        break;
    case 75:// ��
        player->direct = 'a';
        break;
    case 77:// ��
        player->direct = 'd';
        break;
    case 80:// ��
        player->direct = 's';
        break;
    default:
        break;
    }
    move_object(player);
}

void textColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

element create_zombie(element zombie, int speed[2]) {
    zombie.create = true;
    zombie.id = ZOMBIE; //���� ����,
    if (rand() % 2)
        zombie.x = XLIMIT + EDGE - 1;
    else
        zombie.x = EDGE + 1;
    if (rand() % 2)
        zombie.y = YLIMIT + EDGE - 1;
    else
        zombie.y = EDGE + 1;
    zombie.period = 0;
    zombie.speed = rand() % (speed[1] - speed[0] + 1) + speed[0]; //��ü�� �������� �ӵ� ����.
    return zombie;
}

void move_zombie(element* zombie, element player)
{
    int xdiff = player.x - zombie->x, ydiff = player.y - zombie->y;
    if (xdiff == 0 && ydiff == 0) //�̹� �� ��ü�� ���� ��ġ�� ����.
        return;
    int c = rand() % (abs(xdiff) + abs(ydiff)); //���� �������� Ȯ�������� ����.
    if (c < abs(xdiff))//���η� �����ϰ���.
    {
        if (xdiff < 0)//ĳ���Ͱ� ������ ���ʿ� ����
            zombie->direct = 'a';
        else if (xdiff > 0)//ĳ���Ͱ� ������ �����ʿ� ����
            zombie->direct = 'd';
    }
    else //���η� �����ϰ���.
    {
        if (ydiff < 0)//ĳ���Ͱ� ������ ���ʿ� ����
            zombie->direct = 'w';
        else if (ydiff > 0)//ĳ���Ͱ� ������ �Ʒ��ʿ� ����
            zombie->direct = 's';
    }
    move_object(zombie);
}

element create_bullet(element bullet, char direct, element player)
{
    bullet.create = true;
    bullet.id = BULLET;
    bullet.x = player.x, bullet.y = player.y;
    bullet.period = 0;
    bullet.speed = 10;
    bullet.direct = direct;
    return bullet;
}

void move_object(element* object)
{
    switch (object->direct) {
    case 'w':// ��
        if (object->y > EDGE + 1)
            object->y--;
        break;
    case 'a':// ��
        if (object->x > EDGE + 1)
            object->x--;
        break;
    case 'd':// ��
        if (object->x < XLIMIT + EDGE - 1)
            object->x++;
        break;
    case 's':// ��
        if (object->y < YLIMIT + EDGE - 1)
            object->y++;
        break;
    default:
        break;
    }
}

void sound_gun(void) {
    PlaySound(NULL, 0, 0); //���� �ѼҸ��� �ִٸ� �Ҹ� ����� ����
    PlaySound(TEXT(GunSound), 0, SND_ASYNC); //�ѼҸ� ���
}

void sound_reload(void) {
    PlaySound(NULL, 0, 0); //���� �����Ҹ��� �ִٸ� �Ҹ� ����� ����
    PlaySound(TEXT(ReloadSound), 0, SND_ASYNC); //�����Ҹ� ���
}