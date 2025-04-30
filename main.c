#include <zip.h>
#include <zipconf.h>
#include <stdlib.h>

#define FILE_SIZE(FILE)({                                               \
  zip_fseek(FILE, 0, SEEK_END);                                         \
  int size = zip_ftell(FILE);                                           \
  zip_fseek(FILE, 0, SEEK_SET);                                         \
  size;                                                                 \
    })

int main()
{
  int test;
  zip_t * zip = zip_open("./test.epub", ZIP_RDONLY, NULL);
  int num = zip_get_num_entries(zip, 0);
  /* for(int i = 0; i < num; i++) */
  /* { */
  /*   Const char * name = zip_get_name(zip, i, 0); */
  /*   printf("%s\n",name); */
  /* } */
  const char *file_name = zip_get_name(zip, 0, 0);
  zip_file_t *zfile = zip_fopen(zip, file_name, ZIP_RDONLY);
  if (zfile == NULL) {
    fprintf(stderr, "Failed to open file in ZIP archive\n");
    zip_close(zip);
    return 1;
  }
  long size = FILE_SIZE(zfile);
  char *buff = malloc(size);
  printf("%ld\n",size);
  
  int bytes = zip_fread(zfile,buff,size);

  if(bytes < 0)
  {
    perror("balls");
  }
  printf("file %s: %s",file_name,buff);
  free(buff);

  

}
