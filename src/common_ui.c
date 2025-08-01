#define _XOPEN_SOURCE 700
#include <locale.h>
#include <string.h>
#include <stdio.h>
#include <wchar.h>
#include "term.h"
#include "common.h"
#include "common_ui.h"
#include "utils.h"

/*

common_ui.c

 UI functions.

*/

#define MIN_CHANNEL 50 // Minimum color red green blue

unsigned int updateCounter = 0;

// Name scrolling
bool finishedScrolling = false;
int lastNamePosition = 0;
bool isLongName = false;
int scrollDelaySkippedCount = 0;
bool isSameNameAsLastTime = false;
const int startScrollingDelay = 10; // Delay before beginning to scroll
const int scrollingInterval = 1;    // Interval between scrolling updates

void setTextColorRGB2(int r, int g, int b, const UISettings *ui)
{
        if (!ui->useConfigColors)
                setTextColorRGB(r, g, b);
}

void setColor(UISettings *ui)
{
        setColorAndWeight(0, ui->color, ui->useConfigColors);
}

void setColorAndWeight(int bold, PixelData color, int useConfigColors)
{
        if (useConfigColors)
        {
                printf("\033[%dm", bold);
                return;
        }

        if (color.r == defaultColor && color.g == defaultColor && color.b == defaultColor)
        {
                printf("\033[%dm", bold);
        }
        else if (color.r >= 210 && color.g >= 210 && color.b >= 210)
        {
                printf("\033[%d;38;2;%03u;%03u;%03um", bold, defaultColor, defaultColor, defaultColor);
        }
        else
        {
                printf("\033[%d;38;2;%03u;%03u;%03um", bold, color.r, color.g, color.b);
        }
}

void resetNameScroll()
{
        lastNamePosition = 0;
        isLongName = false;
        finishedScrolling = false;
        scrollDelaySkippedCount = 0;
}

/*
 * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

struct interval
{
        int first;
        int last;
};

/* auxiliary function for binary search in interval table */
static int bisearch(wchar_t ucs, const struct interval *table, int max)
{
        int min = 0;

        if (ucs < table[0].first || ucs > table[max].last)
                return 0;
        while (max >= min)
        {
                int mid = (min + max) / 2;
                if (ucs > table[mid].last)
                        min = mid + 1;
                else if (ucs < table[mid].first)
                        max = mid - 1;
                else
                        return 1;
        }

        return 0;
}

