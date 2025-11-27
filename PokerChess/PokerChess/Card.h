#ifndef CARD_H
#define CARD_H

typedef enum {
    SPADE,
    HEART,
    DIAMOND,
    CLUB
} Suit;

typedef struct {
    Suit suit;   // ♠ ♥ ◆ ♣
    int value;   // 1(A), 2~10, 11(J), 12(Q), 13(K)
    char side;   // 'w' = 백, 'b' = 흑
} Card;

// 전방 선언 (GameState는 game.h에서 정의)
struct GameState;

// 랜덤 카드 생성 (side: 'w' 또는 'b')
Card* CreateRandomCard(char side);

// 고정 카드 생성 (K/Q 같은 것)
Card* CreateFixedCard(char side, int value);

// 카드 1개 이동 가능 여부 (게임 보드 상태까지 고려)
int CanCardMove(const struct GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn);

// 콘솔에 카드 한 칸 출력
void PrintCard(const Card* c);

#endif

