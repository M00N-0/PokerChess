#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BOARD_SIZE 8

typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];  // '.' 빈칸, 백: 대문자, 흑: 소문자
    char currentTurn;                    // 'w' 또는 'b'
} GameState;

// ---------- 유틸 함수 ----------

int IsInsideBoard(int row, int column) {
    return row >= 0 && row < BOARD_SIZE && column >= 0 && column < BOARD_SIZE;
}

int IsWhitePiece(char piece) {
    return (piece >= 'A' && piece <= 'Z');
}

int IsBlackPiece(char piece) {
    return (piece >= 'a' && piece <= 'z');
}

char GetPieceSide(char piece) {
    if (piece == '.') return 0;
    return IsWhitePiece(piece) ? 'w' : 'b';
}

void ConvertAlgebraicToIndex(const char* square, int* row, int* column) {
    // square 예: "e2"
    *column = tolower(square[0]) - 'a';
    *row = 8 - (square[1] - '0');
}

void PrintBoard(const GameState* gameState) {
    int row, column;

    printf("  a b c d e f g h\n");
    for (row = 0; row < BOARD_SIZE; row++) {
        printf("%d ", 8 - row);
        for (column = 0; column < BOARD_SIZE; column++) {
            printf("%c ", gameState->board[row][column]);
        }
        printf("%d\n", 8 - row);
    }
    printf("  a b c d e f g h\n\n");
}

void InitializeGame(GameState* gameState) {
    // 1열/8열: R N B Q K B N R 의 비숍→A, 나이트→J로 변경
    // 8: 흑 기본 배치, 1: 백 기본 배치
    const char* startPosition[BOARD_SIZE] = {
        "rjaqkajr",   // 8행: r j a q k a j r
        "pppppppp",   // 7행: 폰 (설명상 2~10 역할)
        "........",
        "........",
        "........",
        "........",
        "PPPPPPPP",   // 2행: 폰 (설명상 2~10 역할)
        "RJAQKAJR"    // 1행: R J A Q K A J R
    };

    int row, column;
    for (row = 0; row < BOARD_SIZE; row++) {
        for (column = 0; column < BOARD_SIZE; column++) {
            gameState->board[row][column] = startPosition[row][column];
        }
    }

    gameState->currentTurn = 'w';
}

// ---------- 경로 검사(룩/퀸/커스텀용) ----------

int IsPathClearStraight(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn)
{
    int rowStep = 0;
    int columnStep = 0;
    int row, column;

    if (startRow == endRow) {
        columnStep = (endColumn > startColumn) ? 1 : -1;
        for (column = startColumn + columnStep; column != endColumn; column += columnStep) {
            if (gameState->board[startRow][column] != '.') {
                return 0;
            }
        }
        return 1;
    }
    else if (startColumn == endColumn) {
        rowStep = (endRow > startRow) ? 1 : -1;
        for (row = startRow + rowStep; row != endRow; row += rowStep) {
            if (gameState->board[row][startColumn] != '.') {
                return 0;
            }
        }
        return 1;
    }

    return 0; // 직선이 아니면 여기 오지 않음
}

int IsPathClearDiagonal(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn)
{
    int rowDifference = endRow - startRow;
    int columnDifference = endColumn - startColumn;

    if (rowDifference == 0 || columnDifference == 0) {
        return 0; // 대각선이 아님
    }
    if (rowDifference * rowDifference != columnDifference * columnDifference) {
        return 0; // |rowDifference| != |columnDifference| 경우
    }

    int rowStep = (rowDifference > 0) ? 1 : -1;
    int columnStep = (columnDifference > 0) ? 1 : -1;

    int row = startRow + rowStep;
    int column = startColumn + columnStep;

    while (row != endRow && column != endColumn) {
        if (gameState->board[row][column] != '.') {
            return 0;
        }
        row += rowStep;
        column += columnStep;
    }

    return 1;
}

// ---------- 말별 이동 규칙 검사 ----------

// 폰(P/p) : 기본 체스 폰 역할
int IsLegalPawnMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn,
    char side)
{
    int direction = (side == 'w') ? -1 : 1;      // 백은 위로(-1), 흑은 아래로(+1)
    int startRowForPawn = (side == 'w') ? 6 : 1; // 백 폰 시작행 6, 흑 폰 시작행 1

    char targetPiece = gameState->board[endRow][endColumn];

    int rowDifference = endRow - startRow;
    int columnDifference = endColumn - startColumn;

    // 1) 전진(앞으로만, 같은 파일)
    if (columnDifference == 0) {
        // 한 칸 전진
        if (rowDifference == direction && targetPiece == '.') {
            return 1;
        }
        // 첫 시작 위치에서 두 칸 전진
        if (startRow == startRowForPawn &&
            rowDifference == 2 * direction &&
            targetPiece == '.' &&
            gameState->board[startRow + direction][startColumn] == '.')
        {
            return 1;
        }
        return 0;
    }

    // 2) 대각선으로 한 칸 이동하며 잡기
    if (rowDifference == direction && (columnDifference == 1 || columnDifference == -1)) {
        if (targetPiece != '.' && GetPieceSide(targetPiece) != side) {
            return 1;
        }
    }

    return 0;
}

