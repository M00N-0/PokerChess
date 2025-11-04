// moves.c
#include "MovementPart.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// -------- 내부 유틸 --------
static int in_bounds(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
static int is_white(char p) { return (p >= 'A' && p <= 'Z'); }
static int is_black(char p) { return (p >= 'a' && p <= 'z'); }
static char side_of(char p) { return p == '.' ? 0 : (is_white(p) ? 'w' : 'b'); }

void king_pos(const Game* g, char side, int* kr, int* kc) {
    const char k = (side == 'w' ? 'K' : 'k');
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (g->board[r][c] == k) { *kr = r; *kc = c; return; }
    *kr = *kc = -1; // 못 찾은 경우
}

int square_attacked_by(const Game* g, int r, int c, char attacker) {
    // 폰
    int dir = (attacker == 'w' ? -1 : 1);
    for (int dc = -1; dc <= 1; dc += 2) {
        int rr = r - dir, cc = c + dc;
        if (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.' && side_of(p) == attacker && tolower((unsigned char)p) == 'p') return 1;
        }
    }
    // 나이트
    static const int K[8][2] = { {2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2} };
    for (int i = 0; i < 8; i++) {
        int rr = r + K[i][0], cc = c + K[i][1];
        if (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.' && side_of(p) == attacker && tolower((unsigned char)p) == 'n') return 1;
        }
    }
    // 킹
    for (int dr = -1; dr <= 1; dr++) for (int dc = -1; dc <= 1; dc++) {
        if (!dr && !dc) continue;
        int rr = r + dr, cc = c + dc;
        if (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.' && side_of(p) == attacker && tolower((unsigned char)p) == 'k') return 1;
        }
    }
    // 직선(룩/퀸)
    static const int R[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
    for (int i = 0; i < 4; i++) {
        int rr = r + R[i][0], cc = c + R[i][1];
        while (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.') {
                if (side_of(p) == attacker && (tolower((unsigned char)p) == 'r' || tolower((unsigned char)p) == 'q')) return 1;
                break;
            }
            rr += R[i][0]; cc += R[i][1];
        }
    }
    // 대각(비숍/퀸)
    static const int B[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };
    for (int i = 0; i < 4; i++) {
        int rr = r + B[i][0], cc = c + B[i][1];
        while (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.') {
                if (side_of(p) == attacker && (tolower((unsigned char)p) == 'b' || tolower((unsigned char)p) == 'q')) return 1;
                break;
            }
            rr += B[i][0]; cc += B[i][1];
        }
    }
    return 0;
}

int in_check(const Game* g, char side) {
    int kr, kc; king_pos(g, side, &kr, &kc);
    if (kr < 0) return 0;
    return square_attacked_by(g, kr, kc, (side == 'w' ? 'b' : 'w'));
}

// -------- 기본 push/ray --------
// 한칸 전진
static void push_move(Move* arr, int* n, int sr, int sc, int er, int ec,
    char promo, int cs, int cl, int ep, char cap) {
    Move m; m.sr = sr; m.sc = sc; m.er = er; m.ec = ec;
    m.promo = promo; m.castle_short = cs; m.castle_long = cl; m.en_passant = ep; m.captured = cap;
    arr[(*n)++] = m;
}

static void ray_moves(const Game* g, int r, int c, char side,
    const int (*d)[2], int nd, Move* out, int* n) {
    for (int i = 0; i < nd; i++) {
        int dr = d[i][0], dc = d[i][1];
        int rr = r + dr, cc = c + dc;
        while (in_bounds(rr, cc)) {
            char t = g->board[rr][cc];
            if (t == '.') {
                push_move(out, n, r, c, rr, cc, 0, 0, 0, 0, '.');
            }
            else {
                if (side_of(t) != side) push_move(out, n, r, c, rr, cc, 0, 0, 0, 0, t);
                break;
            }
            rr += dr; cc += dc;
        }
    }
}

