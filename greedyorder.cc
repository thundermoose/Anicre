/*
  cd ~/mfrtd ; g++ --std=c++11 -Wall -W -o greedyorder greedyorder.cc -O3
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
#include <assert.h>

struct array_t;
struct cblock_t;

typedef std::vector<array_t *> vect_array_ptr;
typedef std::vector<cblock_t *> vect_cblock_ptr;

typedef std::set<array_t *> set_array_ptr;
typedef std::set<cblock_t *> set_cblock_ptr;

typedef std::vector<int> vect_array_id;

struct array_t
{
  uint64_t         _size;
  vect_cblock_ptr  _users;
  bool             _used;
};

struct array_ptr_size_t
{
  array_t *_ptr;
  uint64_t _size;
};

typedef std::vector<array_ptr_size_t> vect_array_ptr_size;

struct cblock_t
{
  cblock_t       *_checked; // temporary while checking siblings
  uint64_t        _tot_size;
  /*
  int E_p_in;
  int E_n_in;
  int M_p_in;
  int M_n_in;
  int E_p_out;
  int E_n_out;
  int M_p_out;
  int M_n_out;
  int depth_p;
  int depth_n;
  */
  vect_array_id   _aids;
  vect_array_ptr  _arrays; 
};

typedef std::map<int,array_t *> map_array_ptr;

map_array_ptr  _arrayids;
set_cblock_ptr _cblockids;

enum interaction_type : int {n=0,nn=1,nnn=2,p=3,pp=4,ppp=5,np=6,npp=7,nnp=8};


void print_cblock(cblock_t *cblock){
  
  printf("BLOCK: ");
  for (int v : cblock->_aids){
    printf("%d ",v);
  }
  printf("\n");
}

void print_cblock_unload(cblock_t *cblock){
  printf("BLOCK_UNLOAD: ");
  for (int v : cblock->_aids){
    printf("%d ",v);
  }
  printf("\n");
}

