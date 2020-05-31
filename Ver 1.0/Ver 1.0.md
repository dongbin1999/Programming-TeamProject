```c
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>
#include <time.h>
#include <stdbool.h>

#define EASY 1 //난이도
#define HARD 2 //난이도
#define XLIMIT 35 //가로길이
#define YLIMIT 23//세로길이
#define FRAME 5 //게임 프레임
#define UNIT 20 //물체 최대 개수
#define EDGE 5 //커서 시작점

//글씨 색깔 상수,
#define DARKGRAY 8
#define LIGHTCYAN 11
#define YELLOW 14
#define LIGHTGREEN 10
#define LIGHTMAGENTA 13
#define RED 4

//물체가 무엇인지 구분하는 상수.
const int ZOMBIE = 1;
const int BULLET = 2;
const int PLAYER = 3;

typedef struct result {
    int score; //획득 점수.
    clock_t runtime; //생존 시간.
}result;

typedef struct element {
    int x, y; //x,y좌표.
    int id; //물체의 특성.(좀비/총알/캐릭터)
    int speed; //물체가 떨어지는 속도 = FRAME * speed(ms).
    int period; //period == 0일 때, object가 한 칸 내려감. (speed값을 주기로)
    bool create; //물체의 생성 여부.
    char direct; //물체가 움직이는 방향.
}element;

int start(int highest); //게임 시작 전 메인 화면.
result running_game(int difficulty); //게임 구동 화면.
bool Gameover(result R); //결과창

void gotoxy(int x, int y); //커서 위치 이동.
void show(element object); //물체 출력
void erase(element object); //물체 지우기
void outline(); //게임 화면의 틀 그리기.
void textColor(int color); //글씨 색 조정.
bool is_collide(element drop, element player); //캐릭터와 물체의 충돌 여부.
element create_zombie(element zombie, int speed[2]); //좀비 생성.
element create_bullet(element bullet, char direct, element player); //총알 생성.
void move_player(element* player, char move); //캐릭터의 움직임 구현.
void move_zombie(element* zombie, element player); //좀비 움직임 구현.
void move_object(element* object); //좀비,총알,캐릭터 움직임 구현.

int main() {
    int highest = 0; //최고점수 기록.
    while (true) {
        int difficulty = start(highest); //난이도는 시작화면에서 결정됨.
        srand((unsigned)time(NULL));
        result R = running_game(difficulty);
        highest = max(highest, R.score);
        if (Gameover(R) == false)
            break;
    }
    gotoxy(EDGE, YLIMIT + EDGE + 1); //종료 메시지 출력 위치.
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
    printf("다운로드 : https://github.com/dongbin1999/Programming-TeamProject");
    gotoxy(EDGE, 2);
    printf("연락처(github) : https://dongbin1999.github.io/jekyll/update/Programming-TeamProject/");
    gotoxy(EDGE, 3);
    printf("연락처(email) : dongbin1999@inu.ac.kr");
    gotoxy(17, 7);
    printf("[미니 좀비게임 Ver.1.0]\n\n");
    gotoxy(17, 9);
    printf("현재 최고점수 : %5d점", highest);
    gotoxy(20, 11);
    textColor(LIGHTCYAN);
    printf("[게임 방법]\n");
    gotoxy(7, 13);
    printf("1) 방향키로 캐릭터를 움직여 좀비로부터 피합니다.\n");
    gotoxy(7, 14);
    printf("2) W,A,S,D키로 총알을 발사합니다.\n");
    gotoxy(7, 15);
    printf("3) 좀비를 잡으면 10점을, 생존시간 1초마다 5점을 획득합니다.\n");
    gotoxy(7, 16);
    printf("4) 라이프를 모두 잃으면 패배합니다.\n");
    gotoxy(12, 20);
    textColor(YELLOW);
    printf("난이도를 설정하세요 (쉬움 : 1, 어려움 : 2)"); }

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
    printf("난이도 : %s를 선택하셨습니다.", (difficulty == EASY) ? "쉬움" : "어려움");
    {Sleep(500);
    gotoxy(12, 22);
    printf("아무키나 입력하여 게임을 시작하세요!");
    getch();

    for (int i = 3; i > 0; i--) {
        system("cls");
        printf("%d초 후 게임이 시작됩니다...\n", i);
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
    int i = 0, j = 0, remains = UNIT, delay = 1; //remains : 남은 총알 수, delay : 총 재장전 시간.
    char command = 0;

    //probability : 물체가 생성될 확률.(1/probability), speed[2] : 물체가 떨어지는 속도의 최대/최소 범위.
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
    printf("▤  라이프 :   %d  ▤", life);
    gotoxy(45, 5);
    printf("▤  점수 : %5d  ▤", R.score);

    //초기화 후, 시간측정 시작.
    clock_t startTime, currentTime;
    startTime = clock();

    while (1) {
        //좀비 한마리 생성.
        if (rand() % probability == 0)
            for (i = 0; i < UNIT; i++)
                if (zombie[i].create == false) {
                    zombie[i] = create_zombie(zombie[i], speed);
                    break;
                }

        Sleep(FRAME);
        show(player);

        //총 발사/재장전, 캐릭터 움직임 구현,
        if (kbhit()) {
            command = getch();
            switch (command)
            {
            case 'w':case 'a':case 's':case 'd': //총알을 발사한 경우
                if (delay < 0) { //재장전 시간이 지났으면,
                    remains--; //총알 한 발 감소.
                    if (remains >= 0)
                        bullet[remains] = create_bullet(bullet[remains], command, player);
                    gotoxy(15, 3);
                    textColor(YELLOW);
                    printf("남은 총알 : %2d/%2d", max(0, remains), UNIT);
                }
                break;
            case 72:case 75:case 77:case 80: //캐릭터를 움직인 경우
                erase(player);
                move_player(&player, command);
                show(player);
                break;
            case 'r': //총알 재장전하는 경우
                if (delay < 0) {
                    remains = UNIT; //총알 충전.
                    delay = 300; //재장전 시간 : 1.5초.
                    gotoxy(15, 3);
                    textColor(YELLOW);
                    printf("총알 재장전중....");
                }
                break;
            default:
                break;
            }
        }

        //좀비와 캐릭터 충돌 여부 확인.
        for (i = 0; i < UNIT; i++) {
            if (zombie[i].create == true) {
                zombie[i].period--;
                if (zombie[i].period <= 0) {
                    zombie[i].period = zombie[i].speed;//다시 움직일 차례를 기다림.

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
                        printf("▤  라이프 :   %d  ▤", life);
                        if (life == 0) {
                            gotoxy(EDGE, 30);
                            textColor(YELLOW);
                            currentTime = clock();
                            system("pause");
                            R.runtime = currentTime - startTime;
                            R.score += (int)(R.runtime / 1000) * 5;
                            return R;
                        }
                        //캐릭터와 부딪히면 좀비가 자폭.
                        erase(zombie[i]);
                        zombie[i].create = false;
                    }
                }
            }
        }

        //총알이 움직이고, 그 때 좀비와 충돌했는지 확인하는 반복문.
        for (j = 0; j < UNIT; j++) {
            if (bullet[j].create == true) {
                bullet[j].period--;
                if (bullet[j].period <= 0) {
                    bullet[j].period = bullet[j].speed;
                    erase(bullet[j]);
                    temp = bullet[j];
                    move_object(&bullet[j]);

                    //총알이 벽에 닿으면(움직임이 없었으면) 사라짐.
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
                                printf("▤  점수 : %5d  ▤", R.score + (int)(R.runtime / 1000) * 5);
                                //총알과 부딪히면 좀비, 총알 삭제,
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

        //생존 시간에 따라 점수 추가.
        currentTime = clock();
        R.runtime = currentTime - startTime;
        gotoxy(45, 5);
        textColor(YELLOW);
        printf("▤  점수 : %5d  ▤", R.score + (int)(R.runtime / 1000) * 5);

        //재장전 시간 카운트.
        delay--;
        if (delay == 0)
        {
            gotoxy(15, 3);
            textColor(YELLOW);
            printf("남은 총알 : %2d/%2d", UNIT, UNIT);
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
    printf("획득 점수 : %d점\n", R.score);
    gotoxy(12, 15);
    printf("생존 시간 : %d.%d초\n\n", (int)(R.runtime / 1000), (int)R.runtime % 1000);
    Sleep(1000);
    gotoxy(10, 20);
    printf("다시 시작하려면 r, 종료하려면 e를 눌러주세요.");
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
        printf("♠");
    }
    else if (object.id == BULLET)
    {
        textColor(DARKGRAY);
        printf("⊙");
    }
    else if (object.id == PLAYER)
    {
        textColor(LIGHTCYAN);
        printf("★");
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
        printf("─");

    gotoxy(EDGE, EDGE + YLIMIT);
    for (i = 0; i < 2 * XLIMIT; i++)
        printf("─");

    for (i = EDGE; i < EDGE + YLIMIT; i++) {
        gotoxy(EDGE, i);
        printf("│");
        gotoxy(EDGE + XLIMIT, i);
        printf("│");
    }
    gotoxy(EDGE, EDGE); printf("┌");
    gotoxy(EDGE + XLIMIT, EDGE); printf("┐");
    gotoxy(EDGE, EDGE + YLIMIT); printf("└");
    gotoxy(EDGE + XLIMIT, EDGE + YLIMIT); printf("┘");
}

void move_player(element* player, char move) {
    switch (move) {
    case 72:// ↑
        player->direct = 'w';
        break;
    case 75:// ←
        player->direct = 'a';
        break;
    case 77:// →
        player->direct = 'd';
        break;
    case 80:// ↓
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
    zombie.id = ZOMBIE; //좀비 생성,
    if (rand() % 2)
        zombie.x = XLIMIT + EDGE - 1;
    else
        zombie.x = EDGE + 1;
    if (rand() % 2)
        zombie.y = YLIMIT + EDGE - 1;
    else
        zombie.y = EDGE + 1;
    zombie.period = 0;
    zombie.speed = rand() % (speed[1] - speed[0] + 1) + speed[0]; //물체가 떨어지는 속도 설정.
    return zombie;
}

void move_zombie(element* zombie, element player)
{
    int xdiff = player.x - zombie->x, ydiff = player.y - zombie->y;
    if (xdiff == 0 && ydiff == 0) //이미 두 물체가 같은 위치에 있음.
        return;
    int c = rand() % (abs(xdiff) + abs(ydiff)); //좀비 움직임을 확률적으로 조정.
    if (c < abs(xdiff))//가로로 움직일것임.
    {
        if (xdiff < 0)//캐릭터가 좀비의 왼쪽에 있음
            zombie->direct = 'a';
        else if (xdiff > 0)//캐릭터가 좀비의 오른쪽에 있음
            zombie->direct = 'd';
    }
    else //가로로 움직일것임.
    {
        if (ydiff < 0)//캐릭터가 좀비의 위쪽에 있음
            zombie->direct = 'w';
        else if (ydiff > 0)//캐릭터가 좀비의 아래쪽에 있음
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
    case 'w':// ↑
        if (object->y > EDGE + 1)
            object->y--;
        break;
    case 'a':// ←
        if (object->x > EDGE + 1)
            object->x--;
        break;
    case 'd':// →
        if (object->x < XLIMIT + EDGE - 1)
            object->x++;
        break;
    case 's':// ↓
        if (object->y < YLIMIT + EDGE - 1)
            object->y++;
        break;
    default:
        break;
    }
}
```

