// 작성자: 김동규
// 설명: 체크, 체크메이트, 자살수 방지 로직 구현
// 의존성: GameState 구조체, 기본 이동 규칙 함수들(IsLegal*Move), ApplyMove

/*
[ 병합 가이드 ]

이 파일은 단독 실행 불가. 병합 코드에 복붙 해야 함.

병합 순서

1. [1단계: 코드 복사]
   - 아래 #include 부터 파일 끝까지 복사.
   - 메인 파일 GameState 구조체 정의 바로 아래, main 함수 위에 붙여넣기.
   - (중복되는 extern 선언이나 include는 지워도 됨.)

2. [2단계: 자살수 방지 연결 (IsLegalMove 함수 안)]
   - 메인 파일 IsLegalMove 함수 찾기.
   - 함수 맨 끝 return 1; (성공 반환) 바로 윗줄에 아래 코드 추가.

   왕 잡히는 곳 이동 불가 (자살수 방지)
   if (IsMovePuttingSelfInCheck(gameState, startRow, startColumn, endRow, endColumn)) {
       printf("왕이 위험해지는 곳으로는 갈 수 없습니다!\n");
       return 0;
   }

3. [3단계: 승패 판정 연결 (main 함수 안)]
   - main 함수 while(1) 반복문 안으로 이동.
   - PrintBoard(&gameState); 바로 다음 줄에 아래 코드 추가.

   턴 시작 시 체크/체크메이트 확인
   if (IsInCheck(&gameState, gameState.currentTurn)) {
       if (IsCheckmate(&gameState)) {
           printf("\n### 체크메이트! %s 승리! ###\n", (gameState.currentTurn == 'w') ? "흑" : "백");
           break; // 게임 종료
       }
       printf("\n!! 체크 상태입니다 (왕을 보호하세요) !!\n");
   }
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

// [1] 구조체 및 외부 의존성 선언
// (병합 시 중복 방지 위해 ifndef 처리)
#ifndef BOARD_SIZE
#define BOARD_SIZE 8
#endif

#ifndef GAME_STATE_DEFINED
#define GAME_STATE_DEFINED
typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];
    char currentTurn;
} GameState;
#endif

// 다른 파일(팀원 코드)에 정의된 함수들
// 병합 시 이미 메인 코드에 있다면 extern 지워도 됨
extern int IsInsideBoard(int row, int column);
extern char GetPieceSide(char piece);
extern void ApplyMove(GameState* gameState, int startRow, int startColumn, int endRow, int endColumn);

// [외부 함수] 이동 규칙 (병합된 파일에 존재해야 함)
extern int IsLegalPawnMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn, char side);
extern int IsLegalJumperMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn, char side);
extern int IsLegalAreaMoverMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn, char side);
extern int IsLegalRookMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn, char side);
extern int IsLegalQueenMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn, char side);
extern int IsLegalKingMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn, char side);

// [2] 핵심 로직 구현

// 함수: 이동 가능 여부 확인 (printf 없음, 시뮬레이션용)
int CanPieceMove(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn) {
    if (!IsInsideBoard(startRow, startColumn) || !IsInsideBoard(endRow, endColumn)) return 0;

    char movingPiece = gameState->board[startRow][startColumn];
    if (movingPiece == '.') return 0;

    char targetPiece = gameState->board[endRow][endColumn];
    char movingSide = GetPieceSide(movingPiece);

    // 팀킬 방지
    if (targetPiece != '.' && GetPieceSide(targetPiece) == movingSide) return 0;

    char lowerPiece = (char)tolower((unsigned char)movingPiece);
    switch (lowerPiece) {
    case 'p': return IsLegalPawnMove(gameState, startRow, startColumn, endRow, endColumn, movingSide);
    case 'j': return IsLegalJumperMove(gameState, startRow, startColumn, endRow, endColumn, movingSide);
    case 'a': return IsLegalAreaMoverMove(gameState, startRow, startColumn, endRow, endColumn, movingSide);
    case 'r': return IsLegalRookMove(gameState, startRow, startColumn, endRow, endColumn, movingSide);
    case 'q': return IsLegalQueenMove(gameState, startRow, startColumn, endRow, endColumn, movingSide);
    case 'k': return IsLegalKingMove(gameState, startRow, startColumn, endRow, endColumn, movingSide);
    default: return 0;
    }
}

// 함수: 특정 진영 킹 위치 탐색
void FindKing(const GameState* gameState, char side, int* kingRow, int* kingCol) {
    char targetKing = (side == 'w') ? 'K' : 'k';
    *kingRow = -1;
    *kingCol = -1;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (gameState->board[i][j] == targetKing) {
                *kingRow = i;
                *kingCol = j;
                return;
            }
        }
    }
}

// 함수: 체크 판정 (side 진영 왕이 공격받는 상태인가)
int IsInCheck(const GameState* gameState, char side) {
    int kRow, kCol;
    FindKing(gameState, side, &kRow, &kCol);

    if (kRow == -1) return 0; // 왕 없음

    char enemySide = (side == 'w') ? 'b' : 'w';

    // 적군 모든 기물이 내 왕 공격 가능한지 검사
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            char piece = gameState->board[r][c];
            if (GetPieceSide(piece) == enemySide) {
                if (CanPieceMove(gameState, r, c, kRow, kCol)) {
                    return 1; // 공격 가능 (체크)
                }
            }
        }
    }
    return 0;
}

// 함수: 자살수 방지 (이동 후 자신 왕이 체크 상태 되는지 확인)
int IsMovePuttingSelfInCheck(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn) {
    GameState tempState = *gameState;
    char mySide = tempState.currentTurn;

    // 가상 이동 실행 (ApplyMove는 턴 넘기므로 주의)
    ApplyMove(&tempState, startRow, startColumn, endRow, endColumn);

    // 이동한 쪽(mySide) 왕이 여전히 체크 상태인지 확인
    if (IsInCheck(&tempState, mySide)) {
        return 1; // 자살수 (불법)
    }
    return 0;
}

// 함수: 체크메이트 판정 (현재 턴 플레이어 패배 여부)
int IsCheckmate(const GameState* gameState) {
    char side = gameState->currentTurn;

    if (!IsInCheck(gameState, side)) return 0; // 체크 아니면 체크메이트 아님

    // 내 모든 기물의 모든 이동 경로 시뮬레이션
    for (int r = 0; r < BOARD_SIZE; r++) {
        for (int c = 0; c < BOARD_SIZE; c++) {
            if (GetPieceSide(gameState->board[r][c]) == side) {
                for (int tr = 0; tr < BOARD_SIZE; tr++) {
                    for (int tc = 0; tc < BOARD_SIZE; tc++) {
                        // 1. 이동 가능하고
                        if (CanPieceMove(gameState, r, c, tr, tc)) {
                            // 2. 그 이동이 체크 해소한다면
                            if (!IsMovePuttingSelfInCheck(gameState, r, c, tr, tc)) {
                                return 0; // 살 길 있음 (게임 진행)
                            }
                        }
                    }
                }
            }
        }
    }
    return 1; // 모든 수 실패 (체크메이트)
}

// 함수: 최종 이동 검증 (Main 호출용 래퍼)
// 반환값: 1(가능), 0(불가능)
// 설명: 병합 시 이 함수 호출하거나, 기존 IsLegalMove에 내부 로직 복사
int IsLegalMove_CheckLogic(const GameState* gameState, int startRow, int startColumn, int endRow, int endColumn) {
    // 1. 규칙상 이동 불가
    if (!CanPieceMove(gameState, startRow, startColumn, endRow, endColumn)) {
        return 0;
    }

    // 2. 자살수(체크 방치) 불가
    if (IsMovePuttingSelfInCheck(gameState, startRow, startColumn, endRow, endColumn)) {
        return 0;
    }

    return 1;
}