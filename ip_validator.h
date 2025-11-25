#ifndef IP_VALIDATOR_H
#define IP_VALIDATOR_H

#include <stdbool.h>

/**
 * is_valid_ipv4_address - Validate dotted-decimal IPv4 text.
 * @str: Null-terminated string to examine.
 *
 * Return: true if @str represents a syntactically valid IPv4 address, false otherwise.
 */
bool is_valid_ipv4_address(const char *str);

/**
 * is_valid_ipv6_address - Validate textual IPv6 representation.
 * @str: Null-terminated string to examine.
 *
 * Return: true if @str represents a syntactically valid IPv6 address, false otherwise.
 */
bool is_valid_ipv6_address(const char *str);

#endif                          /* IP_VALIDATOR_H */
