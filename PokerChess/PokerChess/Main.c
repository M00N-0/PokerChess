#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define BOARD_SIZE 8

// Windows/MSVC용 대체 함수
int strcasecmp(const char* s1, const char* s2) {
    while (*s1 && *s2) {
        char c1 = tolower((unsigned char)*s1++);
        char c2 = tolower((unsigned char)*s2++);
        if (c1 != c2) return (c1 - c2);
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}


typedef struct {
    int sr, sc, er, ec;   // start row/col, end row/col (0..7)
    char promo;           // 'q','r','b','n' (대/소문자 자동 처리), 0이면 없음
    int castle_short;     // O-O
    int castle_long;      // O-O-O
    int en_passant;       // 앙파상 이동인지
    char captured;        // 캡처된 말(시뮬레이션용 정보)
} Move;

typedef struct {
    char board[8][8];     // '.' 비어있음, 백 대문자(KQ R B N P), 흑 소문자
    char turn;            // 'w' or 'b'
    // 캐슬 권리
    int K, Q, k, q;       // 백 킹사이드/퀸사이드, 흑 킹사이드/퀸사이드
    int ep_r, ep_c;       // 앙파상 타겟 칸(-1이면 없음). 예: e2-e4 후 e3가 ep 타겟
} Game;

// ---------- 유틸 ----------
static const char* FILES = "abcdefgh";

int in_bounds(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }
int is_white(char p) { return p >= 'A' && p <= 'Z'; }
int is_black(char p) { return p >= 'a' && p <= 'z'; }
char side_of(char p) { if (p == '.') return 0; return is_white(p) ? 'w' : 'b'; }

void algebraic_to_rc(const char* sq, int* r, int* c) {
    *c = (int)(strchr(FILES, tolower(sq[0])) - FILES);
    *r = 8 - (sq[1] - '0');
}
void rc_to_algebraic(int r, int c, char out[3]) {
    out[0] = FILES[c];
    out[1] = '0' + (8 - r);
    out[2] = '\0';
}

void print_board(Game* g) {
    printf("  a b c d e f g h\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", 8 - i);
        for (int j = 0; j < 8; j++) {
            printf("%c ", g->board[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n\n");
}

void init_start(Game* g) {
    const char* start[8] = {
        "rnbqkbnr",
        "pppppppp",
        "........",
        "........",
        "........",
        "........",
        "PPPPPPPP",
        "RNBQKBNR"
    };
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            g->board[r][c] = start[r][c];
    g->turn = 'w';
    g->K = g->Q = g->k = g->q = 1;
    g->ep_r = g->ep_c = -1;
}

// ---------- 체크/공격 판정 ----------
void king_pos(Game* g, char side, int* kr, int* kc) {
    char k = (side == 'w' ? 'K' : 'k');
    for (int r = 0; r < 8; r++)
        for (int c = 0; c < 8; c++)
            if (g->board[r][c] == k) { *kr = r; *kc = c; return; }
    // 안전상 기본값
    *kr = -1; *kc = -1;
}

int square_attacked_by(Game* g, int r, int c, char attacker) {
    // 1) 폰
    int dir = (attacker == 'w' ? -1 : 1);
    for (int dc = -1; dc <= 1; dc += 2) {
        int rr = r - dir, cc = c + dc;
        if (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.' && side_of(p) == attacker && tolower(p) == 'p') return 1;
        }
    }
    // 2) 나이트
    int K[8][2] = { {2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2} };
    for (int i = 0; i < 8; i++) {
        int rr = r + K[i][0], cc = c + K[i][1];
        if (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.' && side_of(p) == attacker && tolower(p) == 'n') return 1;
        }
    }
    // 3) 킹
    for (int dr = -1; dr <= 1; dr++) for (int dc = -1; dc <= 1; dc++) {
        if (!dr && !dc) continue;
        int rr = r + dr, cc = c + dc;
        if (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.' && side_of(p) == attacker && tolower(p) == 'k') return 1;
        }
    }
    // 4) 직선(룩/퀸)
    int R[4][2] = { {1,0},{-1,0},{0,1},{0,-1} };
    for (int i = 0; i < 4; i++) {
        int rr = r + R[i][0], cc = c + R[i][1];
        while (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.') {
                if (side_of(p) == attacker && (tolower(p) == 'r' || tolower(p) == 'q')) return 1;
                break;
            }
            rr += R[i][0]; cc += R[i][1];
        }
    }
    // 5) 대각(비숍/퀸)
    int B[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };
    for (int i = 0; i < 4; i++) {
        int rr = r + B[i][0], cc = c + B[i][1];
        while (in_bounds(rr, cc)) {
            char p = g->board[rr][cc];
            if (p != '.') {
                if (side_of(p) == attacker && (tolower(p) == 'b' || tolower(p) == 'q')) return 1;
                break;
            }
            rr += B[i][0]; cc += B[i][1];
        }
    }
    return 0;
}

int in_check(Game* g, char side) {
    int kr, kc; king_pos(g, side, &kr, &kc);
    if (kr < 0) return 0; // 방어
    return square_attacked_by(g, kr, kc, (side == 'w' ? 'b' : 'w'));
}

// ---------- 이동 생성 ----------
void push_move(Move* arr, int* n, int sr, int sc, int er, int ec, char promo, int cs, int cl, int ep, char cap) {
    Move m; m.sr = sr; m.sc = sc; m.er = er; m.ec = ec; m.promo = promo; m.castle_short = cs; m.castle_long = cl; m.en_passant = ep; m.captured = cap;
    arr[(*n)++] = m;
}

void ray_moves(Game* g, int r, int c, char side, int deltas[][2], int nd, Move* out, int* n) {
    for (int i = 0; i < nd; i++) {
        int dr = deltas[i][0], dc = deltas[i][1];
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

void gen_piece_moves(Game* g, int r, int c, Move* out, int* n) {
    char p = g->board[r][c]; if (p == '.') return;
    char side = side_of(p); char low = tolower(p);
    if (low == 'p') {
        int dir = (side == 'w' ? -1 : 1);
        int start_row = (side == 'w' ? 6 : 1);
        // 전진1
        int nr = r + dir;
        if (in_bounds(nr, c) && g->board[nr][c] == '.') {
            int promo = (nr == 0 && side == 'w') || (nr == 7 && side == 'b');
            if (promo) {
                char sym[4] = { 'q','r','b','n' };
                for (int i = 0; i < 4; i++) push_move(out, n, r, c, nr, c, sym[i], 0, 0, 0, '.');
            }
            else push_move(out, n, r, c, nr, c, 0, 0, 0, 0, '.');
            // 전진2
            int nr2 = r + 2 * dir;
            if (r == start_row && g->board[nr2][c] == '.') push_move(out, n, r, c, nr2, c, 0, 0, 0, 0, '.');
        }
        // 대각 캡처
        for (int dc = -1; dc <= 1; dc += 2) {
            int nc = c + dc; nr = r + dir;
            if (in_bounds(nr, nc)) {
                char t = g->board[nr][nc];
                if (t != '.' && side_of(t) != side) {
                    int promo = (nr == 0 && side == 'w') || (nr == 7 && side == 'b');
                    if (promo) {
                        char sym[4] = { 'q','r','b','n' };
                        for (int i = 0; i < 4; i++) push_move(out, n, r, c, nr, nc, sym[i], 0, 0, 0, t);
                    }
                    else push_move(out, n, r, c, nr, nc, 0, 0, 0, 0, t);
                }
            }
        }
        // 앙파상
        if (g->ep_r >= 0) {
            int tr = g->ep_r, tc = g->ep_c;
            if (tr == r + dir && abs(tc - c) == 1) {
                // 잡히는 말은 (r, tc)
                char victim = g->board[r][tc];
                if (victim != '.' && tolower(victim) == 'p' && side_of(victim) != side)
                    push_move(out, n, r, c, tr, tc, 0, 0, 0, 1, victim);
            }
        }
    }
    else if (low == 'n') {
        int K[8][2] = { {2,1},{2,-1},{-2,1},{-2,-1},{1,2},{1,-2},{-1,2},{-1,-2} };
        for (int i = 0; i < 8; i++) {
            int nr = r + K[i][0], nc = c + K[i][1];
            if (!in_bounds(nr, nc)) continue;
            char t = g->board[nr][nc];
            if (t == '.' || side_of(t) != side) push_move(out, n, r, c, nr, nc, 0, 0, 0, 0, (t == '.' ? '.' : t));
        }
    }
    else if (low == 'b') {
        int D[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} }; ray_moves(g, r, c, side, D, 4, out, n);
    }
    else if (low == 'r') {
        int D[4][2] = { {1,0},{-1,0},{0,1},{0,-1} }; ray_moves(g, r, c, side, D, 4, out, n);
    }
    else if (low == 'q') {
        int D[8][2] = { {1,0},{-1,0},{0,1},{0,-1},{1,1},{1,-1},{-1,1},{-1,-1} }; ray_moves(g, r, c, side, D, 8, out, n);
    }
    else if (low == 'k') {
        for (int dr = -1; dr <= 1; dr++) for (int dc = -1; dc <= 1; dc++) {
            if (!dr && !dc) continue;
            int nr = r + dr, nc = c + dc;
            if (!in_bounds(nr, nc)) continue;
            char t = g->board[nr][nc];
            if (t == '.' || side_of(t) != side) push_move(out, n, r, c, nr, nc, 0, 0, 0, 0, (t == '.' ? '.' : t));
        }
        // 캐슬링
        // 킹은 e열(4)에 있어야 함(간단화)
        if (c == 4) {
            int row = (side == 'w' ? 7 : 0);
            char enemy = (side == 'w' ? 'b' : 'w');
            if (r == row && !in_check(g, side)) {
                // 킹사이드
                int canK = (side == 'w' ? g->K : g->k);
                if (canK && g->board[row][5] == '.' && g->board[row][6] == '.') {
                    if (!square_attacked_by(g, row, 5, enemy) && !square_attacked_by(g, row, 6, enemy)) {
                        // 룩 확인
                        if (tolower(g->board[row][7]) == 'r' && side_of(g->board[row][7]) == side)
                            push_move(out, n, row, 4, row, 6, 0, 1, 0, 0, '.');
                    }
                }
                // 퀸사이드
                int canQ = (side == 'w' ? g->Q : g->q);
                if (canQ && g->board[row][1] == '.' && g->board[row][2] == '.' && g->board[row][3] == '.') {
                    if (!square_attacked_by(g, row, 3, enemy) && !square_attacked_by(g, row, 2, enemy)) {
                        if (tolower(g->board[row][0]) == 'r' && side_of(g->board[row][0]) == side)
                            push_move(out, n, row, 4, row, 2, 0, 0, 1, 0, '.');
                    }
                }
            }
        }
    }
}

int gen_pseudo_legal(Game* g, char side, Move* out) {
    int n = 0;
    for (int r = 0; r < 8; r++) for (int c = 0; c < 8; c++) {
        char p = g->board[r][c];
        if (p != '.' && side_of(p) == side) gen_piece_moves(g, r, c, out, &n);
    }
    return n;
}

// ---------- make/undo 없이 시뮬레이션 복제 ----------
void clone_game(Game* dst, Game* src) {
    memcpy(dst, src, sizeof(Game));
}

char promoted_as(char orig, char promo) { // orig 색상 보존
    if (is_white(orig)) return toupper(promo);
    else return tolower(promo);
}

void apply_move_no_turn(Game* g, Move m) {
    char p = g->board[m.sr][m.sc];
    char side = side_of(p);
    // 캐슬
    if (m.castle_short) {
        int row = (side == 'w' ? 7 : 0);
        g->board[row][6] = p; g->board[row][4] = '.';
        g->board[row][5] = g->board[row][7]; g->board[row][7] = '.';
        return;
    }
    if (m.castle_long) {
        int row = (side == 'w' ? 7 : 0);
        g->board[row][2] = p; g->board[row][4] = '.';
        g->board[row][3] = g->board[row][0]; g->board[row][0] = '.';
        return;
    }
    if (m.en_passant) {
        g->board[m.er][m.ec] = p;
        g->board[m.sr][m.sc] = '.';
        // 잡히는 폰은 (m.sr, m.ec)
        g->board[m.sr][m.ec] = '.';
        return;
    }
    // 일반/승격
    g->board[m.er][m.ec] = (m.promo ? promoted_as(p, m.promo) : p);
    g->board[m.sr][m.sc] = '.';
}

void update_rights_on_capture(Game* g, Move m, char side) {
    // 잡힌 룩이면 상대 캐슬 권리 박탈(원위치 기준)
    if (m.captured && tolower(m.captured) == 'r') {
        if (side == 'w') { // 백이 잡음 -> 흑 권리 깎기
            if (m.er == 0 && m.ec == 7) g->k = 0;
            if (m.er == 0 && m.ec == 0) g->q = 0;
        }
        else {
            if (m.er == 7 && m.ec == 7) g->K = 0;
            if (m.er == 7 && m.ec == 0) g->Q = 0;
        }
    }
}

void make_move(Game* g, Move m) {
    char p = g->board[m.sr][m.sc];
    char side = side_of(p);

    // EP 기본 해제
    g->ep_r = g->ep_c = -1;

    if (m.castle_short) {
        int row = (side == 'w' ? 7 : 0);
        g->board[row][6] = p; g->board[row][4] = '.';
        g->board[row][5] = g->board[row][7]; g->board[row][7] = '.';
        if (side == 'w') { g->K = 0; g->Q = 0; }
        else { g->k = 0; g->q = 0; }
    }
    else if (m.castle_long) {
        int row = (side == 'w' ? 7 : 0);
        g->board[row][2] = p; g->board[row][4] = '.';
        g->board[row][3] = g->board[row][0]; g->board[row][0] = '.';
        if (side == 'w') { g->K = 0; g->Q = 0; }
        else { g->k = 0; g->q = 0; }
    }
    else if (m.en_passant) {
        g->board[m.er][m.ec] = p;
        g->board[m.sr][m.sc] = '.';
        g->board[m.sr][m.ec] = '.'; // 잡힌 폰 제거
    }
    else {
        // 룩/킹 이동 시 권리 소멸
        if (tolower(p) == 'k') {
            if (side == 'w') { g->K = 0; g->Q = 0; }
            else { g->k = 0; g->q = 0; }
        }
        if (tolower(p) == 'r') {
            if (side == 'w') {
                if (m.sr == 7 && m.sc == 7) g->K = 0;
                if (m.sr == 7 && m.sc == 0) g->Q = 0;
            }
            else {
                if (m.sr == 0 && m.sc == 7) g->k = 0;
                if (m.sr == 0 && m.sc == 0) g->q = 0;
            }
        }
        // 캡처로 룩 권리 소멸
        update_rights_on_capture(g, m, side);

        // 이동/승격
        g->board[m.er][m.ec] = (m.promo ? promoted_as(p, m.promo) : p);
        g->board[m.sr][m.sc] = '.';

        // 폰 두 칸 전진이면 EP 타겟 설정
        if (tolower(p) == 'p' && abs(m.er - m.sr) == 2) {
            g->ep_r = (m.er + m.sr) / 2;
            g->ep_c = m.sc;
        }
    }

    // 턴 넘기기
    g->turn = (g->turn == 'w' ? 'b' : 'w');
}

// 완전 합법수 필터링
int legal_moves(Game* g, Move* out) {
    Move buf[256]; int n = gen_pseudo_legal(g, g->turn, buf);
    int k = 0;
    for (int i = 0; i < n; i++) {
        Game tmp; clone_game(&tmp, g);
        apply_move_no_turn(&tmp, buf[i]);
        // 캐슬 권리/EP 등의 부가조건은 체크 여부에 영향 없음 -> in_check로만 필터
        if (!in_check(&tmp, g->turn)) out[k++] = buf[i];
    }
    return k;
}

// ---------- 파서 ----------
int parse_move(Game* g, const char* s, Move* req) {
    // 캐슬
    if (!strcasecmp(s, "o-o") || !strcasecmp(s, "0-0")) {
        int r, kc; king_pos(g, g->turn, &r, &kc);
        req->sr = r; req->sc = kc; req->er = r; req->ec = 6;
        req->promo = 0; req->castle_short = 1; req->castle_long = 0; req->en_passant = 0; req->captured = '.';
        return 1;
    }
    if (!strcasecmp(s, "o-o-o") || !strcasecmp(s, "0-0-0")) {
        int r, kc; king_pos(g, g->turn, &r, &kc);
        req->sr = r; req->sc = kc; req->er = r; req->ec = 2;
        req->promo = 0; req->castle_short = 0; req->castle_long = 1; req->en_passant = 0; req->captured = '.';
        return 1;
    }
    // "e2 e4" 또는 "e7 e8=Q"
    char a[8] = { 0 }, b[8] = { 0 }, promo = 0;
    if (sscanf(s, "%2s %7s", a, b) != 2) return 0;
    char* eq = strchr(b, '=');
    if (eq) {
        promo = tolower(*(eq + 1));
        *eq = '\0';
        if (!(promo == 'q' || promo == 'r' || promo == 'b' || promo == 'n')) return 0;
    }
    int sr, sc, er, ec;
    if (strlen(a) != 2 || strlen(b) != 2) return 0;
    algebraic_to_rc(a, &sr, &sc);
    algebraic_to_rc(b, &er, &ec);
    req->sr = sr; req->sc = sc; req->er = er; req->ec = ec;
    req->promo = promo; req->castle_short = 0; req->castle_long = 0; req->en_passant = 0; req->captured = '.';
    return 1;
}

int moves_equal(Move* a, Move* b) {
    if (a->castle_short && b->castle_short) return 1;
    if (a->castle_long && b->castle_long) return 1;
    if (a->sr == b->sr && a->sc == b->sc && a->er == b->er && a->ec == b->ec) {
        if (a->promo && b->promo && tolower(a->promo) != tolower(b->promo)) return 0;
        return 1;
    }
    return 0;
}

// ---------- 종료 판정 ----------
int result_if_over(Game* g, char* msg, size_t nmsg) {
    Move leg[256]; int n = legal_moves(g, leg);
    if (n > 0) return 0;
    if (in_check(g, g->turn)) {
        snprintf(msg, nmsg, "체크메이트: %s 패배", (g->turn == 'w' ? "백" : "흑"));
    }
    else {
        snprintf(msg, nmsg, "스테일메이트(무승부)");
    }
    return 1;
}

// ---------- 메인 ----------
int main(void) {
    Game g; init_start(&g);
    char line[128];

    puts("=== 텍스트 체스 (C, 완전 규칙) ===");
    puts("백: 대문자 / 흑: 소문자");
    puts("입력 예) e2 e4  /  g7 g8=Q  /  O-O  /  O-O-O  /  quit\n");

    for (;;) {
        print_board(&g);
        if (in_check(&g, g.turn)) {
            printf("%s가 체크!\n", (g.turn == 'w' ? "백" : "흑"));
        }
        char endmsg[64];
        if (result_if_over(&g, endmsg, sizeof(endmsg))) {
            puts(endmsg);
            break;
        }

        printf("%s 수: ", (g.turn == 'w' ? "백" : "흑"));
        if (!fgets(line, sizeof(line), stdin)) break;
        printf("\n");
        // 공백/개행 정리
        for (size_t i = 0; i < strlen(line); ++i) if (line[i] == '\n' || line[i] == '\r') line[i] = 0;
        if (!strcmp(line, "quit")) { puts("게임 종료."); break; }
        if (!line[0]) continue;

        Move req;
        if (!parse_move(&g, line, &req)) { puts("입력 형식이 올바르지 않습니다."); continue; }

        // 출발칸/차례 확인
        if (!in_bounds(req.sr, req.sc) || !in_bounds(req.er, req.ec)) { puts("보드 범위를 벗어났습니다."); continue; }
        char p = g.board[req.sr][req.sc];
        if (p == '.') { puts("출발 칸에 말이 없습니다."); continue; }
        if (side_of(p) != g.turn) { puts("현재 차례의 말이 아닙니다."); continue; }

        // 합법수 목록에서 매칭
        Move leg[256]; int n = legal_moves(&g, leg);
        int found = -1;
        for (int i = 0; i < n; i++) {
            // 승격 생략 시 자동 퀸
            Move cand = leg[i];
            if (cand.promo && !req.promo) cand.promo = 'q';
            if (moves_equal(&cand, &req)) { found = i; break; }
        }
        if (found < 0) { puts("합법적이지 않은 수입니다."); continue; }

        // 실제 적용: 권리/EP 등 갱신 포함
        // 선택된 합법수(원본) 다시 찾아서 make_move
        Move chosen = leg[found];
        // make_move는 캐슬/EP/권리 업데이트 처리
        make_move(&g, chosen);
    }
    return 0;
}