// J/j : 기본 나이트 + 상하좌우 3칸
int IsLegalJumperMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn,
    char side)
{
    (void)gameState;
    (void)side;

    int rowDifference = endRow - startRow;
    int columnDifference = endColumn - startColumn;

    int absRowDifference = (rowDifference < 0) ? -rowDifference : rowDifference;
    int absColumnDifference = (columnDifference < 0) ? -columnDifference : columnDifference;

    // 기본 나이트 이동
    if ((absRowDifference == 2 && absColumnDifference == 1) ||
        (absRowDifference == 1 && absColumnDifference == 2)) {
        return 1;
    }

    // 상하좌우 3칸 (점프 가능)
    if ((absRowDifference == 3 && absColumnDifference == 0) ||
        (absRowDifference == 0 && absColumnDifference == 3)) {
        return 1;
    }

    return 0;
}

// A/a : 주변 5×5 (최대 2칸 이내 어디든, 점프 불가)
int IsLegalAreaMoverMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn,
    char side)
{
    (void)side;

    int rowDifference = endRow - startRow;
    int columnDifference = endColumn - startColumn;

    int absRowDifference = (rowDifference < 0) ? -rowDifference : rowDifference;
    int absColumnDifference = (columnDifference < 0) ? -columnDifference : columnDifference;

    // 자기 칸은 불가
    if (absRowDifference == 0 && absColumnDifference == 0) {
        return 0;
    }

    // 5x5 범위 → 최대 2칸 이내
    if (absRowDifference > 2 || absColumnDifference > 2) {
        return 0;
    }

    // 이동 방향이 직선/대각 어느 것이든 허용, 대신 사이에 말이 있으면 안 됨
    // (킹의 강화판 느낌, 2칸까지 확장 + 점프 불가)
    int stepRow = (rowDifference == 0) ? 0 : (rowDifference > 0 ? 1 : -1);
    int stepColumn = (columnDifference == 0) ? 0 : (columnDifference > 0 ? 1 : -1);

    int currentRow = startRow + stepRow;
    int currentColumn = startColumn + stepColumn;

    while (currentRow != endRow || currentColumn != endColumn) {
        if (gameState->board[currentRow][currentColumn] != '.') {
            return 0;
        }
        currentRow += stepRow;
        currentColumn += stepColumn;
    }

    return 1;
}

// 룩(R/r)
int IsLegalRookMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn,
    char side)
{
    (void)side; // 색상은 여기서는 사용하지 않지만 형태 통일을 위해 남김

    if (!IsPathClearStraight(gameState, startRow, startColumn, endRow, endColumn)) {
        return 0;
    }
    return 1;
}

// 퀸(Q/q)
int IsLegalQueenMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn,
    char side)
{
    (void)side; // 색상은 여기서는 사용하지 않지만 형태 통일을 위해 남김

    if (IsPathClearStraight(gameState, startRow, startColumn, endRow, endColumn)) {
        return 1;
    }
    if (IsPathClearDiagonal(gameState, startRow, startColumn, endRow, endColumn)) {
        return 1;
    }
    return 0;
}

// 킹(K/k)
int IsLegalKingMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn,
    char side)
{
    (void)gameState;
    (void)side;

    int rowDifference = endRow - startRow;
    int columnDifference = endColumn - startColumn;

    int absRowDifference = (rowDifference < 0) ? -rowDifference : rowDifference;
    int absColumnDifference = (columnDifference < 0) ? -columnDifference : columnDifference;

    // 한 칸 이내로만 이동
    if (absRowDifference <= 1 && absColumnDifference <= 1 &&
        !(absRowDifference == 0 && absColumnDifference == 0)) {
        return 1;
    }
    return 0;
}

// ---------- 전체 이동 합법성 검사 ----------

