/* This file is automatically generated. */
/* Editing is useless.                   */

/********************************************/
/* Table.  min_m:  -5 max_m:   5 max_N:   4 */

state_for_miss_m_N _table_1_0_miss[] =
{
  /*   m  N [num] */  /* parity = 0 */

  /*  -5          */
  /*      2 [  1] */  19,
  /*  -3          */
  /*      2 [  2] */  13,  18,
  /*  -1          */
  /*      0 [  1] */   1,
  /*      2 [  3] */   9,  12,  17,
  /*   1          */
  /*      0 [  1] */   0,
  /*      2 [  3] */   8,  11,  16,
  /*   3          */
  /*      2 [  2] */  10,  15,
  /*   5          */
  /*      2 [  1] */  14,

  /*   m  N [num] */  /* parity = 1 */

  /*  -5          */
  /*  -3          */
  /*      1 [  1] */   7,
  /*  -1          */
  /*      1 [  2] */   3,   6,
  /*      3 [  4] */  21,  24,  28,  32,
  /*   1          */
  /*      1 [  2] */   2,   5,
  /*      3 [  4] */  20,  23,  27,  31,
  /*   3          */
  /*      1 [  1] */   4,
  /*      3 [  3] */  22,  26,  30,
  /*   5          */
  /*      3 [  2] */  25,  29,
};


index_into_state_for_miss _table_1_0_offset[] =
{
  /* parity = 0 */

  /*   m \ N   0     2     4  */

  /*  -5 */    0,    0,    1, /*    1 */
  /*  -3 */    1,    1,    3, /*    3 */
  /*  -1 */    3,    4,    7, /*    7 */
  /*   1 */    7,    8,   11, /*   11 */
  /*   3 */   11,   11,   13, /*   13 */
  /*   5 */   13,   13,   14, /*   14 */

  /* parity = 1 */

  /*   m \ N   0     1     3  */

  /*  -5 */   14,   14,   14, /*   14 */
  /*  -3 */   14,   14,   15, /*   15 */
  /*  -1 */   15,   15,   17, /*   21 */
  /*   1 */   21,   21,   23, /*   27 */
  /*   3 */   27,   27,   28, /*   31 */
  /*   5 */   31,   31,   31, /*   33 */

  /*     */   33
};

info_state_for_miss _table_1_0_info =
{
  -5, 11, 3, 5, 18,
  _table_1_0_miss,
  _table_1_0_offset,
};

/********************************************/

/********************************************/
/* Table.  min_m: -10 max_m:  10 max_N:   6 */

