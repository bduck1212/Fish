#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <conio.h>

#define FISHTANK_COUNT 6 // 게임에서 사용할 전체 어항의 개수

typedef struct {
    int waterLevel;    // 어항의 현재 물 높이 (0 ~ 100)
    int isAlive;       // 물고기 생존 여부 (1: 생존, 0: 사망)
    int fishSize;      // 물고기 크기
} FishTank;

int level;                      // 난이도 (1 ~ 5)
FishTank tanks[FISHTANK_COUNT]; // 6개의 어항을 다루기 위한 구조체 배열
FishTank* cursor;               // 플레이어가 현재 선택 중인 어항을 가리키는 구조체 포인터
int cursorIndex = 0;            // 현재 커서가 위치한 배열의 인덱스 (0 ~ 5)

// 함수 프로토타입 선언 (핵심 로직은 배열 주소를 받는 '구조체 포인터' 매개변수 활용)
void initData();
void printfFishes();
void decreaseWater(FishTank* tankArr, long elapsedTime); // 구조체 포인터 사용
int checkAllDead(FishTank* tankArr);                    // 구조체 포인터 사용
void growFishes(FishTank* tankArr);                     // 물고기 성장 함수

int main(void) 
{
    // 시간 계산을 위한 변수들 (초 단위 측정)
    long startTime = time(NULL);     // 게임이 시작된 절대 시간
    long totalElapsedTime = 0;       // 게임 시작 후 흘러간 총 시간
    long prevElapsedTime = 0;        // 직전 흐름에서 기록된 시간 (물 감소 타이밍 체크용)

    initData();                     // 게임 데이터(어항, 물고기) 초기화
    cursor = &tanks[cursorIndex];   // 시작할 때 0번 어항의 주소를 커서 포인터에 대입

    printf("<<< 물고기 기르기 게임 (확장판) >>>\n");
    printf("조작법: j(왼쪽 이동), l(오른쪽 이동), k(물 주기)\n\n");
    _getch(); // 키를 누를 때까지 잠시 대기

    // 조건을 만족해 break가 걸릴 때까지 무한 반복
    while (1) 
    {
        printfFishes(); // 현재 모든 어항의 텍스트 그래픽 출력
        
        // 사용자가 키를 눌렀는지 실시간으로 감지
        if (_kbhit()) 
        { 
            char ch = _getch(); // 블로킹 없이 누른 키 값을 곧바로 가져옴

            if (ch == 'j') // 왼쪽 이동 요청
            {
                if (cursorIndex > 0) cursorIndex--; // 가장 왼쪽(0)이 아니면 인덱스 감소
            }
            else if (ch == 'l') // 오른쪽 이동 요청
            { 
                if (cursorIndex < FISHTANK_COUNT - 1) cursorIndex++; // 가장 오른쪽(5)이 아니면 인덱스 증가
            }
            else if (ch == 'k') // 물 주기 요청
            {
                // 구조체 포인터(cursor)를 활용해 현재 가리키는 어항의 상태를 직접 검사 및 수정
                if (cursor->isAlive == 0) 
                {
                    printf("\n%d번 어항의 물고기가 죽어 물을 줄 수 없습니다!\n", cursorIndex + 1);
                }
                else 
                {
                    printf("\n%d번 어항에 물을 줍니다. (+30)\n", cursorIndex + 1);
                    cursor->waterLevel += 30; // 물 충전
                    if (cursor->waterLevel > 100) cursor->waterLevel = 100; // 최대 수위 제한
                }
            }
            // 이동이나 액션이 끝난 후, 바뀐 인덱스를 바탕으로 커서 포인터 주소를 갱신
            cursor = &tanks[cursorIndex];
        }

        // 매 1초마다 물을 감소시키고 레벨업을 판단
        totalElapsedTime = time(NULL) - startTime; // 총 경과 시간 업데이트
        long diffTime = totalElapsedTime - prevElapsedTime; // 직전 체크 이후 흐른 시간 계산

        if (diffTime >= 1) // 최소 1초 이상 흘렀다면 실행
        {
            decreaseWater(tanks, diffTime); // 구조체 포인터(tanks 배열의 시작 주소)를 전달해 물 감소
            prevElapsedTime = totalElapsedTime; // 기준 시간을 현재 시간으로 동기화

            // 10초마다 게임 레벨이 1씩 상승
            if (totalElapsedTime / 10 > level - 1) 
            {
                level++;
                printf("\n 레벨업! 현재 레벨: %d \n", level);
                
                // 레벨업 시 살아있는 물고기들의 크기를 키움
                growFishes(tanks); 
                
                // 승리 조건 검사
                if (level >= 5) 
                {
                    printf("\n최고 레벨 달성\n");
                    break; // 게임 루프 종료
                }
            }
        }

        // 구조체 포인터를 넘겨 모든 물고기가 죽었는지 판단
        if (checkAllDead(tanks)) 
        {
            printfFishes(); // 마지막으로 전멸한 어항 상태를 보여줌
            printf("\n모든 물고기 사망... 게임 오버\n");
            break; // 게임 루프 종료
        }
        
        // 5. 화면 갱신 및 상단 인터페이스 출력
        system("cls"); // 콘솔 화면을 깨끗이 지움
        printf("진행 시간: %ld초 | 현재 레벨: %d\n", totalElapsedTime, level);
        printf("현재 선택: %d번 어항 (물고기 크기: %d)\n", cursorIndex + 1, cursor->fishSize);
        printf("--------------------------------------------------------------\n");
    }

    printf("\n아무 키나 누르면 종료합니다.");
    _getch();
    return 0;
}

