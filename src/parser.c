#include "parser.h"
#include "xml.h"
#include <stdlib.h>
#include <string.h>
#include <zip.h>
#include <zipconf.h>
#include <zlib.h>

#define FILE_SIZE(FILE)                                                        \
  ({                                                                           \
    zip_fseek(FILE, 0, SEEK_END);                                              \
    int size = zip_ftell(FILE);                                                \
    zip_fseek(FILE, 0, SEEK_SET);                                              \
    size;                                                                      \
  })

typedef struct _zip_t {
  zip_t *zip;
  long size;
} pupby_zip;

pupby_zip epub_zip;

void read_node(struct xml_node *node, struct epub *epub) {

  int x = xml_node_children(node);

  struct xml_node *_navLabel = xml_node_child(node, 0);
  if (_navLabel) {
    struct xml_node *label = xml_node_child(_navLabel, 0);
    struct xml_node *content = xml_node_child(node, 1);

    if (label && content) {
      struct xml_string *xml_tag = xml_node_name(content);
      long tag_length = xml_string_length(xml_tag);
      char *tag_name = malloc(tag_length);
      xml_string_copy(xml_tag, tag_name, tag_length);

      if (strcmp(tag_name, "content") == 0) {
        struct xml_string *xml_str = xml_node_content(label);
        long length = xml_string_length(xml_str);
        char *string = malloc(length + 1);
        string[length] = '\0';

        xml_string_copy(xml_str, string, length);

        epub->toc[epub->pos] = malloc(strlen(string) + 1);
        strcpy(epub->toc[epub->pos],string);

        epub->pos++;

        free(string);
        free(tag_name);
      }
    }
  }

  for (int i = 0; i < x; i++) {
    struct xml_node *child = xml_node_child(node, i);

    read_node(child, epub);
  }
}

char **_parse_xml_buffer(char *buff, long buff_len) {
  /* printf("%d",buff_len); */
  struct xml_document *document = xml_parse_document(buff, buff_len);

  if (!document) {
    printf("Couldn't parse document\n");
    exit(EXIT_FAILURE);
  }

  struct xml_node *root = xml_document_root(document);
  struct epub *epub_buffer = malloc(sizeof(struct epub));

  epub_buffer->buffer = malloc(buff_len);
  epub_buffer->toc = malloc(1500 * sizeof(char*));
  epub_buffer->buffer_size = buff_len;
  if (epub_buffer->buffer == NULL)
    perror("cannot allocate memory");

  epub_buffer->pos = 0;
  read_node(root, epub_buffer);

  xml_document_free(document, 1);
  return epub_buffer->toc;
}

char **read_zip_file(char *name) {
  zip_stat_t *finfo = malloc(400 * sizeof(int));
  zip_stat_init(finfo);

  int err = zip_stat(epub_zip.zip, name, 0, finfo);
  if (err == -1) {
    fprintf(stderr, "[ERROR] %d: file %s not found", -1, name);
    return NULL;
  }

  zip_file_t *zfile = zip_fopen(epub_zip.zip, finfo->name, 0);
  if (zfile == NULL) {
    fprintf(stderr, "Failed to open file in ZIP archive\n");
    zip_close(epub_zip.zip);
    return NULL;
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
  return _parse_xml_buffer(buff, strlen(buff));
}

void zip_init() {
  epub_zip.zip = zip_open("./sv.epub", ZIP_RDONLY, NULL);
  if (epub_zip.zip == NULL) {
    perror("cannot open epub file");
  }
  epub_zip.size = zip_get_num_entries(epub_zip.zip, 0);
}
