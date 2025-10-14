// the only difference between s_elseutf8.c and original s_utf8.c are theese 3 methods,
// so we leave only them, the rest will be provided by libpd

#include <s_elseutf8.h>

/* moo: get byte length of character number, or 0 if not supported */
int else_u8_wc_nbytes(uint32_t ch)
{
  if (ch < 0x80) return 1;
  if (ch < 0x800) return 2;
  if (ch < 0x10000) return 3;
  if (ch < 0x200000) return 4;
  return 0; /*-- bad input --*/
}

void else_u8_inc(const char *s, int *i)
{
    if (s[(*i)++] & 0x80) {
        if (!isutf(s[*i])) {
            ++(*i);
            if (!isutf(s[*i])) {
                ++(*i);
                if (!isutf(s[*i])) {
                    ++(*i);
                }
            }
        }
    }
}

void else_u8_dec(const char *s, int *i)
{
    (void)(isutf(s[--(*i)]) || isutf(s[--(*i)]) ||
           isutf(s[--(*i)]) || --(*i));
}
