#ifndef GAME_H
#define GAME_H

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "card.h"

#define BOARD_SIZE 8

typedef struct GameState {
    Card* board[BOARD_SIZE][BOARD_SIZE];
    char currentTurn;  // 'w' or 'b'

    int whiteTime;     // 백 남은 시간(sec)
    int blackTime;     // 흑 남은 시간(sec)
} GameState;


// 콘솔 핸들 (색상 출력용)
extern HANDLE hConsole;

// 콘솔 초기화
void InitConsole();

// 공용 유틸
int IsInsideBoard(int row, int column);
void ConvertAlgebraicToIndex(const char* square, int* row, int* column);

// 게임 초기화 및 출력
void InitializeGame(GameState* gameState);
void PrintBoard(const GameState* gameState);

// 이동 적용 (검증은 밖에서 이미 했다는 가정)
void ApplyMove(GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn);

// 카드 색 (side) 가져오기
char GetCardSide(const Card* c);

#endif

