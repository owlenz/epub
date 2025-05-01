#include "xml.c/src/xml.h"
#include <stdlib.h>
#include <zip.h>
#include <zipconf.h>
#include <zlib.h>
#include <lexbor/html/parser.h>
#include "parser.h"

#define FILE_SIZE(FILE)                                                        \
  ({                                                                           \
    zip_fseek(FILE, 0, SEEK_END);                                              \
    int size = zip_ftell(FILE);                                                \
    zip_fseek(FILE, 0, SEEK_SET);                                              \
    size;                                                                      \
  })

struct epub {
  char *buffer;
  int pos;
};

void read_node(struct xml_node *node, struct epub *epub) {
  
  int x = xml_node_children(node);

  if (x == 0) {
    struct xml_string *xml_str = xml_node_content(node);
    long length = xml_string_length(xml_str);
    char *string = malloc(length + 1);
    string[length] = '\0';
    xml_string_copy(xml_str, string, length);
    strcpy(&epub->buffer[epub->pos], string);
    epub->pos+=length;
    printf("%s\n", epub->buffer);
    free(string);
    return;
  }
  for (int i = 0; i < x; i++) {
    struct xml_node *child = xml_node_child(node, i);
    read_node(child,epub);
  }
}

char * parse_main() {
  int test;
  zip_t *zip = zip_open("./test.epub", ZIP_RDONLY, NULL);
  int zip_size = zip_get_num_entries(zip, 0);

  zip_stat_t *finfo = malloc(400 * sizeof(int));
  zip_stat_init(finfo);

  zip_stat(zip, "OEBPS/Text/chapter-002.xhtml", 0, finfo);
  
  zip_file_t *zfile = zip_fopen(zip, finfo->name, 0);
  if (zfile == NULL) {
    fprintf(stderr, "Failed to open file in ZIP archive\n");
    zip_close(zip);
    return "balls";
  }
  printf("file:%s size: %ld\n", finfo->name, finfo->size);
  
  char *buff = malloc(finfo->size);
  if (buff == NULL) {
    perror("buffer");
  }
  int bytes = zip_fread(zfile, buff, finfo->size);
  if (bytes < 0) {
    fprintf(stderr, "error reading file %s\n", finfo->name);
  }

  /* printf("%s\n", buff); */

  char *buffer = malloc(100);
  FILE *fptr = fopen("./OEBPS/Text/chapter-001.xhtml", "rw");
  fread(buffer, sizeof(char), 100, fptr);
  /* printf("%s\n",buffer); */

  uint8_t *source = "<?xml version=\"1.0\"?><html class=\"ballsxddmors\" xmln=\"xddmors\"><balls>xddmors</balls></html>";
  struct xml_document *document = xml_parse_document(buff, strlen(buff));

  if (!document) {
    printf("Couldn't parse document\n");
    exit(EXIT_FAILURE);
  }

  struct xml_node *root = xml_document_root(document);
  struct epub *epub_buffer;
  epub_buffer->buffer = calloc(strlen(buff),1);
  epub_buffer->pos = 0;
  read_node(root, epub_buffer);

  return epub_buffer->buffer;
  /* xml_document_free(document, 1); */
}
