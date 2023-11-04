#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <json_object.h>
#include <json_tokener.h>

int main(int argc, char **argv) {
  char buf[1024];
  const char *layout = NULL;

  json_tokener *tok = json_tokener_new();
  enum json_tokener_error jerr;

  {
    json_object *jobj = NULL;

    int fds[2];
    if (pipe(fds) == -1) {
      return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
      return 1;
    } if (pid == 0) {
      while ((dup2(fds[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
      close(fds[1]);
      close(fds[0]);
      execl(
        "/usr/bin/swaymsg",
        "/usr/bin/swaymsg",
        "-t",
        "get_inputs",
        NULL);
      _exit(1);
    }
    close(fds[1]);

    ssize_t len = 0;
    size_t end = 0;
    do {
      const size_t res = len - end;
      if (res) {
        memmove(buf, buf + end, res);
        len = read(fds[0], &buf[res], sizeof(buf) - res);
      } else {
        len = read(fds[0], buf, sizeof(buf));
      }

      if (len == -1) {
        return 1;
      }

      len += res;

      jobj = json_tokener_parse_ex(tok, buf, len + res);
      end = json_tokener_get_parse_end(tok);
    } while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue);
    close(fds[0]);

    if (jerr != json_tokener_success) {
      return 1;
    }

    for (int i = 0; i < json_object_array_length(jobj); i++) {
      struct json_object *input = json_object_array_get_idx(jobj, i);
      if (input == NULL) {
        continue;
      }

      struct json_object *xkb_active_layout_name;
      if (!json_object_object_get_ex(input, "xkb_active_layout_name", &xkb_active_layout_name)) {
        continue;
      }

      const char *xkb_active_layout_name_str = json_object_get_string(xkb_active_layout_name);
      if (xkb_active_layout_name_str == NULL) {
        continue;
      }

      layout = strdup(xkb_active_layout_name_str);
      printf("{\"text\": \"%s\"}\n", layout);
      fflush(stdout);
      break;
    }

    json_object_put(jobj);
  }

  json_tokener_reset(tok);

  {
    json_object *jobj = NULL;

    int fds[2];
    if (pipe(fds) == -1) {
      return 1;
    }

    pid_t pid = fork();
    if (pid == -1) {
      return 1;
    } else if (pid == 0) {
      while ((dup2(fds[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
      close(fds[1]);
      close(fds[0]);
      execl(
        "/usr/bin/swaymsg",
        "/usr/bin/swaymsg",
        "-mt",
        "subscribe",
        "[\"input\"]",
        NULL);
      _exit(1);
    }
    close(fds[1]);

    ssize_t len = 0;
    size_t end = 0;
    while (1) {
      const size_t res = len - end;
      if (res) {
        memmove(buf, buf + end, res);
        len = read(fds[0], &buf[res], sizeof(buf) - res);
      } else {
        len = read(fds[0], buf, sizeof(buf));
      }

      if (len == -1) {
        return 1;
      }

      len += res;

      jobj = json_tokener_parse_ex(tok, buf, len);
      end = json_tokener_get_parse_end(tok);

      if (json_tokener_get_error(tok) == json_tokener_continue) {
        continue;
      }

      json_tokener_reset(tok);

      if (jobj == NULL) {
        continue;
      }

      struct json_object *input = NULL;
      if (!json_object_object_get_ex(jobj, "input", &input)) {
        continue;
      }

      struct json_object *xkb_active_layout_name;
      if (!json_object_object_get_ex(input, "xkb_active_layout_name", &xkb_active_layout_name)) {
        continue;
      }

      const char *xkb_active_layout_name_str = json_object_get_string(xkb_active_layout_name);
      if (xkb_active_layout_name_str == NULL) {
        continue;
      }

      if (strcmp(xkb_active_layout_name_str, layout) != 0) {
        free((void *)layout);
        layout = strdup(xkb_active_layout_name_str);
        printf("{\"text\": \"%s\"}\n", layout);
        fflush(stdout);
      }

      json_object_put(jobj);
    }
  }
}
