#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "card.h"
#include "game.h"

// 문양 출력
static const char* SuitSymbol(Suit s) {
    switch (s) {
    case SPADE:   return "♠";
    case HEART:   return "♥";
    case DIAMOND: return "◆";
    case CLUB:    return "♣";
    default:      return "?";
    }
}

// 값 출력
static const char* ValueSymbol(int v) {
    static char buf[3];
    if (v == 1)  return "A";
    if (v == 11) return "J";
    if (v == 12) return "Q";
    if (v == 13) return "K";
    snprintf(buf, sizeof(buf), "%d", v);
    return buf;
}

void PrintCard(const Card* c) {
    if (!c) {
        printf(" . ");
        return;
    }

    char out[4] = { 0 };
    const char* s = SuitSymbol(c->suit);
    const char* v = ValueSymbol(c->value);

    // s는 3바이트 이상일 수 있지만, 콘솔에서 대충 맞게 보일 거라 단순 결합
    snprintf(out, sizeof(out), "%s", s); // 첫 글자만 써도 OK
    printf("%s%s", out, v);
    if (v[1] == '\0') printf(" "); // 1자리 숫자는 뒤에 공백 하나
}

// 랜덤 범위 [min, max]
static int RandRange(int min, int max) {
    return min + rand() % (max - min + 1);
}

Card* CreateRandomCard(char side) {
    Card* c = (Card*)malloc(sizeof(Card));
    if (!c) return NULL;
    c->suit = (Suit)RandRange(0, 3); // 0~3
    c->value = RandRange(1, 11);     // 1~11 (A~J)
    c->side = side;
    return c;
}

Card* CreateFixedCard(char side, int value) {
    Card* c = (Card*)malloc(sizeof(Card));
    if (!c) return NULL;
    c->suit = (Suit)RandRange(0, 3); // 문양은 랜덤
    c->value = value;
    c->side = side;
    return c;
}

// 경로 검사: 직선
static int IsPathClearStraight(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn) {
    int rowStep = 0;
    int colStep = 0;
    int row, col;

    if (startRow == endRow) {
        colStep = (endColumn > startColumn) ? 1 : -1;
        for (col = startColumn + colStep; col != endColumn; col += colStep) {
            if (gameState->board[startRow][col] != NULL)
                return 0;
        }
        return 1;
    }
    else if (startColumn == endColumn) {
        rowStep = (endRow > startRow) ? 1 : -1;
        for (row = startRow + rowStep; row != endRow; row += rowStep) {
            if (gameState->board[row][startColumn] != NULL)
                return 0;
        }
        return 1;
    }
    return 0;
}

// 경로 검사: 대각
static int IsPathClearDiagonal(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn) {
    int rowDiff = endRow - startRow;
    int colDiff = endColumn - startColumn;

    if (rowDiff == 0 || colDiff == 0) return 0;
    if (rowDiff * rowDiff != colDiff * colDiff) return 0;

    int rowStep = (rowDiff > 0) ? 1 : -1;
    int colStep = (colDiff > 0) ? 1 : -1;

    int row = startRow + rowStep;
    int col = startColumn + colStep;

    while (row != endRow && col != endColumn) {
        if (gameState->board[row][col] != NULL)
            return 0;
        row += rowStep;
        col += colStep;
    }
    return 1;
}

// 카드 이동 규칙
int CanCardMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn) {
    if (!IsInsideBoard(startRow, startColumn) ||
        !IsInsideBoard(endRow, endColumn))
        return 0;

    Card* moving = gameState->board[startRow][startColumn];
    if (!moving) return 0;

    Card* target = gameState->board[endRow][endColumn];
    char side = moving->side;

    // 같은 편 캡처 불가
    if (target && target->side == side)
        return 0;

    int dr = endRow - startRow;
    int dc = endColumn - startColumn;
    int adr = (dr < 0) ? -dr : dr;
    int adc = (dc < 0) ? -dc : dc;

    // 자기 자리 제자리 이동 불가
    if (adr == 0 && adc == 0)
        return 0;

    int v = moving->value;

    // A(1): 5x5, 최대 2칸, 점프 불가
    if (v == 1) {
        if (adr > 2 || adc > 2) return 0;
        int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
        int r = startRow + stepR;
        int c = startColumn + stepC;
        while (r != endRow || c != endColumn) {
            if (gameState->board[r][c] != NULL)
                return 0;
            r += stepR;
            c += stepC;
        }
        return 1;
    }

    // J(11): 나이트 + 상하좌우 3칸 점프
    if (v == 11) {
        if ((adr == 2 && adc == 1) || (adr == 1 && adc == 2))
            return 1;
        if ((adr == 3 && adc == 0) || (adr == 0 && adc == 3))
            return 1;
        return 0;
    }

    // Q(12): 체스 퀸
    if (v == 12) {
        if (IsPathClearStraight(gameState, startRow, startColumn, endRow, endColumn))
            return 1;
        if (IsPathClearDiagonal(gameState, startRow, startColumn, endRow, endColumn))
            return 1;
        return 0;
    }

    // K(13): 기본 1칸, ♥일 경우 최대 2칸
    if (v == 13) {
        int limit = (moving->suit == HEART) ? 2 : 1;
        if (adr > limit || adc > limit) return 0;

        // 2칸 이상이면 경로 체크
        if (adr > 1 || adc > 1) {
            int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
            int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
            int r = startRow + stepR;
            int c = startColumn + stepC;
            while (r != endRow || c != endColumn) {
                if (gameState->board[r][c] != NULL)
                    return 0;
                r += stepR;
                c += stepC;
            }
        }
        return 1;
    }

    // 2~10
    if (v >= 2 && v <= 10) {
        int base = 1;
        if (moving->suit == DIAMOND)
            base = 2; // ♦이면 항상 2칸

        if (adr > base || adc > base) return 0;

        // 2칸 이동이면 경로 체크
        if (base == 2 && (adr == 2 || adc == 2)) {
            int stepR = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
            int stepC = (dc == 0) ? 0 : (dc > 0 ? 1 : -1);
            int r = startRow + stepR;
            int c = startColumn + stepC;
            while (r != endRow || c != endColumn) {
                if (gameState->board[r][c] != NULL)
                    return 0;
                r += stepR;
                c += stepC;
            }
        }

        return 1;
    }

    return 0;
}
