/*
  cd ~/mfrtd ; g++ -Wall -W -o greedyorder greedyorder.cc -O3
 */


#include <stdio.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

struct array_t;
struct cblock_t;

typedef std::vector<array_t *> vect_array_ptr;
typedef std::vector<cblock_t *> vect_cblock_ptr;

typedef std::set<array_t *> set_array_ptr;
typedef std::set<cblock_t *> set_cblock_ptr;

typedef std::vector<int> vect_array_id;

struct array_t
{
  uint64_t        _size;
  set_cblock_ptr  _users;
};

struct cblock_t
{
  cblock_t       *_checked; // temporary while checking siblings
  uint64_t        _tot_size;
  vect_array_id   _aids;
  vect_array_ptr  _arrays; 
};

typedef std::map<int,array_t *> map_array_ptr;

map_array_ptr  _arrayids;
set_cblock_ptr _cblockids;

int main()
{


  while (!feof(stdin))
    {
      char line[1024];

      fgets (line, sizeof (line), stdin);

      char *start = strstr(line, "ARRAY:");

      if (start)
	{
	  int array_no;
	  uint64_t array_sz;

	  int n = sscanf(start+6, "%d=%" SCNu64 "", &array_no, &array_sz);

	  if (n == 2)
	    {
	      array_t *array = new array_t;

	      array->_size = array_sz;
	      
	      _arrayids.insert(map_array_ptr::value_type(array_no, array));
	    }
	  else
	    goto bad_line;
	}

      start = strstr(line, "CALCBLOCK:");

      if (start)
	{
	  int a[5];

	  int n = sscanf(start+10,
			 "%d,%d,%d,%d,%d",
			 &a[0], &a[1], &a[2], &a[3], &a[4]);

	  if (n == 4 || n == 5)
	    {
	      cblock_t *cblock = new cblock_t;

	      for (int i = 0; i < n; i++)
		{
		  cblock->_aids.push_back(a[i]);
		}
	      cblock->_checked = NULL;

	      _cblockids.insert(cblock);
	    }
	  else
	    goto bad_line;
	}

      continue;
    bad_line:
      fprintf (stderr, "BAD LINE: %s\n", line);
      exit(1);
    }

  printf ("NUM-ARRAYS: %zd\n", _arrayids.size());
  printf ("NUM-BLOCKS: %zd\n", _cblockids.size());

  uint64_t sumraworder = 0;
  uint64_t maxblocksize = 0;

  for (set_cblock_ptr::iterator cbiter = _cblockids.begin();
       cbiter != _cblockids.end(); ++cbiter)
    {
      cblock_t *cblock = *cbiter;

      cblock->_tot_size = 0;

      for (size_t i = 0; i < cblock->_aids.size(); i++)
	{
	  map_array_ptr::iterator iter;

	  iter = _arrayids.find(cblock->_aids[i]);

	  if (iter == _arrayids.end())
	    fprintf (stderr, "Cannot find array #%d\n", cblock->_aids[i]);

	  array_t *array = iter->second;

	  cblock->_arrays.push_back(iter->second);

	  sumraworder += array->_size;
	  cblock->_tot_size += array->_size;

	  array->_users.insert(cblock);
	}

      std::sort(cblock->_arrays.begin(), cblock->_arrays.end());

      if (cblock->_tot_size > maxblocksize)
	maxblocksize = cblock->_tot_size;
    }

  printf ("RAW-LOAD-SIZE: %" PRIu64 "\n", sumraworder);
  printf ("MAX-BLOCK-SIZE: %" PRIu64 "\n", maxblocksize);

  /* We employ a greedy approach to choosing blocks to calculate.
   * When nothing else applies, (and at startup) we begin with the
   * totally largest block (i.e. largest sum array size) When
   * selecting a block to move to, we choose the one that reuses the
   * most information, and of those possible, the one that loads the
   * most.  I.e., we overall begin with large arrays.
   */

  uint64_t sumgreedyload = 0;

  while (!_cblockids.empty())
    {
      /* We are at start, or have no better idea.  Find largest one.
       * Expensive to evaluate...
       */

      cblock_t *largestid = NULL;
      uint64_t largestsz = 0;

      for (set_cblock_ptr::iterator cbiter = _cblockids.begin();
	   cbiter != _cblockids.end(); ++cbiter)
	{
	  cblock_t *cblock = *cbiter;

	  uint64_t sz = 0;

	  for (size_t i = 0; i < cblock->_arrays.size(); i++)
	    {
	      array_t *array = cblock->_arrays[i];
	      sz += array->_size;
	    }

	  if (sz > largestsz)
	    {
	      largestid = cblock;
	      largestsz = sz;
	    }
	}

      printf ("%p : %" PRIu64 "\n", largestid, largestsz);

      cblock_t *currentid = largestid;

      sumgreedyload += largestsz;

      _cblockids.erase(currentid);

      /* Now try to find a block which reuses some information.
       */

      for ( ; ; )
	{
	  set_array_ptr curarrays;

	  curarrays.insert(currentid->_arrays.begin(),
			   currentid->_arrays.end());

	  /* First remove us from the list of siblings.
	   */

	  for (size_t i = 0; i < currentid->_arrays.size(); i++)
	    {
	      array_t *array = currentid->_arrays[i];

	      /* We are no longer a user.*/

	      array->_users.erase(currentid);	      
	    }

	  /* Instead of first creating a combined list with all
	   * siblings, it is much faster to just go through all the
	   * lists.
	   */

	  cblock_t *bestblockid = NULL;
	  uint64_t bestunloadsz = 0;
	  uint64_t bestloadsz = 0;

	  for (size_t i = 0; i < currentid->_arrays.size(); i++)
	    {
	      array_t *array = currentid->_arrays[i];

	      set_cblock_ptr &siblings = array->_users;

	      for (set_cblock_ptr::iterator scbiter = siblings.begin();
		   scbiter != siblings.end(); ++scbiter)
		{
		  cblock_t *siblingid = *scbiter;
		  
		  /* This does not improve things too much.  even negative? */
		  if (siblingid->_checked == currentid)
		    continue;
		  siblingid->_checked = currentid;
		  
		  uint64_t loadsz = 0;

		  for (size_t i = 0; i < siblingid->_arrays.size(); i++)
		    {
		      array_t *array = siblingid->_arrays[i];

		      /* Does the current know about this array? */

		      if (curarrays.find(array) == curarrays.end())
			{
			  loadsz += array->_size;
			}
		    }
		  
		  uint64_t reusesz = siblingid->_tot_size - loadsz;
		  uint64_t unloadsz = currentid->_tot_size - reusesz;

		  if (!bestblockid ||
		      loadsz < bestloadsz)
		    {
		      bestblockid = siblingid;
		      bestunloadsz = unloadsz;
		      bestloadsz = loadsz;
		    }
		}
	    }

	  if (!bestblockid)
	    break;
	  /*
	  printf ("%p : %" PRIu64 " : %" PRIu64 "\n",
		  bestblockid, bestunloadsz, bestloadsz);
	  */
	  (void) bestunloadsz;

	  currentid = bestblockid;

	  sumgreedyload += bestloadsz;

	  _cblockids.erase(currentid);

	  if (_cblockids.size() % 500 == 0)
	    {
	      printf ("%zd  \r", _cblockids.size());
	      fflush(stdout);
	    }
	}
    }

  printf ("GREEDY-LOAD-SIZE: %" PRIu64 "\n", sumgreedyload);

#if 0

while (scalar keys %cblockids)
{
    # Now try to find a block which reuses some information

  find_sibling:
    for ( ; ; )
    {
	my %curarrays = map { $_, 1 } @{$curblockref};

	my $bestblockid = ();
	my $bestunloadsz = 0;
	my $bestloadsz = 0;

      try_sibling:

    }
}

#endif
}
