#include <stdio.h>
#include <stdlib.h>

#include "mr_file_reader.hh"
#include "mr_base_reader.hh"
#include "magic_antoine_read.hh"

#define countof(x) (sizeof(x)/sizeof(x[0]))

int _debug = 2;






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

int main(int argc,char *argv[])
{
  if (argc < 2)
    {
      fprintf (stderr,"Usage: %s infile\n",argv[0]);
      exit(1);
    }

  mr_file_reader *file_reader = new mr_file_reader;

  file_reader->open(argv[1]);

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
