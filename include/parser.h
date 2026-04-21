#ifndef PARSER_H
#define PARSER_H
#include "xml.h"
#include "zip.h"
#include <stdint.h>
#include <sys/stat.h>

struct pubby_epub {
  struct toc *toc;
  char *buffer;
  int pos;
  int buffer_size;
};

struct toc {
  char *entry;
  uint8_t *file;
};

typedef struct html_tag {
  char *tag_name;
  char *content;
  struct html_tag **children;
  int n_children;
  char *css;
} html_tag;

struct chapter {
  char *buffer;
  char *html;
  html_tag *tags;
  int n_tags;
  char *title;
  int pos;
};

typedef struct _zip_t {
  zip_t *zip;
  uint8_t *root;
  long size;
} pubby_zip;

extern pubby_zip *epub_zip;

typedef struct {
  uint8_t *buff;
  size_t buff_len;
} epub_string;

void read_node(struct xml_node *node, struct pubby_epub *epub);
void zip_init(const char *path);
epub_string *read_zip_file(uint8_t *);
struct toc *read_toc();
struct chapter *read_html(uint8_t *);

#endif
