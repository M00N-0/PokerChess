#pragma once
#define MovementPart

extern "C" {

    // ---- 최소 타입 정의 (필요한 상태만) ----
    // 보드: '.'=빈칸, 백=대문자(KQRBNP), 흑=소문자(kqrbnp)
    typedef struct {
        char board[8][8];
        char turn;     // 'w' or 'b' (이 모듈에선 사실 큰 의미 없음)
        // 캐슬 권리: 백 K(킹사이드)/Q(퀸사이드), 흑 k/q (1=가능, 0=불가)
        int K, Q, k, q;
        // 앙파상 타깃: 직전 수가 폰 2칸 전진이면, 중간칸 좌표 저장. (-1이면 없음)
        int ep_r, ep_c;
    } Game;

    // 이동 한 수
    typedef struct {
        int sr, sc;      // start (0..7)
        int er, ec;      // end   (0..7)
        char promo;      // 'q','r','b','n' (승격 없으면 0)
        int castle_short; // O-O  (킹사이드 캐슬)
        int castle_long;  // O-O-O(퀸사이드 캐슬)
        int en_passant;   // 앙파상 이동 여부
        char captured;    // 캡처된 말(시뮬레이션/디버그용, 없어도 됨)
    } Move;

    // ---- 이동/공격 관련 API (이 모듈만 제공) ----

    // (필수) 한 칸의 말을 기준으로 "슈도레갈" 이동을 out[]에 채움. 반환=개수.
    //  - 경로막힘/말 점프 규칙, 폰 전진/대각 캡처, 캐슬링 형식적 생성, 앙파상 생성 포함
    //  - "체크 여부"는 검증하지 않음(즉, 자살수 포함)
    int gen_piece_moves(const Game* g, int r, int c, Move* out);

    // (선택) 한 사이드 전체 슈도레갈 생성
    int gen_pseudo_legal(const Game* g, char side, Move* out);

    // (보조) 공격 판정: (r,c)가 attacker('w' 또는 'b')에게 공격받는가?
    int square_attacked_by(const Game* g, int r, int c, char attacker);

    // (보조) 체크 판정(이동 생성 외 참고용). 필요 없으면 안 써도 됨.
    int in_check(const Game* g, char side);

    // (보조) 시뮬레이션용 적용: 보드에 수를 “그냥 적용” (턴/권리 갱신 없음)
    //  - 합법성/체크는 검사하지 않음. 외부에서 필터링에 쓰라고 제공.
    void apply_move_no_turn(Game* g, const Move m);

    // (보조) 유틸: 킹 위치 찾기
    void king_pos(const Game* g, char side, int* kr, int* kc);

}
