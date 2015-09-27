
typedef struct couple_item_t
{
  uint64_t _nlj_key;
  int      _fact_anni_crea; /* x1: anni_nlj_same, x2: crea_nlj_same */
  double   _value;

} couple_item;



void alloc_couple_items(size_t max_anni, size_t max_crea);


typedef struct couple_j_item_t
{
  int _j;
  double _val;
} couple_j_item;
