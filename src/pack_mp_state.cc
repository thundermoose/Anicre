
#include "pack_mp_state.hh"
#include <stdlib.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

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

  if (!max_index)
    bits = 0;

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

template<typename pack_T>
void pack_mp_state<pack_T>::clear_packed(pack_T *pack)
{
  for (int i = 0; i < _words; i++)
    pack[i] = 0;
}

template<typename pack_T>
void pack_mp_state<pack_T>::insert_packed(pack_T *pack, int i, int value)
{
  pms_info &info = _items[i];

  pack[(int) info._word] |= ((pack_T) value) << info._shift;
}

template<typename pack_T>
void pack_mp_state<pack_T>::generate_code(file_output &out)
{
  out.fprintf ("void packed_to_int_list(int *list, uint64_t *packed)\n");
  out.fprintf ("{\n");
  for (int i = 0; i < _len[0] + _len[1]; i++)
    {
      out.fprintf ("  list[%2d] = (int) (packed[%d] >> %2d) & 0x%llx;\n",
		   i,
		   _items[i]._word,
		   _items[i]._shift,
		   ((unsigned long long) 1 << _items[i]._bits) - 1);

    }
  out.fprintf ("}\n");

  out.fprintf ("\n");

  out.fprintf ("void int_list_to_packed(uint64_t *packed, int *list)\n");
  out.fprintf ("{\n");
  for (int i = 0; i < _words; i++)
    out.fprintf ("  packed[%d] = 0;\n", i);
  for (int i = 0; i < _len[0] + _len[1]; i++)
    {
      out.fprintf ("  packed[%d] |= (((uint64_t) list[%2d]) << %2d);\n",
		   _items[i]._word,
		   i,
		   _items[i]._shift);

    }
  out.fprintf ("}\n");

  out.fprintf ("\n");

  out.fprintf ("void int_list2_to_packed(uint64_t *packed, "
	       "int *list0, int *list1)\n");
  out.fprintf ("{\n");
  for (int i = 0; i < _words; i++)
    out.fprintf ("  packed[%d] = 0;\n", i);
  for (int i = 0; i < _len[0]; i++)
    {
      out.fprintf ("  packed[%d] |= (((uint64_t) list0[%2d]) << %2d);\n",
		   _items[i]._word,
		   i,
		   _items[i]._shift);
    }
  for (int i = 0; i < _len[1]; i++)
    {
      out.fprintf ("  packed[%d] |= (((uint64_t) list1[%2d]) << %2d);\n",
		   _items[i+_len[0]]._word,
		   i,
		   _items[i+_len[0]]._shift);
    }
  out.fprintf ("}\n");

  out.fprintf ("\n");


}

template<typename pack_T>
void pack_mp_state<pack_T>::generate_tables(file_output &out)
{
  out.fprintf("/********************************************/\n");
  out.fprintf("\n");
  out.fprintf("mp_pack_info _mp_pack_info[] = \n");
  out.fprintf("{\n");

  for (int i = 0; i < _len[0] + _len[1]; i++)
    {
      out.fprintf ("  { /* %2d */ %d, %2d, 0x%016"PRIx64"ull },\n",
		   i,
		   _items[i]._word,
		   _items[i]._shift,
		   ((((uint64_t) 1) << _items[i]._bits)-1) <<
		   _items[i]._shift);
    }

  out.fprintf("};\n");
  out.fprintf("\n");


}


// Explicit instantiation

template class pack_mp_state<uint64_t>;
