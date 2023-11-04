#include "config.h"

#include <stdio.h>
#include <stdlib.h>

int parse_config(const char* config_str) {
  int config = 0;
  char config_c = config_str[0];
  for (int i = 1; config_c; i++) {
    switch (config_c) {
      case 'g':
        config |= OPT_GRAPH;
        break;
      case 'r':
        config |= OTP_REAL_TIME;
        break;
      default:
        fprintf(stderr, "Unknown config parameter '%c'\n", config_c);
        exit(1);
    }
    config_c = config_str[i];
  }
  return config;
}
