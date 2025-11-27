#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "card.h"
#include "check.h"

int main(void) {
    GameState gameState;
    char inputLine[128];

    InitConsole();
    InitializeGame(&gameState);

    printf("=== 포커 카드 기반 텍스트 체스 ===\n\n");

    while (1) {
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

        if (!fgets(inputLine, sizeof(inputLine), stdin)) {
            break;
        }

        // 개행 제거
        size_t length = strlen(inputLine);
        if (length > 0 &&
            (inputLine[length - 1] == '\n' || inputLine[length - 1] == '\r')) {
            inputLine[length - 1] = '\0';
        }

        if (strcmp(inputLine, "quit") == 0) {
            printf("게임을 종료합니다.\n");
            break;
        }

        if (inputLine[0] == '\0') {
            continue;
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

        // 순수 규칙상 이동 가능?
        if (!CanCardMove(&gameState, startRow, startColumn, endRow, endColumn)) {
            printf("규칙상 불가능한 이동입니다.\n\n");
            continue;
        }

        // 자살수 체크
        if (IsMovePuttingSelfInCheck(&gameState, startRow, startColumn, endRow, endColumn)) {
            printf("자신의 왕을 체크 상태로 만드는 수는 둘 수 없습니다.\n\n");
            continue;
        }

        // 실제 이동 적용
        ApplyMove(&gameState, startRow, startColumn, endRow, endColumn);
        printf("\n");
    }

    return 0;
}
