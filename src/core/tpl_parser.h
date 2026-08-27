#pragma once

#include "core/arena.h"
#include "core/defs.h"
#include "core/string.h"
#include <string.h>

// template var: {{ NAME }} -> value (substituted in content)
typedef struct {
  const char *name;
  const char *value;
} tpl_var_t;

// parsed template: one output file per block
typedef struct {
  char *path;
  char *content;
} tpl_file_t;

// --- internal helpers -------------------------------------------------------

static inline bool tpl_line_is_fence(const char *line, size_t len)
{
  while (len && (line[0] == ' ' || line[0] == '\t')) {
    line++;
    len--;
  }
  return len >= 3 && line[0] == '`' && line[1] == '`' && line[2] == '`';
}

static inline bool tpl_line_is_heading(const char *line, size_t len)
{
  while (len && (line[0] == ' ' || line[0] == '\t')) {
    line++;
    len--;
  }
  return len >= 2 && line[0] == '#' && (line[1] == ' ' || line[1] == '\t');
}

static inline char *tpl_heading_path(arena_t *arena, const char *line, size_t len)
{
  const char *s = line;
  size_t n = len;

  while (n && (s[0] == ' ' || s[0] == '\t')) {
    s++;
    n--;
  }
  s++; // skip '#'
  n--;
  while (n && (s[0] == ' ' || s[0] == '\t')) {
    s++;
    n--;
  }

  const char *end = s + n;
  while (end > s && (end[-1] == ' ' || end[-1] == '\t' || end[-1] == '\r')) {
    end--;
  }

  size_t path_len = (size_t)(end - s);
  char *path = arena_push(arena, char, path_len + 1);
  mem_copy((void *)s, path, path_len);
  path[path_len] = '\0';
  return path;
}

static inline const char *tpl_lookup(const char *token, size_t token_len,
                                     const tpl_var_t *vars, u32 var_count)
{
  const char *s = token;
  size_t n = token_len;

  while (n && (s[0] == ' ' || s[0] == '\t')) {
    s++;
    n--;
  }
  while (n && (s[n - 1] == ' ' || s[n - 1] == '\t')) {
    n--;
  }

  // sanitize: uppercase + null terminate into a temp buffer (case-insensitive)
  char tmp[64];
  if (n >= sizeof(tmp)) {
    return NULL;
  }
  for (size_t i = 0; i < n; i++) {
    char c = s[i];
    tmp[i] = (c >= 'a' && c <= 'z') ? (char)(c - 'a' + 'A') : c;
  }
  tmp[n] = '\0';

  for (u32 i = 0; i < var_count; i++) {
    if (str_eq(tmp, (char *)vars[i].name)) {
      return vars[i].value;
    }
  }
  return NULL;
}

// --- public -----------------------------------------------------------------

// parse a markdown template file into a list of (path, content) blocks
// format:
//   # <output path>
//   ```<lang> {{{
//   ...content...
//   ``` }}}
// returns number of parsed blocks
API int tpl_parse(arena_t *arena, const char *data, tpl_file_t **out_files)
{
  *out_files = NULL;

  // pass 1: count blocks (opening fences)
  int count = 0;
  {
    bool in_block = false;
    const char *p = data;
    while (*p) {
      const char *line = p;
      const char *eol = strchr(p, '\n');
      size_t len = eol ? (size_t)(eol - p) : strlen(p);
      const char *next = eol ? eol + 1 : p + len;

      if (tpl_line_is_fence(line, len)) {
        if (in_block) {
          in_block = false;
        } else {
          in_block = true;
          count++;
        }
      }
      p = next;
    }
  }

  tpl_file_t *files = arena_push(arena, tpl_file_t, count);
  memset(files, 0, count * sizeof(tpl_file_t));

  // pass 2: fill path + content
  {
    bool in_block = false;
    char *current_path = NULL;
    const char *block_start = NULL;
    int idx = 0;

    const char *p = data;
    while (*p) {
      const char *line = p;
      const char *eol = strchr(p, '\n');
      size_t len = eol ? (size_t)(eol - p) : strlen(p);
      const char *next = eol ? eol + 1 : p + len;

      if (in_block) {
        if (tpl_line_is_fence(line, len)) {
          size_t content_len = (size_t)(line - block_start);
          files[idx].path = current_path;
          files[idx].content = arena_push(arena, char, content_len + 1);
          mem_copy((void *)block_start, files[idx].content, content_len);
          files[idx].content[content_len] = '\0';
          idx++;
          in_block = false;
          current_path = NULL;
        }
      } else {
        if (tpl_line_is_fence(line, len)) {
          in_block = true;
          block_start = next;
        } else if (tpl_line_is_heading(line, len)) {
          current_path = tpl_heading_path(arena, line, len);
        }
      }
      p = next;
    }
  }

  *out_files = files;
  return count;
}

// substitute {{ VAR }} (spaces optional, case-insensitive) in content
API char *tpl_render(arena_t *arena, const char *content,
                     const tpl_var_t *vars, u32 var_count)
{
  // pass 1: compute output length
  size_t out_len = 0;
  {
    const char *p = content;
    while (*p) {
      if (p[0] == '{' && p[1] == '{' && p[2] != '{') {
        const char *close = strstr(p + 2, "}}");
        if (close) {
          size_t token_len = (size_t)(close - (p + 2));
          const char *value = tpl_lookup(p + 2, token_len, vars, var_count);
          out_len += value ? strlen(value) : (2 + token_len + 2);
          p = close + 2;
        } else {
          out_len += 1;
          p += 1;
        }
      } else {
        out_len += 1;
        p += 1;
      }
    }
  }
  out_len += 1; // null terminator

  char *out = arena_push(arena, char, out_len);

  // pass 2: copy + substitute
  {
    char *w = out;
    const char *p = content;
    while (*p) {
      if (p[0] == '{' && p[1] == '{' && p[2] != '{') {
        const char *close = strstr(p + 2, "}}");
        if (close) {
          size_t token_len = (size_t)(close - (p + 2));
          const char *value = tpl_lookup(p + 2, token_len, vars, var_count);
          if (value) {
            size_t vlen = strlen(value);
            mem_copy((void *)value, w, vlen);
            w += vlen;
          } else {
            size_t lit_len = 2 + token_len + 2;
            mem_copy((void *)p, w, lit_len);
            w += lit_len;
          }
          p = close + 2;
        } else {
          *w++ = *p++;
        }
      } else {
        *w++ = *p++;
      }
    }
    *w = '\0';
  }

  return out;
}
