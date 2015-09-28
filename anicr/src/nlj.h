typedef struct nlj_hash_item_t
{
  uint64_t _key;
  double   _value;
} nlj_hash_item;

int compare_nlj_item(const void *p1, const void *p2);

double findState2(nlj_hash_item *nlj_items, size_t num_nlj_items,int i1,int i2, int j1,int j2, int J1, int J2,int jtrans);

double norm(int na,int la,int ja,int nb,int lb,int jb,int J,int T);

void*  readDumpfile(char *filename, size_t *num_nlj_items);

nlj_hash_item *_nlj_items_nn;
size_t     _num_nlj_items_nn;
nlj_hash_item *_nlj_items_pp;
size_t     _num_nlj_items_pp;
nlj_hash_item *_nlj_items_np;
size_t     _num_nlj_items_np;
nlj_hash_item *_nlj_items_pn;
size_t     _num_nlj_items_pn;

