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
        printf(" . "); 
        return; 
    }
    printf("%s%s", suitSymbol(c->suit), valueSymbol(c->value));
}

int canMove(Card* c, int r1, int c1, int r2, int c2)
{
    int dr = abs(r2 - r1);
    int dc = abs(c2 - c1);

    // A : 5x5
    if (c->value == 1) 
    {
        return (dr <= 2 && dc <= 2);
    }

    // J : knight + 3칸 직선
    if (c->value == 11)
    {
        if ((dr == 2 && dc == 1) || (dr == 1 && dc == 2)) return 1;
        if ((dr == 3 && dc == 0) || (dr == 0 && dc == 3)) return 1;
        return 0;
    }

    // Q : queen
    if (c->value == 12)
    {
        return (dr == dc || dr == 0 || dc == 0);
    }

    // K : king (하트는 2칸까지)
    if (c->value == 13) 
    {
        int limit = (c->suit == HEART ? 2 : 1);
        return (dr <= limit && dc <= limit);
    }

    // 2~10 : 폰처럼 1칸 / 다이아는 항상 2칸
    if (c->value >= 2 && c->value <= 10) 
    {
        if (c->suit == DIAMOND)
            return (dr + dc == 2);
        return (dr + dc == 1);
    }

    return 0;
}

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


// =========================
//        메인 (여기만 수정)
// =========================

int main() 
{
    Board b = { 0 };

    // --- 문양 효과용 변수 ---
    int turns = 1;              // 기본 1턴
    int spadeBonusUsed = 0;     // 스페이드 효과 1회만 발동

    // 카드 생성
    Card a = { SPADE, 1 };      // ♠A
    Card j = { HEART, 11 };     // ♥J
    Card k = { HEART, 13 };     // ♥K
    Card d5 = { DIAMOND, 5 };   // ♦5
    Card c3 = { CLUB, 3 };      // ♣3 (클로버 효과)

    // *** 클로버 효과 : 게임 시작 시 2턴 ***
    if (c3.suit == CLUB)
    {
        turns = 2;
        printf("클로버 효과 발동! 시작 턴이 2턴으로 증가.\n");
    }

    // 보드 배치
    b.board[0][0] = &a;
    b.board[1][1] = &j;
    b.board[2][2] = &k;
    b.board[4][4] = &d5;
    b.board[6][6] = &c3;

    printBoard(&b);

    printf("현재 턴: %d\n\n", turns);

    // ---- 턴 1 ----
    printf("A(0,0)->(2,2)\n");
    if (moveCard(&b, 0, 0, 2, 2))
    {
        // *** 스페이드 효과 : 이동 성공 시 추가 턴 (딱 한 번) ***
        if (!spadeBonusUsed && a.suit == SPADE)
        {
            spadeBonusUsed = 1;
            turns++;
            printf("스페이드 효과 발동! +1 턴 추가.\n");
        }
    }
    printf("현재 턴: %d\n\n", turns);

    // ---- 턴 2 ----
    printf("♥K(2,2)->(4,2)\n");
    moveCard(&b, 2, 2, 4, 2);
    turns--;
    printf("현재 턴: %d\n\n", turns);

    // ---- 턴 3 ----
    printf("♦5(4,4)->(4,6)\n");
    moveCard(&b, 4, 4, 4, 6);
    turns--;
    printf("현재 턴: %d\n\n", turns);

    printBoard(&b);

    return 0;
}
