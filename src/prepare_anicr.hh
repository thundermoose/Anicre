
#ifndef __PREPARE_ANICR_HH__
#define __PREPARE_ANICR_HH__

class mr_base_reader;

class prepare_anicr
{

public:
  mr_base_reader *_initial;
  mr_base_reader *_final;

public:
  void set(mr_base_reader *initial);

  void create();

};

#endif/*__PREPARE_ANICR_HH__*/
