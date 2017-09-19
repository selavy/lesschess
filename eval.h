#ifndef EVAL__H_
#define EVAL__H_

#define INFINITI 1000000
#define NEG_INFINITI -1000000
#define WHITE_WIN INFINITI
#define BLACK_WIN NEG_INFINITI

struct position;
int eval(struct position *restrict const pos);

#endif // EVAL__H_
