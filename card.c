#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef enum { SPADE, HEART, DIAMOND, CLUB } Suit;

typedef struct 
{
    Suit suit;
    int value;  // A=1, J=11, Q=12, K=13
} Card;

typedef struct 
{
    Card* board[8][8];
} Board;

// 카드 출력용 함수
const char* suitSymbol(Suit s)
{
    switch (s) 
    {
    case SPADE: return "♠";
    case HEART: return "♥";
    case DIAMOND: return "♦";
    case CLUB: return "♣";
    }
    return "?";
}

const char* valueSymbol(int v) 
{
    static char buf[3];
    if (v == 1) return "A";
    if (v == 11) return "J";
    if (v == 12) return "Q";
    if (v == 13) return "K";
    snprintf(buf, 3, "%d", v);
    return buf;
}

void printCard(Card* c)
{
    if (!c)
    { 
        printf(" . "); return; 
    }
    printf("%s%s", suitSymbol(c->suit), valueSymbol(c->value));
}

// 이동 규칙 판단 함수
int canMove(Card* c, int r1, int c1, int r2, int c2)
{
    int dr = abs(r2 - r1);
    int dc = abs(c2 - c1);

    // ---- 숫자 규칙 ----

    // A: 5x5 범위 이동
    if (c->value == 1) 
    {
        return (dr <= 2 && dc <= 2);
    }

    // J: 나이트 + 상하좌우 3칸
    if (c->value == 11)
    {
        if ((dr == 2 && dc == 1) || (dr == 1 && dc == 2)) return 1;
        if ((dr == 3 && dc == 0) || (dr == 0 && dc == 3)) return 1;
        return 0;
    }

    // Q: 체스 퀸
    if (c->value == 12)
    {
        return (dr == dc || dr == 0 || dc == 0);
    }

    // K: 체스 킹 (+ 문양 하트일 경우 2칸까지)
    if (c->value == 13) 
    {
        int limit = (c->suit == HEART ? 2 : 1);
        return (dr <= limit && dc <= limit);
    }

    // 2~10: 폰처럼 1칸 직진 (편의상 전진 방향 없음 → 어디든 1칸 움직이게 설정)
    if (c->value >= 2 && c->value <= 10) 
    {
        // 다이아몬드면 항상 2칸 이동
        if (c->suit == DIAMOND)
            return (dr + dc == 2); // 2칸 이동

        // 일반 1칸 이동
        return (dr + dc == 1);
    }

    return 0;
}


// 이동 시도 함수
int moveCard(Board* b, int r1, int c1, int r2, int c2)
{
    Card* c = b->board[r1][c1];
    if (!c) return 0;

    if (!canMove(c, r1, c1, r2, c2))
    {
        printf("이동 불가!\n");
        return 0;
    }

    b->board[r2][c2] = c;
    b->board[r1][c1] = NULL;
    return 1;
}

// 보드 출력
void printBoard(Board* b) 
{
    printf("\n");
    for (int r = 0; r < 8; r++) 
    {
        for (int c = 0; c < 8; c++) 
        {
            printCard(b->board[r][c]);
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}

// 메인
int main() 
{
    Board b = { 0 };

    Card a = { SPADE, 1 };      // A
    Card j = { HEART, 11 };     // ♥J
    Card k = { HEART, 13 };     // ♥K (2칸까지)
    Card d5 = { DIAMOND, 5 };   // ♦5 (무조건 2칸)

    b.board[0][0] = &a;
    b.board[1][1] = &j;
    b.board[2][2] = &k;
    b.board[4][4] = &d5;

    printBoard(&b);

    printf("A(0,0)->(2,2)\n");
    moveCard(&b, 0, 0, 2, 2);

    printf("♥K(2,2)->(4,2)\n");
    moveCard(&b, 2, 2, 4, 2);

    printf("♦5(4,4)->(4,6)\n");
    moveCard(&b, 4, 4, 4, 6);

    printBoard(&b);

    return 0;
}
