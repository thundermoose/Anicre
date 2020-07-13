typedef struct nlj_hash_item_t
{
  uint64_t _key;
  double   _value;
} nlj_hash_item;

int compare_nlj_item(const void *p1, const void *p2);

double findState2(nlj_hash_item *nlj_items, size_t num_nlj_items,int i1,int i2, int j1,int j2, int J1, int J2,int jtrans);

double norm(int na,int la,int ja,int nb,int lb,int jb,int J,int T);

void*  readDumpfile(char *filename, size_t *num_nlj_items);

extern nlj_hash_item *_nlj_items_nn;
extern size_t     _num_nlj_items_nn;
extern nlj_hash_item *_nlj_items_pp;
extern size_t     _num_nlj_items_pp;
extern nlj_hash_item *_nlj_items_np;
extern size_t     _num_nlj_items_np;
extern nlj_hash_item *_nlj_items_pn;
extern size_t     _num_nlj_items_pn;

typedef struct {
  double _nn;
  double _pp;
  double _np;
} retval;
retval computeres(int i1, int i2,int j1,int j2,int Jab,int Jcd,int jtrans,int Tab,int Tcd,double mult);
