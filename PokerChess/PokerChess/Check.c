#include "check.h"
#include "card.h"

// side 진영의 왕 위치 찾기
static void FindKing(const GameState* gameState, char side,
    int* kingRow, int* kingCol) {
    *kingRow = -1;
    *kingCol = -1;

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Card* card = gameState->board[r][c];
            if (!card) continue;
            if (card->side == side && card->value == 13) { // K = 13
                *kingRow = r;
                *kingCol = c;
                return;
            }
        }
    }
}

int IsInCheck(const GameState* gameState, char side) {
    int kRow, kCol;
    FindKing(gameState, side, &kRow, &kCol);

    if (kRow == -1) {
        // 왕이 없으면 체크라고 보지 않음
        return 0;
    }

    char enemy = (side == 'w') ? 'b' : 'w';

    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Card* card = gameState->board[r][c];
            if (!card) continue;
            if (card->side == enemy) {
                if (CanCardMove(gameState, r, c, kRow, kCol)) {
                    return 1; // 왕을 공격 가능
                }
            }
        }
    }

    return 0;
}

int IsMovePuttingSelfInCheck(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn) {
    GameState temp = *gameState; // 얕은 복사 (Card* 포인터만 복사)
    char side = gameState->currentTurn;

    // 가상 이동 적용
    temp.board[endRow][endColumn] = temp.board[startRow][startColumn];
    temp.board[startRow][startColumn] = NULL;

    // 이동한 쪽(mySide) 왕이 여전히 체크 상태인지 확인
    if (IsInCheck(&temp, side)) {
        return 1; // 자살수
    }
    return 0;
}

int IsCheckmate(const GameState* gameState) {
    char side = gameState->currentTurn;

    // 체크 상태가 아니면 체크메이트도 아님
    if (!IsInCheck(gameState, side))
        return 0;

    // 자신의 모든 카드에 대해
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            Card* card = gameState->board[r][c];
            if (!card) continue;
            if (card->side != side) continue;

            // 이동 가능한 모든 칸 시도
            for (int tr = 0; tr < BOARD_SIZE; tr++) {
                for (int tc = 0; tc < BOARD_SIZE; tc++) {
                    if (!CanCardMove(gameState, r, c, tr, tc)) continue;

                    if (!IsMovePuttingSelfInCheck(gameState, r, c, tr, tc)) {
                        // 왕을 구할 수 있는 수가 하나라도 있으면 체크메이트가 아님
                        return 0;
                    }
                }
            }
        }
    }

    // 모든 수가 자살수 → 체크메이트
    return 1;
}
