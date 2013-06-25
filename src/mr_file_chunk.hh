
#ifndef __MR_FILE_CHUNK_HH__
#define __MR_FILE_CHUNK_HH__

template<typename item_t>
class mr_file_chunk
{
public:
  mr_file_chunk(unsigned int end,
		 unsigned int chunk,
		 unsigned int stride = 1)
  {
    _end = end;
    _chunk = chunk;
    _stride = stride;

    _cur = -chunk;
  }

  ~mr_file_chunk()
  {
    _h.unmap();
  }

protected:
  item_t *_ptr;

  unsigned int _end;
  unsigned int _chunk;
  unsigned int _stride;

  unsigned int _cur;
  unsigned int _num;

  mr_mapped_data _h;

public:
  item_t      *ptr()   { return _ptr; } /* Ptr to current chunk data. */

  unsigned int start() { return _cur; } /* Current start. */
  unsigned int num()   { return _num; } /* Current chunk. */

protected:
  bool next()
  {
    _cur += _chunk;

    if (_cur >= _end)
      return false;

    _num = _chunk;

    if (_num > _end - _cur)
      _num = _end - _cur;

    return true;
  }

  void do_map(mr_file_reader *_file_reader, uint64_t offset_data)
  {
    _ptr = (item_t *)
      MAP_BLOCK_DATA(offset_data +
		     _cur * _stride * sizeof (item_t),
		     _num * _stride * sizeof (item_t), _h);
  }

public:
  bool map_next(mr_file_reader *_file_reader, uint64_t offset_data)
  {
    _h.unmap(); // in case we were used before

    if (!next())
      return false;

    do_map(_file_reader, offset_data);

    return true;
  }

  void map(mr_file_reader *_file_reader, uint64_t offset_data,
	   unsigned int begin,unsigned int len)
  {
    _cur = begin;
    _num = len;

    do_map(_file_reader, offset_data);
  }

};


#endif//__MR_FILE_CHUNK_HH__
