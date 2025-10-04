#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static int is_alnum(char c) {
  if (c >= '0' && c <= '9') return 1;
  if (c >= 'A' && c <= 'Z') return 1;
  if (c >= 'a' && c <= 'z') return 1;
  return 0;
}

static int my_strcmp(const char *a, const char *b) {
  while (*a && *b && *a == *b) { a++; b++; }
  return (uchar)*a - (uchar)*b;
}

static int is_blacklisted(const char *s) {
  const char *blacklist[] = { "0123456789ABCDEF", "mhpmcounters", "This", "" };
  int i = 0;
  while (blacklist[i][0] != '\0') {
    if (my_strcmp(s, blacklist[i]) == 0) return 1;
    i++;
  }
  return 0;
}

static void print_and_exit(char *buf) {
  printf("%s\n", buf);
  exit(0);
}

int main(int argc, char *argv[]) {
  const int PAGE = 4096;
  const int PH1_PAGES = 128;
  const int FIRST_MIN_LEN = 6;
  const int MAX_PAGES = 2048;
  const int MIN_LEN = 4;
  int i, j, k;

  for (i = 0; i < PH1_PAGES; i++) {
    char *p = sbrk(PAGE);
    if (p == (char*)-1) break;

    int cur_start = -1;
    int cur_len = 0;
    for (j = 0; j < PAGE; j++) {
      char c = p[j];
      if (is_alnum(c)) {
        if (cur_start == -1) cur_start = j;
        cur_len++;
      } else {
        if (cur_len >= FIRST_MIN_LEN) {
          int len = cur_len;
          if (len > 1024) len = 1024;
          char tmp[1025];
          for (k = 0; k < len; k++) tmp[k] = p[cur_start + k];
          tmp[len] = '\0';
          if (!is_blacklisted(tmp)) print_and_exit(tmp);
        }
        cur_start = -1;
        cur_len = 0;
      }
    }
    if (cur_len >= FIRST_MIN_LEN) {
      int len = cur_len;
      if (len > 1024) len = 1024;
      char tmp[1025];
      for (k = 0; k < len; k++) tmp[k] = p[cur_start + k];
      tmp[len] = '\0';
      if (!is_blacklisted(tmp)) print_and_exit(tmp);
    }
  }

  char *base = sbrk(0);
  if (base == (char*)-1) exit(1);

  int pages_alloc = 0;
  for (i = 0; i < MAX_PAGES; i++) {
    char *p = sbrk(PAGE);
    if (p == (char*)-1) break;
    pages_alloc++;
  }
  if (pages_alloc == 0) exit(1);

  int total = pages_alloc * PAGE;
  int best_start = -1;
  int best_len = 0;
  int cur_start = -1;
  int cur_len = 0;

  for (i = 0; i < total; i++) {
    char c = base[i];
    if (is_alnum(c)) {
      if (cur_start == -1) cur_start = i;
      cur_len++;
    } else {
      if (cur_len >= MIN_LEN) {
        int len = cur_len;
        if (len > 1024) len = 1024;
        char tmp[1025];
        for (k = 0; k < len; k++) tmp[k] = base[cur_start + k];
        tmp[len] = '\0';
        if (!is_blacklisted(tmp) && cur_len > best_len) {
          best_len = cur_len;
          best_start = cur_start;
        }
      }
      cur_start = -1;
      cur_len = 0;
    }
  }
  if (cur_len >= MIN_LEN) {
    int len = cur_len;
    if (len > 1024) len = 1024;
    char tmp[1025];
    for (k = 0; k < len; k++) tmp[k] = base[cur_start + k];
    tmp[len] = '\0';
    if (!is_blacklisted(tmp) && cur_len > best_len) {
      best_len = cur_len;
      best_start = cur_start;
    }
  }

  if (best_len <= 0) exit(1);

  int to_print = best_len;
  if (to_print > 2048) to_print = 2048;
  char outbuf[2050];
  for (i = 0; i < to_print; i++) outbuf[i] = base[best_start + i];
  outbuf[to_print] = '\0';
  printf("%s\n", outbuf);
  exit(0);
}