int main(int argc, char *argv[])
{
  int maxforce = 0;
  int blocksize = 1;
 
  if (argc < 3)
    {
      fprintf (stderr, "  %s <maxforce> <blocksize>\n", argv[0]);
      exit(1);
    }
  maxforce = atoi(argv[1]);
  blocksize = atoi(argv[2]);
  /*
  interaction_type current_int_type;
  bool is_diagonal = false;
  */
  while (!feof(stdin))
    {
      char line[1024];

      fgets (line, sizeof (line), stdin);

      char *start = strstr(line, "ARRAY:");
      char *startmp = strstr(line, "ARRAYMP:");

      if (start || startmp)
	{
	  int array_no;
	  uint64_t array_sz;
	  bool mparray = false;

	  if (start)
	    start += 6;
	  else
	    {
	      start = startmp + 8;
	      mparray = true;
	    }

	  int n = sscanf(start, "%d=%" SCNu64 "", &array_no, &array_sz);

	  if (n == 2)
	    {
	      array_t *array = new array_t;

	      if (mparray)
		array_sz *= blocksize;

	      array->_size = array_sz;
	      array->_used = false;
	      
	      _arrayids.insert(map_array_ptr::value_type(array_no, array));
	    }
	  else
	    goto bad_line;
	}

      start = strstr(line, "CALCBLOCK:");

      if (start)
	{
	  int nforce;
	  int a[5];

	  int n = sscanf(start+10,
			 "%d:%d,%d,%d,%d,%d",
			 &nforce,
			 &a[0], &a[1], &a[2], &a[3], &a[4]);

	  if (nforce > maxforce)
	    continue;

	  if (n == 5 || n == 6)
	    {
	      cblock_t *cblock = new cblock_t;
	      /*
	      if (is_diagonal)
		{
		  if (current_int_type<3)
		    {
		      // the differing particles are neutrons
		    }
		  else
		    {
		      // the differing particles are protons
		    }
		  
		}
	      else
		{
		  sscanf(line,
			 " %d %d %d %d %d "
			 " %d %d %d %d %d ",
			 &cblock->E_p_in,
			 &cblock->M_p_in,
			 &cblock->E_n_in,
			 &cblock->M_n_in,
			 &cblock->depth_p,
			 &cblock->E_p_out,
			 &cblock->M_p_out,
			 &cblock->E_n_out,
			 &cblock->M_n_out,
			 &cblock->depth_n);
		}
	      */
	      
	      for (int i = 0; i < n-1; i++)
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

  printf ("NUM-BLOCKS-%d-%d: %zd\n",
	  maxforce, blocksize,
	  _cblockids.size());

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

	  array->_users.push_back(cblock);
	  array->_used = true;
	}

      std::sort(cblock->_arrays.begin(), cblock->_arrays.end());

      if (cblock->_tot_size > maxblocksize)
	maxblocksize = cblock->_tot_size;
    }

  printf ("RAW-LOAD-SIZE-%d-%d: %" PRIu64 "\n",
	  maxforce, blocksize,
	  sumraworder);
  printf ("MAX-BLOCK-SIZE-%d-%d: %" PRIu64 "\n",
	  maxforce, blocksize,
	  maxblocksize);

  uint64_t sumarraysize = 0;
  size_t usedarrays = 0;

  for (map_array_ptr::iterator iter = _arrayids.begin();
       iter != _arrayids.end(); ++iter)
    {
      array_t *array = iter->second;

      if (array->_used)
	{
	  sumarraysize += array->_size;
	  usedarrays++;
	}
    }

  printf ("NUM-ARRAYS-%d-%d: %zd\n",
	  maxforce, blocksize,
	  usedarrays);
  printf ("SUM-ARRAY-SIZE-%d-%d: %" PRIu64 "\n",
	  maxforce, blocksize,
	  sumarraysize);

  /* We employ a greedy approach to choosing blocks to calculate.
   * When nothing else applies, (and at startup) we begin with the
   * totally largest block (i.e. largest sum array size) When
   * selecting a block to move to, we choose the one that reuses the
   * most information, and of those possible, the one that loads the
   * most.  I.e., we overall begin with large arrays.
   */

  uint64_t sumgreedyload = 0;
  cblock_t *prev_loaded = NULL;
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
      
      //printf ("%p : %" PRIu64 "\n", largestid, largestsz);
      if (prev_loaded == NULL){
	prev_loaded = largestid;
      }else{
	printf("UNLOAD_");
	print_cblock(prev_loaded);
	prev_loaded = largestid;
      }
      print_cblock(largestid);
      
      
      cblock_t *currentid = largestid;

      sumgreedyload += largestsz;

      currentid->_tot_size = 0;
      _cblockids.erase(currentid);

      /* Now try to find a block which reuses some information.
       */

      vect_array_ptr_size curarrays;

      for ( ; ; )
	{
#define MAX_CUR_ARRAYS 10

	  array_ptr_size_t curarrays[MAX_CUR_ARRAYS];

	  size_t ncur = currentid->_arrays.size();

	  assert(MAX_CUR_ARRAYS >= ncur);

	  for (size_t i = 0; i < currentid->_arrays.size(); i++)
	    {
	      array_t *array = currentid->_arrays[i];

	      curarrays[i]._ptr = array;
	      curarrays[i]._size = array->_size;
	    }

	  /* Instead of first creating a combined list with all
	   * siblings, it is much faster to just go through all the
	   * lists.
	   */

	  cblock_t *bestblockid = NULL;
	  uint64_t bestunloadsz = 0;
	  uint64_t bestloadsz = (uint64_t) -1;

	  for (size_t i = 0; i < currentid->_arrays.size(); i++)
	    {
	      array_t *array = currentid->_arrays[i];

	      vect_cblock_ptr &siblings = array->_users;

	      for (vect_cblock_ptr::iterator scbiter = siblings.begin();
		   scbiter != siblings.end(); ++scbiter)
		{
		  cblock_t *siblingid = *scbiter;
		  
		  if (!siblingid->_tot_size) // already handled
		    continue;
		  /* This does not improve things too much.  even negative? */
		  if (siblingid->_checked == currentid)
		    continue;
		  siblingid->_checked = currentid;
		  
		  uint64_t reusesz = 0;

		  /* We figure out which items we need to load by a
		   * merge-sort style loop over the arrays.
		   */

		  size_t nsib = siblingid->_arrays.size();
		  array_t **sibarrays = &siblingid->_arrays[0];

		  size_t isib = 0, icur = 0;

		  while (isib < nsib && icur < ncur)
		    {
		      if (sibarrays[isib] < curarrays[icur]._ptr)
			{
			  isib++;
			}
		      else if (sibarrays[isib] > curarrays[icur]._ptr)
			{
			  icur++;			  
			}
		      else // equal!
			{
			  reusesz += curarrays[icur]._size;

			  isib++;
			  icur++;
			}
		    }
		  // any remaining isib or icur cannot match!
		  
		  uint64_t loadsz   = siblingid->_tot_size - reusesz;
		  uint64_t unloadsz = currentid->_tot_size - reusesz; 

		  if (loadsz < bestloadsz)
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
	  printf ("%p : -%" PRIu64 " : %" PRIu64 "\n",
	  bestblockid, -bestunloadsz, bestloadsz);*/
	  if (prev_loaded == NULL){
	    prev_loaded = bestblockid;
	  }else{
	    printf("UNLOAD_");
	    print_cblock(prev_loaded);
	    prev_loaded = bestblockid;
	  }
	  print_cblock(bestblockid);
	  
	   (void) bestunloadsz; 

	  currentid = bestblockid;

	  sumgreedyload += bestloadsz;

	  currentid->_tot_size = 0;
	  _cblockids.erase(currentid);

	  //if (_cblockids.size() % 500 == 0)
	  //  {
	  //    printf ("%zd  \r", _cblockids.size());
	  //    fflush(stdout);
	  //  }
	}
    }

  printf ("GREEDY-LOAD-SIZE-%d-%d: %" PRIu64 "\n",
	  maxforce, blocksize,
	  sumgreedyload);

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
