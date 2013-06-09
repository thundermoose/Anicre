
#include "error.hh"

#include "thread_info_window.hh"

#include "worker_thread.hh"
#include "thread_buffer.hh"
#include "colourtext.hh"

#include <sys/types.h>

#include <stdio.h>  // This must be first include

#define FE_DATA_INIT { FE_UNKNOWN,0,0,0 }

#ifdef USE_PTHREAD
#ifdef HAVE_THREAD_LOCAL_STORAGE
__thread formatted_error __fe = FE_DATA_INIT;
#else
// We do not have the __thread attribute, so will go via
// pthread_getspecific
pthread_key_t _fe_key;
pthread_once_t _fe_key_once = PTHREAD_ONCE_INIT;

void fe_key_destroy(void * buf)
{
  formatted_error *data = (formatted_error *) buf;
  delete data;
}

void fe_key_alloc()
{
  pthread_key_create(&_fe_key,fe_key_destroy);
}

void fe_init()
{
  pthread_once(&_fe_key_once,fe_key_alloc);

  formatted_error *data = new formatted_error;

  formatted_error data_init = FE_DATA_INIT;

  *data = data_init;

  pthread_setspecific(_fe_key,data);
}
#endif
#else
formatted_error __fe = FE_DATA_INIT;
#endif

void formatted_error::realloc(size_t newlen)
{
  char *newbuf = (char*) ::realloc (_buffer,newlen);
  
  if (newbuf == NULL)
    {
      // we'll silently fail... (hmmm...)
      // we are not allowed to call the ERROR routine, since we may already be there!
      fprintf (stderr,"Memory allocation failure! (in error printing routine)");
      exit(1);
    }
  
  _buffer = newbuf;
  _alloc = newlen;
}

// code stolen from: Linux Programmer's Manual PRINTF(3)   

void formatted_error::make_message(const char *fmt,va_list ap)
{
  /* Guess we need no more than 100 bytes. */
  int n;
  ssize_t size = _alloc - _length;

  while (1) {
    /* We must make a copy, since amd64 (and ppc?)
     * choke on multiple traversals
     */
    va_list aq;
#ifdef va_copy
    va_copy(aq,ap);
#else
    __va_copy(aq,ap);
#endif
    /* Try to print in the allocated space. */
    n = vsnprintf (_buffer+_length, size, fmt, aq);
    va_end(aq);
    /* If that worked, return the string. */
    if (n > -1 && n < size)
      {
	_length += n;
	return;
      }
    /* Else try again with more space. */
    if (n > -1)    /* glibc 2.1 */
      size = n+1; /* precisely what is needed */
    else           /* glibc 2.0 */
      size = (size + 1) * 2;  /* twice the old size, plus something, since we may have started 0 */

    realloc(_length+size);
  }
}

void formatted_error::printf(const char *fmt,...)
{
  va_list ap;

  assert(fmt);

  char *bold_fmt = markconvbold(fmt);

  va_start(ap, fmt);
  make_message(bold_fmt ? bold_fmt : fmt,ap);
  va_end(ap);

  // text(p);
  // free(p);

  free(bold_fmt);
}

#ifdef USE_PROGRESS
#endif

void formatted_error::append(int c)
{
  if (_length+2/*one for char, one for '\0'*/ > _alloc)
    realloc(_length+2);

  _buffer[_length] = (char) c;
  _length++;
  _buffer[_length] = 0;
}



struct error_reclaim
{
  fe_msgtype   _type;
  size_t       _length;
  char         _message[0];
};

extern thread_info_window *_ti_info_window;

void formatted_error::eject()
{
  if (!_length)
    return; // this would usually not happen, but protects against NULL buffer

  _buffer[_length] = 0;

#ifdef USE_THREADING
  // We must queue the error message along with the currently handled event
  // If we have a current last_reclaim-pointer, then we can enqueue the error
  // message into that

  if (_wt._last_reclaim)
    {
      size_t need = sizeof (error_reclaim) + _length + 1;

      error_reclaim *er = 
	(error_reclaim*) _wt._defrag_buffer->allocate_reclaim(need,RECLAIM_MESSAGE);

      er->_type   = _type;
      er->_length = _length;
      memcpy(er->_message,_buffer,_length+1);
    }
#if defined(USE_THREADING) && defined(USE_PROGRESS)
  else if (_ti_info_window)
    {
      _ti_info_window->add_error(_buffer,_type);
    }
#endif
  else
#endif
    {
      markconvbold_output(_buffer,
			  _type == FE_ERROR ? CTR_WHITE_BG_RED :
			  _type == FE_WARNING ? CTR_BLACK_BG_YELLOW :
			  CTR_NONE);
    }
}

#if defined(USE_PROGRESS) || defined(USE_THREADING)
void reclaim_eject_message(tb_reclaim *tbr)
{
  error_reclaim *er = (error_reclaim *) (tbr+1);

#if defined(USE_THREADING) && defined(USE_PROGRESS)

  if (_ti_info_window)
    {
      _ti_info_window->add_error(er->_message,er->_type);
    }
  else
#endif
    {
      fprintf (stderr,"%s\n",er->_message);
    }

  tbr->_buffer->reclaim(tbr->_reclaim);
}
#endif
