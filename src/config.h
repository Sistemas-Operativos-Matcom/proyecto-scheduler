#ifndef CONFIG_H
#define CONFIG_H

enum {
  OPT_GRAPH = 0x1,
  OTP_REAL_TIME = 0x2,
};

int parse_config(const char* config_str);

#endif
