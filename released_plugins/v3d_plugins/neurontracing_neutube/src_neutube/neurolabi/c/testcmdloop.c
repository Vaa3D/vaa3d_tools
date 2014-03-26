#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Cmd_Entry {
  char *cmd;
  void (*execute) ();
} Cmd_Entry;

void quit()
{
  exit(1);
}

void greet()
{
  printf("Hello!\n");
}

void explore()
{
  printf("Nothing interesting here.\n");
}

void getcmd(char *cmd)
{
  while ((*cmd++ = getchar()) != '\n');
  cmd--;
  *cmd = '\0';
}

Cmd_Entry Cmd_Map[] = {
  {"quit", quit},
  {"hello", greet},
  {"look", explore}
};

void run()
{
 int ncmd = sizeof(Cmd_Map) / sizeof(Cmd_Entry);
  int i;
  char cmd[100];

  while (1) {
    getcmd(cmd);
    for (i = 0; i < ncmd; i++) {    
      if (strcmp(cmd, Cmd_Map[i].cmd) == 0) {
        Cmd_Map[i].execute();
        break;
      }
    }
    if (i >= ncmd) {
      printf("Unknown command.\n");
    }
  }
}

int main(int argc, char *argv[])
{
  run();
 
  return 1;
}
