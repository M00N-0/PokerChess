#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

HANDLE hConsole;  // 전역 콘솔 핸들

void InitConsole() {
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand((unsigned int)time(NULL));
}

void SetColor(int text, int background) {
    SetConsoleTextAttribute(hConsole, text + (background << 4));
}

int IsInsideBoard(int row, int column) {
    return row >= 0 && row < BOARD_SIZE && column >= 0 && column < BOARD_SIZE;
}

void ConvertAlgebraicToIndex(const char* square, int* row, int* column) {
    // square: "e2"
    *column = square[0] - 'a';      // 'a' -> 0
    *row = 8 - (square[1] - '0');   // '1' -> 7, '8' -> 0
}

char GetCardSide(const Card* c) {
    if (!c) return 0;
    return c->side;
}

// 초기 배치:
// row 0,1 : 흑 ('b')
// row 6,7 : 백 ('w')
// row 0,7 : K/Q 고정 위치, 나머지는 랜덤
// row 1,6 : 전부 랜덤
void InitializeGame(GameState* gameState) {
    // 타이머 초기회
    gameState->whiteTime = 30; // 30초
    gameState->blackTime = 30;

    // 전부 NULL 초기화
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            gameState->board[r][c] = NULL;
        }
    }

    // 흑(back rank) row 0
    for (int c = 0; c < BOARD_SIZE; c++) {
        if (c == 3) {
            gameState->board[0][c] = CreateFixedCard('b', 12); // Q
        }
        else if (c == 4) {
            gameState->board[0][c] = CreateFixedCard('b', 13); // K
        }
        else {
            gameState->board[0][c] = CreateRandomCard('b');
        }
    }

    // 흑 second rank row 1
    for (int c = 0; c < BOARD_SIZE; c++) {
        gameState->board[1][c] = CreateRandomCard('b');
    }

    // 백 second rank row 6
    for (int c = 0; c < BOARD_SIZE; c++) {
        gameState->board[6][c] = CreateRandomCard('w');
    }

    // 백 back rank row 7
    for (int c = 0; c < BOARD_SIZE; c++) {
        if (c == 3) {
            gameState->board[7][c] = CreateFixedCard('w', 12); // Q
        }
        else if (c == 4) {
            gameState->board[7][c] = CreateFixedCard('w', 13); // K
        }
        else {
            gameState->board[7][c] = CreateRandomCard('w');
        }
    }

    gameState->currentTurn = 'w';
}

// 보드 출력 (색상 포함)
void PrintBoard(const GameState* gameState) {
    printf("    a    b    c    d    e    f    g    h\n");
    for (int row = 0; row < BOARD_SIZE; row++) {
        printf("%d ", 8 - row);
        for (int col = 0; col < BOARD_SIZE; col++) {
            int isWhiteSquare = ((row + col) % 2 == 0);
            if (isWhiteSquare)
                SetColor(0, 15);  // 검정색 글자, 흰색 배경
            else
                SetColor(15, 0);  // 흰색 글자, 검정색 배경

            printf(" ");
            PrintCard(gameState->board[row][col]);
            printf(" ");

            SetColor(15, 0); // 기본색 복구
        }
        printf(" %d\n", 8 - row);
    }
    printf("    a    b    c    d    e    f    g    h\n\n");
}

// 이동 적용 
void ApplyMove(GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn) {
    Card* moving = gameState->board[startRow][startColumn];

    gameState->board[endRow][endColumn] = moving;
    gameState->board[startRow][startColumn] = NULL;

    // 턴 교대
    gameState->currentTurn = (gameState->currentTurn == 'w') ? 'b' : 'w';
}