int IsLegalMove(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn)
{
    if (!IsInsideBoard(startRow, startColumn) || !IsInsideBoard(endRow, endColumn)) {
        return 0;
    }

    char movingPiece = gameState->board[startRow][startColumn];
    char targetPiece = gameState->board[endRow][endColumn];

    if (movingPiece == '.') {
        printf("출발 칸에 말이 없습니다.\n");
        return 0;
    }

    char movingSide = GetPieceSide(movingPiece);

    if (movingSide != gameState->currentTurn) {
        printf("현재 차례의 말이 아닙니다.\n");
        return 0;
    }

    if (targetPiece != '.' && GetPieceSide(targetPiece) == movingSide) {
        printf("자기 말이 있는 칸으로는 이동할 수 없습니다.\n");
        return 0;
    }

    // 말 종류에 따른 규칙 분기
    char lowerPiece = (char)tolower((unsigned char)movingPiece);

    switch (lowerPiece) {
    case 'p':   // 폰: 2~10 말의 역할이라고 보면 됨
        if (!IsLegalPawnMove(gameState, startRow, startColumn, endRow, endColumn, movingSide)) {
            return 0;
        }
        break;
    case 'j':   // J : 나이트 + 상하좌우 3칸 점프
        if (!IsLegalJumperMove(gameState, startRow, startColumn, endRow, endColumn, movingSide)) {
            return 0;
        }
        break;
    case 'a':   // A : 주변 5x5 (최대 2칸), 점프 불가
        if (!IsLegalAreaMoverMove(gameState, startRow, startColumn, endRow, endColumn, movingSide)) {
            return 0;
        }
        break;
    case 'r':   // 룩
        if (!IsLegalRookMove(gameState, startRow, startColumn, endRow, endColumn, movingSide)) {
            return 0;
        }
        break;
    case 'q':   // 퀸
        if (!IsLegalQueenMove(gameState, startRow, startColumn, endRow, endColumn, movingSide)) {
            return 0;
        }
        break;
    case 'k':   // 킹
        if (!IsLegalKingMove(gameState, startRow, startColumn, endRow, endColumn, movingSide)) {
            return 0;
        }
        break;
    default:
        return 0;
    }

    return 1;
}

// ---------- 실제 말 이동 적용 ----------

void ApplyMove(GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn)
{
    char movingPiece = gameState->board[startRow][startColumn];
    char lowerPiece = (char)tolower((unsigned char)movingPiece);

    // 이동
    gameState->board[endRow][endColumn] = movingPiece;
    gameState->board[startRow][startColumn] = '.';

    // 폰이 마지막 rank에 도달하면 자동 퀸 승격
    if (lowerPiece == 'p') {
        if (endRow == 0 || endRow == 7) {
            if (IsWhitePiece(movingPiece)) {
                gameState->board[endRow][endColumn] = 'Q';
            }
            else {
                gameState->board[endRow][endColumn] = 'q';
            }
        }
    }

    // 턴 변경
    gameState->currentTurn = (gameState->currentTurn == 'w') ? 'b' : 'w';
}

// ---------- 메인 ----------

int main(void) {
    GameState gameState;
    char inputLine[128];

    InitializeGame(&gameState);

    printf("=== 커스텀 텍스트 체스 ===\n");
    printf("백: 대문자 / 흑: 소문자\n");
    printf("말 규칙:\n");
    printf("  R/r : 룩 (기본 체스와 동일)\n");
    printf("  A/a : 주변 5x5 범위(최대 2칸) 이동, 점프 불가\n");
    printf("  J/j : 나이트 이동 + 상/하/좌/우 3칸 점프 이동\n");
    printf("  P/p : 기본 체스 폰 역할 (설정상 2~10 말 나중에 수정하겠습니다.)\n");
    printf("  Q/q : 퀸, K/k : 킹 (기본 체스와 동일)\n");
    printf("체크, 체크메이트, 캐슬링, 앙파상은 구현하지 않았습니다.\n\n");
    printf("입력 예) e2 e4  /  g7 g5  /  quit\n\n");

    while (1) {
        PrintBoard(&gameState);

        printf("%s 차례입니다. 수를 입력하세요: ",
            (gameState.currentTurn == 'w') ? "백" : "흑");

        if (!fgets(inputLine, sizeof(inputLine), stdin)) {
            break;
        }

        // 개행 제거
        size_t length = strlen(inputLine);
        if (length > 0 && (inputLine[length - 1] == '\n' || inputLine[length - 1] == '\r')) {
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

        if (!IsLegalMove(&gameState, startRow, startColumn, endRow, endColumn)) {
            printf("합법적인 수가 아닙니다.\n\n");
            continue;
        }

        ApplyMove(&gameState, startRow, startColumn, endRow, endColumn);
        printf("\n");
    }

    return 0;
}
