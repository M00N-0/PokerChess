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

    printf("--------------- 포커 카드 기반 텍스트 체스 --------------\n");
    printf("--------------------  게임 규칙 요약  -------------------\n");

    // 1. 기본 구조
    printf("-- 1. 기본 구조 (체스 기반)\n");
    printf("- 목표: 상대 킹을 체크메이트 하세요.\n");
    printf("- 체크: 왕이 공격받는 상태. 다음 수에 반드시 방어해야 합니다.\n");
    printf("- 자살수 방지: 자신의 왕이 체크 상태가 되는 이동은 불가능합니다.\n");
    printf("- 무승부: 킹이 체크가 아니지만 합법적인 수가 없는 경우 무승부입니다.\n\n");

    // 2. 카드별 이동 규칙 (Piece Rule)
    printf("-- 2. 카드별 기본이동 규칙\n");
    printf("- Q(퀸)/K(킹): 기본 체스 퀸/킹의 이동 규칙을 따릅니다.\n");
    printf("- A(에어리어): 자신의 주변 5x5 범위 내에서 최대 2칸 이동 가능 (점프 불가).\n");
    printf("- J(점퍼): 기본 나이트 이동 (L자) + 상하좌우 3칸 직선 점프 가능.\n");
    printf("- 2~10: 기본 체스 폰과 같은 1칸 이동 (단, ◆ 문양은 항상 2칸 이동 가능).\n");

    // 3. 문양별 추가 능력 (Suit Ability)
    printf("-- 3. 문양별 특별 능력 (게임 중 언제든 적용)\n");
    printf("- ♠ 스페이드: (단 1회) 상대 킹 체크메이트 시, 본인에게 2번의 연속 턴 부여.\n");
    printf("- ♥ 하트: J 카드가 상하좌우로 1칸, 2칸씩도 이동 가능하게 함.\n");
    printf("- ◆ 다이아몬드: 숫자 카드 (2~10)는 항상 2칸 이동 가능하게 함.\n");
    printf("- ♣ 클럽: (게임 시작 시) 2번의 연속 턴을 가지고 시작.\n");
    printf("----------------------------------------------------------\n\n");

    printf("입력 예시: e2 e4 (시작위치 도착위치)\n\n");

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
