#include "parser.h"
#include "xml.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE(FILE)                                                        \
  ({                                                                           \
    zip_fseek(FILE, 0, SEEK_END);                                              \
    int size = zip_ftell(FILE);                                                \
    zip_fseek(FILE, 0, SEEK_SET);                                              \
    size;                                                                      \
  })

pubby_zip *epub_zip = NULL;


void *_parse_xml_buffer(char *buff, long buff_len) {
  struct xml_document *document = xml_parse_document(buff, buff_len);

  if (!document) {
    printf("Couldn't parse document\n");
    exit(EXIT_FAILURE);
  }

  struct xml_node *root = xml_document_root(document);
  struct pubby_epub *epub_buffer = malloc(sizeof(struct pubby_epub));

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

struct xml_node *_parse_xml_buffer2(struct xml_document* document) {

  if (!document) {
    printf("Couldn't parse document\n");
    exit(EXIT_FAILURE);
  }

  struct xml_node *root = xml_document_root(document);

  return root;
}

void read_node(struct xml_node *node, struct pubby_epub *epub) {
  int x = xml_node_children(node);

  struct xml_node *_navLabel = xml_node_child(node, 0);
  if (_navLabel) {
    struct xml_node *label = xml_node_child(_navLabel, 0);
    struct xml_node *content = xml_node_child(node, 1);

    if (label && content) {
      struct xml_string *xml_tag = xml_node_name(content);
      long tag_length = xml_string_length(xml_tag);
      uint8_t *tag_name = malloc(tag_length);
      xml_string_copy(xml_tag, tag_name, tag_length);

      if (strcmp((const char *)tag_name, "content") == 0) {
        struct xml_string *xml_label_str = xml_node_content(label);
        long label_length = xml_string_length(xml_label_str);
        char *label_string = malloc(label_length + 1);
        label_string[label_length] = '\0';

        xml_string_copy(xml_label_str, label_string, label_length);
        epub->toc[epub->pos].entry = malloc(strlen(label_string) + 1);

        struct xml_string *xml_content_str =
            xml_node_attribute_content(content, 0);
        long content_length = xml_string_length(xml_content_str);
        char *content_string = malloc(content_length + 1);
        content_string[content_length] = '\0';

        xml_string_copy(xml_content_str, content_string, content_length);
        epub->toc[epub->pos].file = malloc(strlen(content_string) + 1);

        strcpy(epub->toc[epub->pos].entry, label_string);
        strcpy(epub->toc[epub->pos].file, content_string);

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

uint8_t *_read_container(struct xml_node* node) {
  size_t child_num = xml_node_children(node);
  struct xml_node *rootfile = xml_easy_child(node, "rootfiles", "rootfile");
  if (rootfile) {
    size_t attr_num = xml_node_attributes(node);
    if (attr_num == 0)
      perror("invalid container file");
    // search in all attrs for full-path attr
    for (int i = 0; i < attr_num; i++) {
      struct xml_string *xml_attr_name = xml_node_attribute_name(rootfile, i);
      long length = xml_string_length(xml_attr_name);
      char attr_name[length];
      xml_string_copy(xml_attr_name, attr_name, length);
      attr_name[length] = '\0';

      if (strcmp(attr_name, "full-path") == 0) {
        struct xml_string *xml_attr_value = xml_node_attribute_content(rootfile, i);
        long length = xml_string_length(xml_attr_value);
        char *attr_value = malloc(length);
        xml_string_copy(xml_attr_value, attr_value, length);
        attr_value[length] = '\0';
        return attr_value;
      }
    }
  }
  /* for (int i = 0; i < child_num; i++) { */
  /*   struct xml_node *child = xml_node_child(node, i); */
  /*   _read_container(child); */
  /* } */
}

uint8_t *read_container(uint8_t *buff, size_t buff_len) {
  struct xml_document *document = xml_parse_document(buff, buff_len);
  struct xml_node *node = _parse_xml_buffer2(document);
  uint8_t *balls = _read_container(node);
  if (document) {
    xml_document_free(document, 1);
    document = NULL;
  }
  return balls;
}


void read_node_html(struct xml_node *node, struct chapter *chapter) {
  int x = xml_node_children(node);

  if (x == 0) {
    struct xml_string *xml_tag = xml_node_name(node);
    long tag_length = xml_string_length(xml_tag);
    char tag_name[tag_length];
    xml_string_copy(xml_tag, tag_name, tag_length);

    tag_name[tag_length] = '\0';
    if (strcmp(tag_name, "title") == 0) {
      struct xml_string *xml_str = xml_node_content(node);
      long length = xml_string_length(xml_str);
      char string[length];
      xml_string_copy(xml_str, string, length);
      string[length] = '\0';

      chapter->title = malloc(strlen(string) + 1);
      strcpy(chapter->title, string);
    } else {
      struct xml_string *xml_str = xml_node_content(node);
      long length = xml_string_length(xml_str);
      char string[length];
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


void *_parse_html_buffer(char *buff, long buff_len) {
  struct xml_document *document = xml_parse_document(buff, buff_len);

  if (!document) {
    printf("Couldn't parse document\n");
    exit(EXIT_FAILURE);
  }

  struct xml_node *root = xml_document_root(document);
  struct chapter *epub_chapter = malloc(sizeof(struct pubby_epub));

  epub_chapter->buffer = malloc(buff_len);
  epub_chapter->title = malloc(200 * sizeof(char));
  epub_chapter->pos = 0;
  if (epub_chapter->buffer == NULL)
    perror("cannot allocate memory");

  read_node_html(root, epub_chapter);
  epub_chapter->buffer[epub_chapter->pos - 1] = '\0';
  if (document) {
    xml_document_free(document, 1);
    document = NULL;
  }
  return epub_chapter;
}

epub_string *read_zip_file(char *name) {
  zip_stat_t *finfo = malloc(400 * sizeof(int));
  if (finfo == NULL)
    perror("error allocating memory");
  zip_stat_init(finfo);

  /* uint8_t* specified_name */
  int err = zip_stat(epub_zip->zip, name, 0, finfo);
  if (err == -1) {
    fprintf(stderr, "[ERROR] %d: file %s not found", -1, name);
    return NULL;
  }

  zip_file_t *zfile = zip_fopen(epub_zip->zip, finfo->name, 0);
  if (zfile == NULL) {
    fprintf(stderr, "Failed to open file in ZIP archive\n");
    zip_close(epub_zip->zip);
    return NULL;
  }

  uint8_t *buff = malloc(finfo->size);
  if (buff == NULL) {
    perror("cannot allocate file content buffer");
  }
  int bytes = zip_fread(zfile, buff, finfo->size);
  if (bytes < 0) {
    fprintf(stderr, "error reading file %s\n", finfo->name);
  }
  free(finfo);

  epub_string *buff_d = malloc(sizeof(epub_string));
  buff_d->buff = buff;
  buff_d->buff_len = strlen((const char *)buff);
  return buff_d;
}

struct toc *read_toc(char *toc_str) {
  epub_string* buff = read_zip_file(toc_str);
  
  struct toc * toc= _parse_xml_buffer(buff->buff,buff->buff_len);
  free(buff);
  return toc;
}

void zip_init() {
  epub_zip = malloc(sizeof(pubby_zip));
  epub_zip->zip = zip_open("./sv.epub", ZIP_RDONLY, NULL);
  if (epub_zip->zip == NULL) {
    perror("cannot open epub file");
  }
  epub_zip->size = zip_get_num_entries(epub_zip->zip, 0);

  // read container.xml
  epub_string *buff = read_zip_file("META-INF/container.xml");
  uint8_t *full_path = read_container(buff->buff, buff->buff_len);
  uint8_t *root_path = malloc(6);
  if (strstr(full_path, "OEBPS"))
    epub_zip->root = "OEBPS/";

  free(buff);
  free(full_path);
  free(root_path);
}