state_for_miss_m_N _table_2_0_miss[] =
{
  /*   m  N [num] */  /* parity = 0 */

  /* -10          */
  /*      4 [  1] */  19,
  /*  -8          */
  /*      4 [  3] */  13,  18,  19,
  /*  -6          */
  /*      2 [  3] */   1,   7,
  /*      4 [  6] */   9,  12,  13,  17,  18,  19,
  /* cnt  2 [  1] */  19,
  /*  -4          */
  /*      2 [  8] */   0,   1,   3,   6,   7,
  /*      4 [ 13] */   8,   9,  11,  12,  13,  16,  17,  18,  19,  21,  24,
                      28,  32,
  /* cnt  2 [  3] */  13,  18,  19,
  /*  -2          */
  /*      0 [  1] */
  /*      2 [ 12] */   0,   1,   2,   3,   5,   6,   7,
  /*      4 [ 19] */   8,   9,  10,  11,  12,  13,  15,  16,  17,  18,  19,
                      20,  21,  23,  24,  27,  28,  31,  32,
  /* cnt  0 [  1] */   1,
  /* cnt  2 [  5] */   9,  12,  13,  17,  18,
  /*   0          */
  /*      0 [  2] */   0,   1,
  /*      2 [ 12] */   2,   3,   4,   5,   6,   7,   8,   9,
  /*      4 [ 21] */  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,
                      21,  22,  23,  24,  26,  27,  28,  30,  31,  32,
  /* cnt  2 [  4] */  11,  12,  16,  17,
  /*   2          */
  /*      0 [  1] */   0,
  /*      2 [ 11] */   1,   2,   3,   4,   5,   6,
  /*      4 [ 25] */   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,
                      18,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
                      30,  31,  32,
  /* cnt  2 [  5] */   8,  10,  11,  15,  16,
  /*   4          */
  /*      2 [  8] */   0,   1,   2,
  /*      4 [ 22] */   3,   4,   5,   6,   8,   9,  10,  11,  12,  14,  15,
                      16,  17,  20,
  /*      6 [ 12] */  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
                      32,
  /* cnt  2 [  5] */   4,   5,  10,  14,  15,
  /* cnt  4 [  8] */  22,  23,  25,  26,  27,  29,  30,  31,
  /*   6          */
  /*      2 [  3] */   0,
  /*      4 [ 14] */   2,   4,   5,   8,  10,  11,  14,  15,  16,
  /*      6 [  9] */  20,  22,  23,  25,  26,  27,  29,  30,  31,
  /* cnt  2 [  2] */   4,  14,
  /* cnt  4 [  5] */  22,  25,  26,  29,  30,
  /*   8          */
  /*      4 [  6] */   4,  10,  14,  15,
  /*      6 [  5] */  22,  25,  26,  29,  30,
  /* cnt  4 [  2] */  25,  29,
  /*  10          */
  /*      4 [  1] */  14,
  /*      6 [  2] */  25,  29,

  /*   m  N [num] */  /* parity = 1 */

  /* -10          */
  /*  -8          */
  /*      3 [  2] */   7,  19,
  /*  -6          */
  /*      3 [  6] */   3,   6,   7,  13,  18,  19,
  /*      5 [  4] */  21,  24,  28,  32,
  /*  -4          */
  /*      1 [  2] */   1,
  /*      3 [ 11] */   2,   3,   5,   6,   7,   9,  12,  13,  17,  18,  19,
  /*      5 [  8] */  20,  21,  23,  24,  27,  28,  31,  32,
  /* cnt  1 [  1] */   7,
  /*  -2          */
  /*      1 [  5] */   0,   1,
  /*      3 [ 19] */   2,   3,   4,   5,   6,   7,   8,   9,  11,  12,  13,
                      16,  17,  18,  19,
  /*      5 [ 11] */  20,  21,  22,  23,  24,  26,  27,  28,  30,  31,  32,
  /* cnt  1 [  3] */   3,   6,   7,
  /* cnt  3 [  4] */  21,  24,  28,  32,
  /*   0          */
  /*      1 [  6] */   0,   1,   2,   3,
  /*      3 [ 22] */   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  15,
                      16,  17,  18,
  /*      5 [ 14] */  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
                      30,  31,  32,
  /* cnt  1 [  2] */   5,   6,
  /* cnt  3 [  8] */  20,  21,  23,  24,  27,  28,  31,  32,
  /*   2          */
  /*      1 [  5] */   0,   1,   2,
  /*      3 [ 21] */   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
  /*      5 [ 19] */  13,  14,  15,  16,  17,  18,  20,  21,  22,  23,  24,
                      25,  26,  27,  28,  29,  30,  31,  32,
  /* cnt  1 [  2] */   4,   5,
  /* cnt  3 [ 11] */  14,  15,  16,  17,  20,  22,  23,  26,  27,  30,  31,
  /*   4          */
  /*      1 [  2] */   0,
  /*      3 [ 17] */   1,   2,   3,   4,   5,   6,   8,
  /*      5 [ 21] */   9,  10,  11,  12,  14,  15,  16,  17,  20,  21,  22,
                      23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
  /* cnt  1 [  1] */   4,
  /* cnt  3 [ 10] */  10,  11,  14,  15,  16,  22,  25,  26,  29,  30,
  /*   6          */
  /*      3 [  9] */   0,   2,   4,   5,
  /*      5 [ 15] */   8,  10,  11,  14,  15,  16,  20,  22,  23,  25,  26,
                      27,  29,  30,  31,
  /* cnt  3 [  5] */  10,  14,  15,  25,  29,
  /*   8          */
  /*      3 [  2] */   4,
  /*      5 [  8] */  10,  14,  15,  22,  25,  26,  29,  30,
  /* cnt  3 [  1] */  14,
  /*  10          */
  /*      5 [  3] */  14,  25,  29,
};


