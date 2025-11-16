#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>

#define BOARD_SIZE 8

char board[BOARD_SIZE][BOARD_SIZE];

// 콘솔 핸들
HANDLE hConsole;

// 배경색 설정 함수
void SetColor(int text, int background)
{
    SetConsoleTextAttribute(hConsole, text + (background << 4));
}

void InitBoard()
{
    for (int r = 0; r < BOARD_SIZE; r++)
        for (int c = 0; c < BOARD_SIZE; c++)
            board[r][c] = '.';
}

void PrintBoard()
{
    printf("\n   a b c d e f g h\n");

    for (int row = 7; row >= 0; row--)
    {
        printf("%d ", row + 1);

        for (int col = 0; col < 8; col++)
        {
            int isWhite = (row + col) % 2 == 0;

            if (isWhite)
                SetColor(0, 15);   // 글자 검정, 배경 흰색
            else
                SetColor(15, 0);   // 글자 흰색, 배경 검정

            printf(" %c", board[row][col]);

            SetColor(15, 0); // 기본색 리셋
        }

        printf("  %d\n", row + 1);
    }

    printf("   a b c d e f g h\n");
}

void SetPiece(const char* coord, char piece)
{
    int col = coord[0] - 'a';
    int row = coord[1] - '1';
    board[row][col] = piece;
}

int main()
{
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    InitBoard();
    //초기 말 배치
    /*SetPiece("a1", 'A');
    SetPiece("b1", 'B');
    SetPiece("c1", 'C');
    SetPiece("d1", 'D');
    SetPiece("e1", 'E');*/

    PrintBoard();

    return 0;
}
