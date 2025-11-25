#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>

/* Maximum dotted-quad length including NUL */
#define MAX_SIZE_IPV4 16

/**
 * parse_int - Parse a decimal substring into an integer.
 * @str: Source string containing digits.
 * @start: Inclusive starting index within @str.
 * @end: Exclusive ending index within @str.
 * @result: Output pointer that receives the parsed value.
 *
 * Return: true on success, false if the substring is empty, contains
 * non-digits, or would overflow the IPv4 octet range.
 */
static bool parse_int(const char *str, int start, int end, int *result)
{
    if (start >= end || str == NULL || *str == '\0')
        return false;

    *result = 0;
    for (int i = start; i < end; i++) {
        if (!isdigit(str[i]))
            return false;

        /* Check for overflow before multiplication */
        if (*result > 25 || (*result == 25 && (str[i] - '0') > 5)) {
            return false;
        }
        *result = *result * 10 + (str[i] - '0');
    }
    return true;
}

/**
 * is_valid_ipv4_address - Validate dotted-quad IPv4 text representation.
 * @str: Null-terminated string to examine.
 *
 * Return: true if @str is a syntactically valid IPv4 address, otherwise false.
 */
bool is_valid_ipv4_address(const char *str)
{
    if (str == NULL)
        return false;

    size_t len = strnlen(str, MAX_SIZE_IPV4);
    int octet_count = 0;
    size_t octet_start = 0;
    int octet_value;

    for (size_t i = 0; i <= len; i++) {
        if (str[i] == '.' || str[i] == '\0') {
            /* Found end of octet */
            if (i == octet_start) {
                /* Empty octet */
                return false;
            }

            if (!parse_int(str, (int)octet_start, (int)i, &octet_value)) {
                return false;
            }

            /* Check range 0-255 */
            if (octet_value > 255) {
                return false;
            }

            octet_count++;
            octet_start = i + 1;

            if (str[i] == '\0')
                break;
        } else if (!isdigit((unsigned char)str[i])) {
            /* Invalid character */
            return false;
        }
    }

    /* Must have exactly 4 octets */
    return octet_count == 4;
}

/**
 * hex_digit_value - Convert a hexadecimal character to its numeric value.
 * @ch: Character to convert.
 *
 * Return: Numeric value 0-15 on success, or -1 if @ch is not hexadecimal.
 */
static int hex_digit_value(char ch)
{
    if ('0' <= ch && ch <= '9')
        return ch - '0';
    if ('a' <= ch && ch <= 'f')
        return ch - 'a' + 10;
    if ('A' <= ch && ch <= 'F')
        return ch - 'A' + 10;
    return -1;
}

/**
 * is_valid_ipv6_address - Validate IPv6 text representation.
 * @str: Null-terminated string to examine.
 *
 * Return: true if @str is a syntactically valid IPv6 address, otherwise false.
 */
bool is_valid_ipv6_address(const char *str)
{
    if (str == NULL || *str == '\0')
        return false;

    const char *src = str;
    const char *src_endp = str + strnlen(str, INET6_ADDRSTRLEN);
    int group_count = 0;
    bool has_compression = false;
    const char *curtok;
    size_t xdigits_seen = 0;
    unsigned int val = 0;

    /* Leading :: requires some special handling. */
    if (src == src_endp)
        return false;
    if (*src == ':') {
        ++src;
        if (src == src_endp || *src != ':')
            return false;
    }

    curtok = src;

    while (src < src_endp) {
        int ch = *src++;
        int digit = hex_digit_value(ch);
        if (digit >= 0) {
            if (xdigits_seen == 4)
                return false;
            val <<= 4;
            val |= (unsigned int)digit;
            if (val > 0xffff)
                return false;
            ++xdigits_seen;
            continue;
        }
        if (ch == ':') {
            curtok = src;
            if (xdigits_seen == 0) {
                if (has_compression)
                    return false;   /* Multiple :: not allowed */
                has_compression = true;
                continue;
            } else if (src == src_endp) {
                return false;
            }

            group_count++;
            if (group_count > 8)
                return false;

            xdigits_seen = 0;
            val = 0;
            continue;
        }
        if (ch == '.' && xdigits_seen > 0) {
            /* IPv4 suffix - validate it separately */
            const char *ipv4_start = curtok;
            int dots = 0;
            const char *p = curtok;

            /* Count dots and validate basic IPv4 format */
            while (p < src_endp) {
                if (*p == '.')
                    dots++;
                else if (!isdigit((unsigned char)*p))
                    return false;
                p++;
            }

            if (dots != 3)
                return false;

            /* Simple IPv4 validation */
            if (!is_valid_ipv4_address(ipv4_start))
                return false;

            group_count += 2;   /* IPv4 takes 2 groups worth */
            xdigits_seen = 0;
            break;
        }
        return false;
    }

    if (xdigits_seen > 0) {
        group_count++;
        if (group_count > 8)
            return false;
    }

    /* Check if we have the right number of groups */
    if (has_compression) {
        if (group_count >= 8)
            return false;
    } else {
        if (group_count != 8)
            return false;
    }

    return true;
}
