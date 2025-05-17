#include "window.h"

int main(int argc, char **argv){
  int status;
  AdwApplication *app = window_init();
  status = g_application_run(G_APPLICATION(app), argc, argv);
  return status;
}