// -------- 핵심: 한 칸 말의 슈도레갈 생성 --------
int gen_piece_moves(const Game* g, int r, int c, Move* out) {
    int n = 0;
    char p = g->board[r][c]; if (p == '.') return 0;
    char side = side_of(p);
    char low = (char)tolower((unsigned char)p);

    if (low == 'p') {
        int dir = (side == 'w' ? -1 : 1);
        int start_row = (side == 'w' ? 6 : 1);
        // 전진 1
        int nr = r + dir;
        if (in_bounds(nr, c) && g->board[nr][c] == '.') {
            int promo = (nr == 0 && side == 'w') || (nr == 7 && side == 'b');
            if (promo) {
                const char S[4] = { 'q','r','b','n' };
                for (int i = 0; i < 4; i++) push_move(out, &n, r, c, nr, c, S[i], 0, 0, 0, '.');
            }
            else push_move(out, &n, r, c, nr, c, 0, 0, 0, 0, '.');
            // 전진 2
            int nr2 = r + 2 * dir;
            if (r == start_row && in_bounds(nr2, c) && g->board[nr2][c] == '.')
                push_move(out, &n, r, c, nr2, c, 0, 0, 0, 0, '.');
        }
        // 대각 캡처
        for (int dc = -1; dc <= 1; dc += 2) {
            int nc = c + dc; nr = r + dir;
            if (in_bounds(nr, nc)) {
                char t = g->board[nr][nc];
                if (t != '.' && side_of(t) != side) {
                    int promo = (nr == 0 && side == 'w') || (nr == 7 && side == 'b');
                    if (promo) {
                        const char S[4] = { 'q','r','b','n' };
                        for (int i = 0; i < 4; i++) push_move(out, &n, r, c, nr, nc, S[i], 0, 0, 0, t);
                    }
                    else push_move(out, &n, r, c, nr, nc, 0, 0, 0, 0, t);
                }
            }
        }
        // 앙파상(형식적으로 생성)
        if (g->ep_r >= 0) {
            int tr = g->ep_r, tc = g->ep_c;
            if (tr == r + dir && abs(tc - c) == 1) {
                // 잡히는 폰은 (r, tc)에 있어야 함(상태 일관성 가정)
                char victim = g->board[r][tc];
                if (victim != '.' && tolower((unsigned char)victim) == 'p' && side_of(victim) != side)
                    push_move(out, &n, r, c, tr, tc, 0, 0, 0, 1, victim);
            }
        }

    }
    else if (low == 'n') {
        static const int K[8][2] = { {2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2} };
        for (int i = 0; i < 8; i++) {
            int nr = r + K[i][0], nc = c + K[i][1];
            if (!in_bounds(nr, nc)) continue;
            char t = g->board[nr][nc];
            if (t == '.' || side_of(t) != side)
                push_move(out, &n, r, c, nr, nc, 0, 0, 0, 0, (t == '.' ? '.' : t));
        }

    }
    else if (low == 'b') {
        static const int D[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };
        ray_moves(g, r, c, side, D, 4, out, &n);

    }
    else if (low == 'r') {
        static const int D[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
        ray_moves(g, r, c, side, D, 4, out, &n);

    }
    else if (low == 'q') {
        static const int D[8][2] = { {1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1} };
        ray_moves(g, r, c, side, D, 8, out, &n);

    }
    else if (low == 'k') {
        for (int dr = -1; dr <= 1; dr++) for (int dc = -1; dc <= 1; dc++) {
            if (!dr && !dc) continue;
            int nr = r + dr, nc = c + dc;
            if (!in_bounds(nr, nc)) continue;
            char t = g->board[nr][nc];
            if (t == '.' || side_of(t) != side)
                push_move(out, &n, r, c, nr, nc, 0, 0, 0, 0, (t == '.' ? '.' : t));
        }
        // 캐슬링(형식적 생성: 칸 비어있음/공격받지 않음 검사는 외부에서 할 수도 있음)
        int row = (side == 'w' ? 7 : 0);
        if (c == 4 && r == row) {
            // 킹사이드 O-O
            if ((side == 'w' ? g->K : g->k) && g->board[row][5] == '.' && g->board[row][6] == '.')
                if (tolower((unsigned char)g->board[row][7]) == 'r' && side_of(g->board[row][7]) == side)
                    push_move(out, &n, row, 4, row, 6, 0, 1, 0, 0, '.');
            // 퀸사이드 O-O-O
            if ((side == 'w' ? g->Q : g->q) && g->board[row][1] == '.' && g->board[row][2] == '.' && g->board[row][3] == '.')
                if (tolower((unsigned char)g->board[row][0]) == 'r' && side_of(g->board[row][0]) == side)
                    push_move(out, &n, row, 4, row, 2, 0, 0, 1, 0, '.');
        }
    }
    return n;
}

int gen_pseudo_legal(const Game* g, char side, Move* out) {
    int n = 0;
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (g->board[r][c] != '.' && side_of(g->board[r][c]) == side)
                n += gen_piece_moves(g, r, c, out + n);
    return n;
}

void apply_move_no_turn(Game* g, const Move m) {
    char p = g->board[m.sr][m.sc];
    // 캐슬/EP/승격을 포함해 보드에 “그대로” 반영 (권리/턴은 건드리지 않음)
    if (m.castle_short) {
        int row = (is_white(p) ? 7 : 0);
        g->board[row][6] = p; g->board[row][4] = '.';
        g->board[row][5] = g->board[row][7]; g->board[row][7] = '.';
        return;
    }
    if (m.castle_long) {
        int row = (is_white(p) ? 7 : 0);
        g->board[row][2] = p; g->board[row][4] = '.';
        g->board[row][3] = g->board[row][0]; g->board[row][0] = '.';
        return;
    }
    if (m.en_passant) {
        g->board[m.er][m.ec] = p;
        g->board[m.sr][m.sc] = '.';
        g->board[m.sr][m.ec] = '.'; // 잡힌 폰 제거
        return;
    }
    // 일반/승격
    if (m.promo) {
        g->board[m.er][m.ec] = is_white(p) ? (char)toupper((unsigned char)m.promo)
            : (char)tolower((unsigned char)m.promo);
    }
    else {
        g->board[m.er][m.ec] = p;
    }
    g->board[m.sr][m.sc] = '.';
}