// 초기 데이터 세팅
void initData() 
{
    level = 1; // 1레벨부터 시작
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        tanks[i].waterLevel = 100; // 물은 100으로 가득 채움
        tanks[i].isAlive = 1;       // 모두 살려둠
        tanks[i].fishSize = 1;      // 초기 물고기 크기는 1로 설정
    }
}

// UI 및 어항 상태 출력
void printfFishes() 
{
    // 현재 플레이어의 선택 위치 출력
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        if (i == cursorIndex) printf("   ▼    "); 
        else printf("        ");
    }
    printf("\n");

    // 어항 고유 번호 표시
    for (int i = 1; i <= FISHTANK_COUNT; i++) 
    {
        printf(" %d번어항  ", i);
    }
    printf("\n");

    // 각 어항의 현재 물 높이 출력
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        printf("  W:%3d   ", tanks[i].waterLevel);
    }
    printf("\n");

    // 각 어항 속 물고기의 현재 크기 출력
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        if (tanks[i].isAlive)
            printf("  S:%3d   ", tanks[i].fishSize);
        else
            printf("  S:---   "); // 죽은 물고기는 크기 표시 제외
    }
    printf("\n");

    // 상태 표시
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        if (tanks[i].isAlive == 0) printf("  사망    ");
        else if (tanks[i].waterLevel <= 30) printf("  위험    ");
        else printf("  생존    ");
    }
    printf("\n--------------------------------------------------------------\n");
}

// 시간에 따른 물 감소 처리 (구조체 포인터 활용)
// 매개변수 `tankArr`는 메인에서 보낸 `tanks` 배열의 시작 주소를 받아 포인터로 작동함
void decreaseWater(FishTank* tankArr, long elapsedTime) 
{
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        // 시작 주소 + 인덱스를 통해 각 어항 구조체의 주소를 가져옴
        FishTank* currentTank = tankArr + i;

        if (currentTank->isAlive) // 살아있는 물고기 어항만 수위 감소
        {
            //물고기가 성장해 크기가 커질수록 초당 물 소비량이 증가
            int waterConsumption = level * 2 * currentTank->fishSize * (int)elapsedTime;
            currentTank->waterLevel -= waterConsumption;

            // 물이 바닥나면 사망 처리
            if (currentTank->waterLevel <= 0) 
            {
                currentTank->waterLevel = 0;
                currentTank->isAlive = 0; 
            }
        }
    }
}

// 모든 어항의 전멸 여부 검사 (구조체 포인터 활용)
int checkAllDead(FishTank* tankArr) 
{
    for (int i = 0; i < FISHTANK_COUNT; i++) 
    {
        // (tankArr + i) 주소가 가리키는 멥버(->)에 접근하여 살아있는 개체가 있는지 확인
        if ((tankArr + i)->isAlive == 1) 
        {
            return 0; // 단 한 마리라도 살아 있다면 즉시 0 반환
        }
    }
    return 1; // 생존자가 없다면 1 반환
}

// 레벨업 시 호출되어 물고기 크기를 키우는 함수 (구조체 포인터 활용)
void growFishes(FishTank* tankArr)
{
    for (int i = 0; i < FISHTANK_COUNT; i++)
    {
        FishTank* currentTank = tankArr + i; // 포인터 주소 연산으로 순차 접근
        
        if (currentTank->isAlive) // 살아있는 물고기만 성장시킴
        {
            currentTank->fishSize++; // 구조체 내부의 fishSize 멤버 값을 1 증가
            printf("[%d번 물고기가 성장했습니다. 크기: %d]\n", i + 1, currentTank->fishSize);
        }
    }
}