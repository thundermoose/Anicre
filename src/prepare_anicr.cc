
#include "prepare_anicr.hh"
#include "mr_base_reader.hh"



void prepare_anicr::set(mr_base_reader *initial)
{
  _initial = initial;
  _final = NULL;
}


void prepare_anicr::create()
{
  _initial->find_used_states();

  _initial->create_code_tables();



}
