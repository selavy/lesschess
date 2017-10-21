#ifndef EVALUATE__H_
#define EVALUATE__H_

#define INFINITI 1000000
#define NEG_INFINITI -1000000
#define WHITE_WIN INFINITI
#define BLACK_WIN NEG_INFINITI

struct position;
int evaluate(struct position *restrict const pos);

#endif // EVALUATE__H_
