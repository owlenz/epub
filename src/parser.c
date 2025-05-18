#include "parser.h"
#include "xml.h"
#include <stdio.h>
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
        struct xml_string *xml_label_str = xml_node_content(label);
        long label_length = xml_string_length(xml_label_str);
        char *label_string = malloc(label_length + 1);
        label_string[label_length] = '\0';

        xml_string_copy(xml_label_str, label_string, label_length);
        epub->toc[epub->pos].entry = malloc(strlen(label_string) + 1);

        struct xml_string *xml_content_str = xml_node_attribute_content(content, 0);
        long content_length = xml_string_length(xml_content_str);
        char *content_string = malloc(content_length + 1);
        content_string[content_length] = '\0';

        xml_string_copy(xml_content_str, content_string, content_length);
        epub->toc[epub->pos].file = malloc(strlen(content_string) + 1);

        strcpy(epub->toc[epub->pos].entry,label_string);
        strcpy(epub->toc[epub->pos].file,content_string);

        epub->pos++;

        free(label_string);
        free(content_string);
        free(tag_name);
      }
    }
  }

  for (int i = 0; i < x; i++) {
    struct xml_node *child = xml_node_child(node, i);
    read_node(child, epub);
  }
}

void read_node_html(struct xml_node *node, struct chapter *chapter) {
  int x = xml_node_children(node);

  if (x==0) {
    struct xml_string *xml_tag = xml_node_name(node);
    long tag_length = xml_string_length(xml_tag);
    char tag_name[tag_length];
    xml_string_copy(xml_tag, tag_name, tag_length);
    tag_name[tag_length] = '\0';
    if (strcmp(tag_name, "title") == 0) {
      struct xml_string *xml_str = xml_node_content(node);
      long length = xml_string_length(xml_str);
      char string[length] ;
      xml_string_copy(xml_str, string, length);
      string[length] = '\0';

      chapter->title = malloc(strlen(string) + 1);
      strcpy(chapter->title,string);
    } else {
      struct xml_string *xml_str = xml_node_content(node);
      long length = xml_string_length(xml_str);
      char string[length] ;
      xml_string_copy(xml_str, string, length);
      string[length] = '\0';

      strncpy(&chapter->buffer[chapter->pos], string, length);
      chapter->buffer[chapter->pos + length] = '\n';
      chapter->buffer[chapter->pos + length + 1] = '\n';
      chapter->pos += length + 2;
    }
  }

  for (int i = 0; i < x; i++) {
    struct xml_node *child = xml_node_child(node, i);
    read_node_html(child, chapter);
  }
}

void *_parse_xml_buffer(char *buff, long buff_len, bool html) {
  struct xml_document *document = xml_parse_document(buff, buff_len);

  if (!document) {
    printf("Couldn't parse document\n");
    exit(EXIT_FAILURE);
  }

  struct xml_node *root = xml_document_root(document);
  if (html) {
    struct chapter *epub_chapter = malloc(sizeof(struct epub));

    epub_chapter->buffer = malloc(buff_len);
    epub_chapter->title = malloc(200 * sizeof(char));
    epub_chapter->pos = 0;
    if (epub_chapter->buffer == NULL)
      perror("cannot allocate memory");

    read_node_html(root, epub_chapter);
    if (document) {
      xml_document_free(document, 1);
      document = NULL;
    }
    return epub_chapter;
  } else {
    struct epub *epub_buffer = malloc(sizeof(struct epub));

    epub_buffer->buffer = malloc(buff_len);
    epub_buffer->toc = malloc(1500 * sizeof(struct toc));
    epub_buffer->buffer_size = buff_len;
    if (epub_buffer->buffer == NULL)
      perror("cannot allocate memory");

    epub_buffer->pos = 0;
    read_node(root, epub_buffer);

    if (document) {
      xml_document_free(document, 1);
      document = NULL;
    }
    return epub_buffer->toc;
  }
}

void *read_zip_file(char *name) {
  zip_stat_t *finfo = malloc(400 * sizeof(int));
  if (finfo == NULL)
    perror("error allocating memory");
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

  bool html = false;
  if (strstr(finfo->name, "xhtml")) {
    html = true;
  }
  char *buff = malloc(finfo->size);
  if (buff == NULL) {
    perror("buffer");
  }
  int bytes = zip_fread(zfile, buff, finfo->size);
  if (bytes < 0) {
    fprintf(stderr, "error reading file %s\n", finfo->name);
  }
  free(finfo);
  return _parse_xml_buffer(buff, strlen(buff), html);
}

void zip_init() {
  epub_zip.zip = zip_open("./sv.epub", ZIP_RDONLY, NULL);
  if (epub_zip.zip == NULL) {
    perror("cannot open epub file");
  }
  epub_zip.size = zip_get_num_entries(epub_zip.zip, 0);
}