int mk_wcwidth(wchar_t ucs)
{
        /* sorted list of non-overlapping intervals of non-spacing characters */
        /* generated by "uniset +cat=Me +cat=Mn +cat=Cf -00AD +1160-11FF +200B c" */
        static const struct interval combining[] = {
            {0x0300, 0x036F}, {0x0483, 0x0486}, {0x0488, 0x0489}, {0x0591, 0x05BD}, {0x05BF, 0x05BF}, {0x05C1, 0x05C2}, {0x05C4, 0x05C5}, {0x05C7, 0x05C7}, {0x0600, 0x0603}, {0x0610, 0x0615}, {0x064B, 0x065E}, {0x0670, 0x0670}, {0x06D6, 0x06E4}, {0x06E7, 0x06E8}, {0x06EA, 0x06ED}, {0x070F, 0x070F}, {0x0711, 0x0711}, {0x0730, 0x074A}, {0x07A6, 0x07B0}, {0x07EB, 0x07F3}, {0x0901, 0x0902}, {0x093C, 0x093C}, {0x0941, 0x0948}, {0x094D, 0x094D}, {0x0951, 0x0954}, {0x0962, 0x0963}, {0x0981, 0x0981}, {0x09BC, 0x09BC}, {0x09C1, 0x09C4}, {0x09CD, 0x09CD}, {0x09E2, 0x09E3}, {0x0A01, 0x0A02}, {0x0A3C, 0x0A3C}, {0x0A41, 0x0A42}, {0x0A47, 0x0A48}, {0x0A4B, 0x0A4D}, {0x0A70, 0x0A71}, {0x0A81, 0x0A82}, {0x0ABC, 0x0ABC}, {0x0AC1, 0x0AC5}, {0x0AC7, 0x0AC8}, {0x0ACD, 0x0ACD}, {0x0AE2, 0x0AE3}, {0x0B01, 0x0B01}, {0x0B3C, 0x0B3C}, {0x0B3F, 0x0B3F}, {0x0B41, 0x0B43}, {0x0B4D, 0x0B4D}, {0x0B56, 0x0B56}, {0x0B82, 0x0B82}, {0x0BC0, 0x0BC0}, {0x0BCD, 0x0BCD}, {0x0C3E, 0x0C40}, {0x0C46, 0x0C48}, {0x0C4A, 0x0C4D}, {0x0C55, 0x0C56}, {0x0CBC, 0x0CBC}, {0x0CBF, 0x0CBF}, {0x0CC6, 0x0CC6}, {0x0CCC, 0x0CCD}, {0x0CE2, 0x0CE3}, {0x0D41, 0x0D43}, {0x0D4D, 0x0D4D}, {0x0DCA, 0x0DCA}, {0x0DD2, 0x0DD4}, {0x0DD6, 0x0DD6}, {0x0E31, 0x0E31}, {0x0E34, 0x0E3A}, {0x0E47, 0x0E4E}, {0x0EB1, 0x0EB1}, {0x0EB4, 0x0EB9}, {0x0EBB, 0x0EBC}, {0x0EC8, 0x0ECD}, {0x0F18, 0x0F19}, {0x0F35, 0x0F35}, {0x0F37, 0x0F37}, {0x0F39, 0x0F39}, {0x0F71, 0x0F7E}, {0x0F80, 0x0F84}, {0x0F86, 0x0F87}, {0x0F90, 0x0F97}, {0x0F99, 0x0FBC}, {0x0FC6, 0x0FC6}, {0x102D, 0x1030}, {0x1032, 0x1032}, {0x1036, 0x1037}, {0x1039, 0x1039}, {0x1058, 0x1059}, {0x1160, 0x11FF}, {0x135F, 0x135F}, {0x1712, 0x1714}, {0x1732, 0x1734}, {0x1752, 0x1753}, {0x1772, 0x1773}, {0x17B4, 0x17B5}, {0x17B7, 0x17BD}, {0x17C6, 0x17C6}, {0x17C9, 0x17D3}, {0x17DD, 0x17DD}, {0x180B, 0x180D}, {0x18A9, 0x18A9}, {0x1920, 0x1922}, {0x1927, 0x1928}, {0x1932, 0x1932}, {0x1939, 0x193B}, {0x1A17, 0x1A18}, {0x1B00, 0x1B03}, {0x1B34, 0x1B34}, {0x1B36, 0x1B3A}, {0x1B3C, 0x1B3C}, {0x1B42, 0x1B42}, {0x1B6B, 0x1B73}, {0x1DC0, 0x1DCA}, {0x1DFE, 0x1DFF}, {0x200B, 0x200F}, {0x202A, 0x202E}, {0x2060, 0x2063}, {0x206A, 0x206F}, {0x20D0, 0x20EF}, {0x302A, 0x302F}, {0x3099, 0x309A}, {0xA806, 0xA806}, {0xA80B, 0xA80B}, {0xA825, 0xA826}, {0xFB1E, 0xFB1E}, {0xFE00, 0xFE0F}, {0xFE20, 0xFE23}, {0xFEFF, 0xFEFF}, {0xFFF9, 0xFFFB}, {0x10A01, 0x10A03}, {0x10A05, 0x10A06}, {0x10A0C, 0x10A0F}, {0x10A38, 0x10A3A}, {0x10A3F, 0x10A3F}, {0x1D167, 0x1D169}, {0x1D173, 0x1D182}, {0x1D185, 0x1D18B}, {0x1D1AA, 0x1D1AD}, {0x1D242, 0x1D244}, {0xE0001, 0xE0001}, {0xE0020, 0xE007F}, {0xE0100, 0xE01EF}};

        /* test for 8-bit control characters */
        if (ucs == 0)
                return 0;
        if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
                return -1;

        /* binary search in table of non-spacing characters */
        if (bisearch(ucs, combining,
                     sizeof(combining) / sizeof(struct interval) - 1))
                return 0;

        /* if we arrive here, ucs is not a combining or C0/C1 control character */

        return 1 +
               (ucs >= 0x1100 &&
                (ucs <= 0x115f || /* Hangul Jamo init. consonants */
                 ucs == 0x2329 || ucs == 0x232a ||
                 (ucs >= 0x2e80 && ucs <= 0xa4cf &&
                  ucs != 0x303f) ||                  /* CJK ... Yi */
                 (ucs >= 0xac00 && ucs <= 0xd7a3) || /* Hangul Syllables */
                 (ucs >= 0xf900 && ucs <= 0xfaff) || /* CJK Compatibility Ideographs */
                 (ucs >= 0xfe10 && ucs <= 0xfe19) || /* Vertical forms */
                 (ucs >= 0xfe30 && ucs <= 0xfe6f) || /* CJK Compatibility Forms */
                 (ucs >= 0xff00 && ucs <= 0xff60) || /* Fullwidth Forms */
                 (ucs >= 0xffe0 && ucs <= 0xffe6) ||
                 (ucs >= 0x20000 && ucs <= 0x2fffd) ||
                 (ucs >= 0x30000 && ucs <= 0x3fffd)));
}

int mk_wcswidth(const wchar_t *pwcs, size_t n)
{
        int width = 0;

        for (; *pwcs && n-- > 0; pwcs++)
        {
                int w;
                if ((w = mk_wcwidth(*pwcs)) < 0)
                        return -1;
                else
                        width += w;
        }
        return width;
}

/* End Markus Kuhn code */

