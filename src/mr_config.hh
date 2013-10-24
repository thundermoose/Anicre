
#ifndef __MR_CONFIG_HH__
#define __MR_CONFIG_HH__

#define DUMP_FULL 1

struct mr_config_t
{
  int _dump;

  const char *_td_dir;

};

extern mr_config_t _config;

#endif/*__MR_CONFIG_HH__*/
