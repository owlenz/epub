#ifndef PARSER_H
#define PARSER_H
#include "xml.h"

struct epub {
  struct toc *toc;
  char *buffer;
  int pos;
  int buffer_size;
};

struct toc {
  char *entry;
  char *file;
};

struct chapter {
  char *buffer;
  char *title;
  int pos;
};

void read_node(struct xml_node *node, struct epub *epub);
void zip_init();
void *read_zip_file(char *);

#endif
