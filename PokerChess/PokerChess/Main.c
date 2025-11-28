#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <conio.h>  // kbhit(), getch()
#include "game.h"
#include "card.h"
#include "check.h"

// return 1 = 정상 입력, 0 = 시간 초과
int TimedTurnInput(GameState* gameState, char* buffer, int bufSize)
{
    int index = 0;
    buffer[0] = '\0';

    int* timeLeft = (gameState->currentTurn == 'w') ?
        &gameState->whiteTime : &gameState->blackTime;

    int lastTick = GetTickCount64();

    while (1)
    {
        // 경과 시간 체크
        int now = GetTickCount64();
        if (now - lastTick >= 1000) {
            lastTick = now;
            (*timeLeft)--; // 남은 시간 감소
        }

        // 화면 출력
        printf("\r[%s] 남은 시간: %d초 | 입력: %s",
            (gameState->currentTurn == 'w' ? "백" : "흑"),
            *timeLeft, buffer);
        fflush(stdout);

        // 시간 초과?
        if (*timeLeft <= 0) {
            printf("\n시간 초과!\n");
            return 0;
        }

        // 논블로킹 키 입력
        if (_kbhit()) {
            char ch = getch();

            if (ch == '\r') {  // 엔터
                buffer[index] = '\0';
                printf("\n");
                return 1;
            }
            else if (ch == '\b') {  // 백스페이스
                if (index > 0) {
                    index--;
                    buffer[index] = '\0';
                }
            }
            else if (index < bufSize - 1 && ch >= 32 && ch < 127) {
                buffer[index++] = ch;
                buffer[index] = '\0';
            }
        }

        Sleep(10);
    }
}


void ClearScreen()
{
    system("cls");
    COORD topLeft = { 0,0 };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), topLeft);
}


int main(void) {
    GameState gameState;
    char inputLine[128];

    InitConsole();
    InitializeGame(&gameState);

    printf("=== 포커 카드 기반 텍스트 체스 ===\n\n");

    while (1) {
        ClearScreen();
        PrintBoard(&gameState);

        // 턴 시작 시 체크/체크메이트 상태 확인
        if (IsInCheck(&gameState, gameState.currentTurn)) {
            if (IsCheckmate(&gameState)) {
                printf("\n### 체크메이트! %s 승리! ###\n",
                    (gameState.currentTurn == 'w') ? "흑" : "백");
                break;
            }
            printf("\n체크 상태입니다\n");
        }

        printf("%s 차례입니다. 수를 입력하세요: ",
            (gameState.currentTurn == 'w') ? "백" : "흑");

        int ok = TimedTurnInput(&gameState, inputLine, sizeof(inputLine));

        if (!ok) {
            // 시간 초과 시 처리
            printf("%s 시간 초과! 턴을 넘깁니다.\n",
                (gameState.currentTurn == 'w') ? "백" : "흑");

            // 패 처리 대신 턴만 넘기기
            gameState.currentTurn = (gameState.currentTurn == 'w') ? 'b' : 'w';
            continue;
        }

        // 정상 입력 후 기존 로직 수행

        // 종료 명령
        if (strcmp(inputLine, "quit") == 0) {
            printf("게임을 종료합니다.\n");
            break;
        }

        char fromSquare[4] = { 0 };
        char toSquare[4] = { 0 };

        if (sscanf(inputLine, "%2s %2s", fromSquare, toSquare) != 2) {
            printf("입력 형식이 올바르지 않습니다. (예: e2 e4)\n\n");
            continue;
        }

        int startRow, startColumn, endRow, endColumn;
        ConvertAlgebraicToIndex(fromSquare, &startRow, &startColumn);
        ConvertAlgebraicToIndex(toSquare, &endRow, &endColumn);

        Card* moving = gameState.board[startRow][startColumn];
        if (!moving) {
            printf("출발 위치에 카드가 없습니다.\n\n");
            continue;
        }

        if (moving->side != gameState.currentTurn) {
            printf("현재 차례의 카드가 아닙니다.\n\n");
            continue;
        }

        if (!CanCardMove(&gameState, startRow, startColumn, endRow, endColumn)) {
            printf("규칙상 불가능한 이동입니다.\n\n");
            continue;
        }

        if (IsMovePuttingSelfInCheck(&gameState, startRow, startColumn, endRow, endColumn)) {
            printf("자신의 왕을 체크 상태로 만드는 수는 둘 수 없습니다.\n\n");
            continue;
        }

        ApplyMove(&gameState, startRow, startColumn, endRow, endColumn);

        // 턴 종료 → 다음 턴으로 넘어가기 직전 인크리먼트 추가
        if (gameState.currentTurn == 'b') {
            // 방금 한 턴 = 백 → 백 시간 +10초
            gameState.whiteTime += 10;
        }
        else {
            // 방금 한 턴 = 흑 → 흑 시간 +10초
            gameState.blackTime += 10;
        }

    }

    return 0;
}
