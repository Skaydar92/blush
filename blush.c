#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define BLUSH_RL_BUFSIZE 1024

char *blush_read_line(void)
{
  int bufsize = BLUSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "blush: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    c = getchar();

    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if (position >= bufsize) {
      bufsize += BLUSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "blush: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define BLUSH_TOK_BUFSIZE 64
#define BLUSH_TOK_DELIM " \t\r\n\a"

char **blush_split_line(char *line)
{
  int bufsize = BLUSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char * token;

  if (!tokens) {
    fprintf(stderr, "blush: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, BLUSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += BLUSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "blush: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, BLUSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int blush_launch(char **args)
{
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("blush");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("blush");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int blush_cd(char **args);
int blush_help(char **args);
int blush_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &blush_cd,
  &blush_help,
  &blush_exit
};

int blush_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

int blush_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("blush");
    }
  }
  return 1;
}

int blush_help(char **args)
{
  int i;
  printf("BLUSH - Bash like User (S)HELL\n");
  printf("it should be able to execute binarys located in $PATH probably maybe if it feels like it\n");
  printf("Builtins available: \n\n");

  for (i = 0; i < blush_num_builtins(); i++) {
    printf(" %s\n", builtin_str[i]);
  }

  printf("\n maybe you can even use the 'man' command\n");
  return 1;
}

int blush_exit(char **args)
{
  return 0;
}


int blush_execute(char **args)
{
  int i;
  
  if (args[0] == NULL) {
    return 1;
  }

  for (i = 0; i < blush_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return blush_launch(args);
}

void blush_loop(void)
{
  char *line;
  char **args;
  int status;

  do {
    printf("$USER@$HOSTNAME > ");
    line = blush_read_line();
    args = blush_split_line(line);
    status = blush_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv)
{
  blush_loop();

  return EXIT_SUCCESS;
}