void copyHalfOrFullWidthCharsWithMaxWidth(const char *src, char *dst, int maxDisplayWidth)
{
        mbstate_t state;
        memset(&state, 0, sizeof(state));

        const char *p = src;
        char *o = dst;
        wchar_t wc;
        int widthSum = 0;

        while (*p)
        {
                size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &state);

                if (len == (size_t)-1)
                { // Invalid UTF-8/locale error
                        // Skip one byte, reinit state
                        p++;
                        memset(&state, 0, sizeof(state));
                        continue;
                }
                if (len == (size_t)-2)
                { // Incomplete sequence
                        break;
                }
                if (len == 0)
                { // Null terminator
                        break;
                }

                int w = wcwidth(wc);
                if (w < 0)
                {
                        // Non-printable character; skip it
                        p += len;
                        continue;
                }

                if (widthSum + w > maxDisplayWidth)
                        break;

                // Copy valid multibyte sequence
                memcpy(o, p, len);
                o += len;
                p += len;
                widthSum += w;
        }

        *o = '\0';
}

static bool hasFullwidthChars(const char *str)
{
        mbstate_t state;
        memset(&state, 0, sizeof(state));

        const char *p = str;
        wchar_t wc;

        while (*p)
        {
                size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &state);
                if (len == (size_t)-1 || len == (size_t)-2 || len == 0)
                        break;

                int w = mk_wcwidth(wc);
                if (w < 0)
                {
                        break;
                }
                if (w > 1)
                {
                        return true;
                }
                p += len;
        }
        return false;
}

void processName(const char *name, char *output, int maxWidth, bool stripUnneededChars)
{
        if (!name) {
                output[0] = '\0';
                return;
        }

        const char *lastDot = strrchr(name, '.');

        if (lastDot != NULL)
        {
                char tmp[MAXPATHLEN];
                size_t len = lastDot - name + 1;
                if (len >= sizeof(tmp))
                        len = sizeof(tmp) - 1;
                c_strcpy(tmp, name, len);
                tmp[len] = '\0';
                copyHalfOrFullWidthCharsWithMaxWidth(tmp, output, maxWidth);
        }
        else
        {
                copyHalfOrFullWidthCharsWithMaxWidth(name, output, maxWidth);
        }

        if (stripUnneededChars)
                removeUnneededChars(output, strnlen(output, maxWidth));

        trim(output, strlen(output));
}

void processNameScroll(const char *name, char *output, int maxWidth, bool isSameNameAsLastTime)
{
        const char *lastDot = strrchr(name, '.');
        size_t nameLength = strnlen(name, maxWidth);
        size_t scrollableLength = (lastDot != NULL) ? (size_t)(lastDot - name) : nameLength;

        if (scrollDelaySkippedCount <= startScrollingDelay && scrollableLength > (size_t)maxWidth)
        {
                scrollableLength = maxWidth;
                scrollDelaySkippedCount++;
                refresh = true;
                isLongName = true;
        }

        int start = (isSameNameAsLastTime) ? lastNamePosition : 0;

        if (finishedScrolling)
                scrollableLength = maxWidth;

        if (hasFullwidthChars(name))
        {
                processName(name, output, maxWidth, true);
        }
        else if (scrollableLength <= (size_t)maxWidth || finishedScrolling)
        {
                processName(name, output, scrollableLength, true);
        }
        else
        {
                isLongName = true;

                if ((size_t)(start + maxWidth) > scrollableLength)
                {
                        start = 0;
                        finishedScrolling = true;
                }

                c_strcpy(output, name + start, maxWidth + 1);

                removeUnneededChars(output, maxWidth);
                trim(output, maxWidth);

                lastNamePosition++;

                refresh = true;
        }
}

bool getIsLongName()
{
        return isLongName;
}

PixelData increaseLuminosity(PixelData pixel, int amount)
{
        PixelData pixel2;
        pixel2.r = pixel.r + amount <= 255 ? pixel.r + amount : 255;
        pixel2.g = pixel.g + amount <= 255 ? pixel.g + amount : 255;
        pixel2.b = pixel.b + amount <= 255 ? pixel.b + amount : 255;

        return pixel2;
}

PixelData decreaseLuminosityPct(PixelData base, float pct)
{
        PixelData c;

        int r = (int)((float)base.r * pct);
        int g = (int)((float)base.g * pct);
        int b = (int)((float)base.b * pct);

        c.r = (r < MIN_CHANNEL) ? MIN_CHANNEL : r;
        c.g = (g < MIN_CHANNEL) ? MIN_CHANNEL : g;
        c.b = (b < MIN_CHANNEL) ? MIN_CHANNEL : b;
        return c;
}

PixelData getGradientColor(PixelData baseColor, int row, int maxListSize, int startGradient, float minPct)
{
        if (row < startGradient)
                return baseColor;

        int steps = maxListSize - startGradient;

        float pct;

        if (steps <= 1)
                pct = minPct;
        else
                pct = 1.0f - ((row - startGradient) * (1.0f - minPct) / (steps - 1));

        if (pct < minPct)
                pct = minPct;
        if (pct > 1.0f)
                pct = 1.0f;
        return decreaseLuminosityPct(baseColor, pct);
}
