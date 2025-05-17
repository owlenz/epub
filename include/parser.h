#ifndef PARSER_H
#define PARSER_H
#include "xml.h"

struct epub {
  char **toc;
  char *buffer;
  int pos;
  int buffer_size;
};

void read_node(struct xml_node *node, struct epub *epub);
void zip_init();
char **read_zip_file(char *);

#endif
