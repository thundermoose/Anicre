#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mr_file_reader.hh"
#include "mr_base_reader.hh"
#include "magic_antoine_read.hh"

#define countof(x) (sizeof(x)/sizeof(x[0]))

int _debug = 0;

mr_base_reader *identify_file(mr_file_reader *file_reader)
{
  mr_base_reader *readers[] = {
    new mr_antoine_reader<mr_antoine_header_old_t>(file_reader),
    new mr_antoine_reader<mr_antoine_header_new_t>(file_reader),
  };

  mr_base_reader *matching = NULL;

  int matches = 0;

  for (int i = 0; i < (int) countof(readers); i++)
    {
      printf ("Trying format '%s' for '%s'.\n",
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
      fprintf (stderr,
	       "Multiple format matches for file '%s':\n",
	       file_reader->_filename);
      bool firstmatch = true;
      for (int i = 0; i < (int) countof(readers); i++)
	{
	  if (readers[i])
	    fprintf (stderr,
		     "%s%s",
		     firstmatch ? "" : ", ",
		     readers[i]->get_format_name());
	  firstmatch = false;
	}
      fprintf (stderr,
	       "\n");
      return NULL;
    }

  if (matching)
    printf ("Found format '%s' for '%s'.\n",
	    matching->get_format_name(),file_reader->_filename);

  return matching;
}

void usage(char *cmdname)
{
  printf ("\n");
  printf ("Reading of m-scheme data files.\n\n");
  printf ("%s [options] infile\n\n",cmdname);
  printf ("  -d[=N]             Debug level\n");
  printf ("\n");
}

int main(int argc,char *argv[])
{
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
      else {
	/* Input file, we hope. */
	/*
	if (_filename)
	  ERROR("Input file already given.");
	*/
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
      fprintf (stderr,
	       "No format match for file '%s'.\n",
	       file_reader->_filename);
      exit(1);
    }

  if (!reader->level2_read())
    {
      fprintf (stderr,
	       "File %s does not match detailed expectations "
	       "for format '%s'.\n",
	       reader->_file_reader->_filename,
	       reader->get_format_name());
      exit(1);
    }

  reader->dump_info();

  // antoine.

  return 0;
}
