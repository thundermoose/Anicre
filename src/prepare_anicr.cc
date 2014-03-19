
#include "prepare_anicr.hh"
#include "mr_base_reader.hh"
#include "mr_config.hh"

#include "mp_state_info.hh"


void prepare_anicr::set(mr_base_reader *initial)
{
  _initial = initial;
  _final = NULL;
}


void prepare_anicr::create()
{
  _initial->find_used_states();

  mp_state_info mp_info;

  _initial->find_inifin_states(mp_info);

  if (_config._td_dir)
    {
      _initial->create_code_tables(mp_info);
    }



}
