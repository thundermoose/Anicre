
#include "pack_mp_state.hh"
#include <stdlib.h>
#include <stdint.h>

template<typename pack_T>
pack_mp_state<pack_T>::pack_mp_state()
{
  _items = NULL;

  _words = 0;
  _len[0] = _len[1] = 0;
}

template<typename pack_T>
pack_mp_state<pack_T>::~pack_mp_state()
{
  delete[] _items;
}

template<typename pack_T>
void add_pms_item(pms_info &info, int max_index, int &words, int &offset)
{
  int bits =
    (int) (sizeof (unsigned int) * 8 - __builtin_clz(max_index));

  info._bits = (char) bits;
  if (offset + bits > (int) sizeof (pack_T) * 8)
    {
      words++;
      offset = 0;
    }
  info._shift = (char) offset;
  info._word = (char) words;

  offset += bits;
}

template<typename pack_T>
void pack_mp_state<pack_T>::setup_pack(int A0, int A1,
				       int *max_index0, int *max_index1)
{
  _items = new pms_info[A0 + A1];

  _words = 0;
  _len[0] = A0;
  _len[1] = A1;

  int offset = 0;

  for (int i = 0; i < _len[0]; i++)
    {
      add_pms_item<pack_T>(_items[i], max_index0[i], _words, offset);
    }

  for (int i = 0; i < _len[1]; i++)
    {
      add_pms_item<pack_T>(_items[i+_len[0]], max_index1[i], _words, offset);
    }

  _words++; // uninteresting case of no entries at all also uses one word...
}

// Explicit instantiation

template class pack_mp_state<uint64_t>;
