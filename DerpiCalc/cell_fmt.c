#include <string.h>

#include "cell_fmt.h"
#include "cell.h"
#include "util.h"

void c_format_value(uint8_t* s, uint8_t len, uint8_t format)
{
    uint8_t i = 0;
    uint8_t diff = 0;
    uint8_t overflow = 0;

    switch (format)
    {
        case CELL_FORMAT_DOLLARS:
            // Find any existing decimal
            for (i = 0;(i < len) && (s[i] != SYMBOL_FULL_STOP);++i);

            if (i < len) {
                if (i <= (len - 3)) {
                    // If there's three or more digits after decimal
                    //   Truncate
                    // If there's one digit after decimal
                    //   Add zero
                    ++i; // Skip decimal
                    ++i; // Skip first digit
                    if (s[i] == SYMBOL_SPACE) {
                        s[i] = SYMBOL_DIGIT_ZERO;
                    }
                    ++i;
                    for (;i < len;++i) {
                        s[i] = SYMBOL_SPACE;
                    }
                } else {
                    overflow = 1;
                    break;
                }
            }
            else {
                // If there's no decimal then add .00
                //   If that won't fit then make it >>>
                // Back up until non-blank
                i = len - 1;
                do {
                    if (s[i] != SYMBOL_SPACE) {
                        ++i;
                        break;
                    }
                    --i;
                } while (i > 0);
                if (i <= (len - 3)) {
                    s[i] = SYMBOL_FULL_STOP;
                    ++i;
                    s[i] = SYMBOL_DIGIT_ZERO;
                    ++i;
                    s[i] = SYMBOL_DIGIT_ZERO;
                    ++i;
                } else {
                    overflow = 1;
                    break;
                }
            }
            break;

        case CELL_FORMAT_RIGHT:
            i = len - 1;
            do {
                if (s[i] != SYMBOL_SPACE)
                    break;
                --i;
            } while (i > 0);
            if (s[i] == SYMBOL_SPACE)
                break;
            diff = (len - i) - 1;

            memmove(s + diff, s, i + 1);

            for (i = 0;i < diff;++i)
                s[i] = SYMBOL_SPACE;
            break;
        default:
            break;
    }

    if (overflow) {
        for (i = 0;i < len;++i) {
            s[i] = SYMBOL_GREATER_THAN_SIGN;
        }
    }
}