index_into_state_for_miss _table_2_0_offset[] =
{
  /* parity = 0 */

  /*   m \ N   0                 2                 4                 6              */

  /* -10 */    0,CL(  0,  0),    0,CL(  0,  0),    0,CL(  0,  0),    1,CL(  0,  0),    1,
  /*  -8 */    1,CL(  0,  0),    1,CL(  0,  0),    1,CL(  0,  0),    4,CL(  0,  0),    4,
  /*  -6 */    4,CL(  0,  0),    4,CL(  6,  1),    6,CL(  0,  0),   12,CL(  0,  0),   12,
  /*  -4 */   13,CL(  0,  0),   13,CL( 13,  3),   18,CL(  0,  0),   31,CL(  0,  0),   31,
  /*  -2 */   34,CL( 26,  1),   34,CL( 20,  5),   41,CL(  0,  0),   60,CL(  0,  0),   60,
  /*   0 */   66,CL(  0,  0),   68,CL( 21,  4),   76,CL(  0,  0),   97,CL(  0,  0),   97,
  /*   2 */  101,CL(  0,  0),  102,CL( 25,  5),  108,CL(  0,  0),  133,CL(  0,  0),  133,
  /*   4 */  138,CL(  0,  0),  138,CL( 26,  5),  141,CL( 17,  8),  155,CL(  0,  0),  167,
  /*   6 */  180,CL(  0,  0),  180,CL( 18,  2),  181,CL( 11,  5),  190,CL(  0,  0),  199,
  /*   8 */  206,CL(  0,  0),  206,CL(  0,  0),  206,CL(  5,  2),  210,CL(  0,  0),  215,
  /*  10 */  217,CL(  0,  0),  217,CL(  0,  0),  217,CL(  0,  0),  218,CL(  0,  0),  220,

  /* parity = 1 */

  /*   m \ N   0                 1                 3                 5              */

  /* -10 */  220,CL(  0,  0),  220,CL(  0,  0),  220,CL(  0,  0),  220,CL(  0,  0),  220,
  /*  -8 */  220,CL(  0,  0),  220,CL(  0,  0),  220,CL(  0,  0),  222,CL(  0,  0),  222,
  /*  -6 */  222,CL(  0,  0),  222,CL(  0,  0),  222,CL(  0,  0),  228,CL(  0,  0),  232,
  /*  -4 */  232,CL(  0,  0),  232,CL( 19,  1),  233,CL(  0,  0),  244,CL(  0,  0),  252,
  /*  -2 */  253,CL(  0,  0),  253,CL( 26,  3),  255,CL( 14,  4),  270,CL(  0,  0),  281,
  /*   0 */  288,CL(  0,  0),  288,CL( 28,  2),  292,CL( 16,  8),  306,CL(  0,  0),  320,
  /*   2 */  330,CL(  0,  0),  330,CL( 29,  2),  333,CL( 21, 11),  343,CL(  0,  0),  362,
  /*   4 */  375,CL(  0,  0),  375,CL( 28,  1),  376,CL( 22, 10),  383,CL(  0,  0),  404,
  /*   6 */  415,CL(  0,  0),  415,CL(  0,  0),  415,CL( 15,  5),  419,CL(  0,  0),  434,
  /*   8 */  439,CL(  0,  0),  439,CL(  0,  0),  439,CL(  8,  1),  440,CL(  0,  0),  448,
  /*  10 */  449,CL(  0,  0),  449,CL(  0,  0),  449,CL(  0,  0),  449,CL(  0,  0),  452,

};

info_state_for_miss _table_2_0_info =
{
  -10, 21, 9, 7, 99,
  _table_2_0_miss,
  _table_2_0_offset,
};

/********************************************/

