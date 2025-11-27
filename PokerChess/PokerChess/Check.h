#ifndef CHECK_H
#define CHECK_H

#include "game.h"

// 이동한 쪽(mySide) 왕이 여전히 체크 상태인지 확인
int IsInCheck(const GameState* gameState, char side);

// 함수: 자살수 방지 (이동 후 자신 왕이 체크 상태 되는지 확인)
int IsMovePuttingSelfInCheck(const GameState* gameState,
    int startRow, int startColumn,
    int endRow, int endColumn);

// 함수: 체크메이트 판정 (현재 턴 플레이어 패배 여부)
int IsCheckmate(const GameState* gameState);

#endif
#pragma once
