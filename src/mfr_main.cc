#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.hh"
#include "colourtext.hh"
#include "mr_file_reader.hh"
#include "mr_base_reader.hh"
#include "antoine_read.hh"
#include "antoine_vector_read.hh"
#include "prepare_anicr.hh"

#include "mr_config.hh"

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

int _debug = 0;
int _t_2_initial=-1;
int _t_2_final=-1;
double _hw=0.0;
int _nporder=1;
void check_create_dir(const char *dir)
{
  struct stat buf;

  int ret = stat (dir, &buf);

  if (ret == -1)
    {
      if (errno == ENOENT)
	{
	  // Split of the last item, and try to create it, after
	  // checking the parent.

	  const char *lastslash = strrchr(dir,'/');
	  if (lastslash != NULL)
	    {
	      char *parent = strndup(dir, lastslash - dir);

	      check_create_dir(parent);
	    }

	  ret = mkdir (dir, S_IRWXU | S_IRWXG | S_IRWXO);

	  if (ret != 0)
	    {
	      perror("mkdir");
	      ERROR("Failed to create directory '%s'.", dir);
	    }

	  INFO("Created directory '%s'.", dir);
	}
      else
	{
	  perror("stat");
	  ERROR("Not a directory: %s.", dir);
	}
    }
  else
    {
      if (!S_ISDIR(buf.st_mode))
	{
	  ERROR("Not a directory: %s.", dir);
	}
    }
}

mr_base_reader *identify_file(mr_file_reader *file_reader)
{
  mr_base_reader *readers[] = {
    new mr_antoine_reader<mr_antoine_header_old_t,
			  mr_antoine_fon_old_t>(file_reader),
    new mr_antoine_reader<mr_antoine_header_new_t,
			  mr_antoine_fon_new_t>(file_reader),
    new mr_antoine_vector_reader<1>(file_reader),
  };

  mr_base_reader *matching = NULL;

  int matches = 0;

  for (int i = 0; i < (int) countof(readers); i++)
    {
      INFO("Trying format '%s' for '%s'.",
	   readers[i]->get_format_name(),file_reader->_filename);
      if (!readers[i]->level1_read())
	{
	  delete readers[i];
	  readers[i] = NULL;
	}
      else
	{
	  matches++;
	  matching = readers[i];
	}
    }

  if (matches > 1)
    {
      WARNING("Multiple format matches for file '%s':",
	      file_reader->_filename);
      bool firstmatch = true;
      for (int i = 0; i < (int) countof(readers); i++)
	{
	  if (readers[i])
	    WARNING("%s%s",
		    firstmatch ? "" : ", ",
		    readers[i]->get_format_name());
	  firstmatch = false;
	}
      return NULL;
    }

  if (matching)
    INFO("Found format '%s' for '%s'.",
	 matching->get_format_name(),file_reader->_filename);

  return matching;
}

void usage(char *cmdname)
{
  printf ("\n");
  printf ("Reading of m-scheme data files.\n\n");
  printf ("%s [options] infile\n\n",cmdname);
  printf ("  -d[=N]             Debug level.\n");
  printf ("  --colour=yes|no    Force colour and markup on or off.\n");
  printf ("  --dump=full        Level of text output.\n");
  printf ("  --td-dir=PATH      (Temporary) directory for generated tables and code.\n");
  printf ("  --Ti=2*T           Set isospin for initial state\n");
  printf ("  --Tf=2*T           Set isospin for final state, if not given Ti=Tf will be assumed.\n");
  printf ("  --hw=FREQ          Set H.O. frequency. (hw=0.0) \n");
  printf ("  --np=1|0      Are the wavefunctions given in NP- or PN-order. (nporder=True) \n"); 
  printf ("  --help             Print this usage information and quit.\n");
  printf ("\n");
}

mr_config_t _config;

int main(int argc,char *argv[])
{
  colourtext_init();

  memset(&_config, 0, sizeof(_config));

  const char *_filename = NULL;

 
  for (int i = 1; i < argc; i++)
    {
      char *post;

#define MATCH_PREFIX(prefix,post) (strncmp(argv[i],prefix,strlen(prefix)) == 0 && *(post = argv[i] + strlen(prefix)) != '\0')
#define MATCH_ARG(name) (strcmp(argv[i],name) == 0)

      if (MATCH_ARG("--help")) {
        usage(argv[0]);
        exit(0);
      }
      else if (MATCH_ARG("-d")) {
	_debug = 1;
      }
      else if (MATCH_PREFIX("-d=",post)) {
	_debug = atoi(post);
      }
      else if (MATCH_PREFIX("--colour=",post) ||
	       MATCH_PREFIX("--color=",post)) {
        int force = 0;

        if (strcmp(post,"yes") == 0)
          force = 1;
        else if (strcmp(post,"no") == 0)
          force = -1;
        else if (strcmp(post,"auto") != 0)
          ERROR("Bad option '%s' for --colour=",post);

        colourtext_setforce(force);
      }
      else if (MATCH_PREFIX("--Ti=",post)){
	_t_2_initial=atoi(post);
	printf("2*T_initial=%d \n",_t_2_initial);
      }
      else if (MATCH_PREFIX("--Tf=",post)){
	_t_2_final=atoi(post);
	printf("2*T_initial=%d \n",_t_2_final);

      }
      else if (MATCH_PREFIX("--hw=",post)){
	_hw=atof(post);
      }
      else if (MATCH_PREFIX("--np=",post)){
	_nporder=atoi(post);
      }
      else if (MATCH_PREFIX("--dump=",post)) {
	if (strcmp(post,"full") == 0)
	  _config._dump = DUMP_FULL;
	else
	  FATAL("Bad dump request '%s'.",post);
      }
      else if (MATCH_PREFIX("--td-dir=",post)) {
	char *dir = strdup(post);
	/* Kill any trailing slash. */
	char *lastslash = strrchr(dir,'/');
	if (lastslash != NULL &&
	    *(lastslash+1) == 0)
	  *(lastslash) = 0;
	_config._td_dir = dir;
      }
      else {
	/* Input file, we hope. */
	
	if (_filename)
	  FATAL("Bad argument '%s', or input file already given.",
		argv[i]);
	
	_filename = argv[i];
      }
    }

  if (!_filename)
    {
      usage(argv[0]);
      exit(1);
    }

  mr_file_reader *file_reader = new mr_file_reader;

  file_reader->open(_filename);

  mr_base_reader *reader = identify_file(file_reader);

  if (!reader)
    {
      FATAL("No format match for file '%s'.",
	    file_reader->_filename);
    }

  if (!reader->level2_read())
    {
      FATAL("File %s does not match detailed expectations "
	    "for format '%s'.",
	    reader->_file_reader->_filename,
	    reader->get_format_name());
    }

  if (_config._td_dir)
    check_create_dir(_config._td_dir);

  reader->dump_info();

  prepare_anicr prep_anicr;

  prep_anicr.set(reader);

  prep_anicr.create();

  // antoine.

  delete reader;
  delete file_reader;

  return 0;
}
