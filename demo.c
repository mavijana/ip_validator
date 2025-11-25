/*
 * IPv4/IPv6 validation regression harness comparing custom validators with
 * inet_pton references. Supports full-suite execution and single-address
 * verification via CLI.
 */

#include "ip_validator.h"

#include <stdio.h>
#include <unistd.h> /* getopt */

#include <arpa/inet.h>

/**
 * is_valid_ipv4_inet - Validate IPv4 text using inet_pton for parity testing.
 * @str: Candidate IPv4 address string.
 *
 * Return: true if inet_pton accepts @str, otherwise false.
 */
bool is_valid_ipv4_inet(const char *str)
{
    if (str == NULL || *str == '\0') {
        return false;
    }

    struct in_addr addr;
    /* inet_pton returns 1 on success, 0 if invalid format, -1 on error */
    int result = inet_pton(AF_INET, str, &addr);

    return result == 1;
}

/**
 * is_valid_ipv6_inet - Validate IPv6 text using inet_pton for parity testing.
 * @str: Candidate IPv6 address string.
 *
 * Return: true if inet_pton accepts @str, otherwise false.
 */
bool is_valid_ipv6_inet(const char *str)
{
    if (str == NULL || *str == '\0') {
        return false;
    }

    struct in6_addr addr;
    int result = inet_pton(AF_INET6, str, &addr);

    return result == 1;
}

/* Aggregates per-suite totals and pass counts for regression reporting. */
typedef struct {
    int total;
    int passed;
} test_stats;

/**
 * Logs per-test outcomes, highlighting discrepancies between reference and
 * custom validators.
 */
void report_test_result(test_stats * stats, const char *name,
                        const char *input,
                        bool expected, bool result_inet, bool result_custom)
{
    stats->total++;
    if (result_inet == expected && result_custom == expected) {
        printf("PASS %s: \"%s\" -> %s\n", name, input ? input : "NULL",
               result_inet ? "valid" : "invalid");
        stats->passed++;
    } else {
        printf("\x1b[31mFAIL %s: \"%s\" -> Expected: %s, inet: %s, "
               "custom: %s\x1b[0m\n",
               name, input ? input : "NULL", expected ? "valid" : "invalid",
               result_inet ? "valid" : "invalid",
               result_custom ? "valid" : "invalid");
    }
}

/**
 * Runs an IPv4 test case against both validators and reports the outcome.
 */
void test_case_ipv4(test_stats * stats, const char *name,
                    const char *input, bool expected)
{
    bool result_inet = is_valid_ipv4_inet(input);
    bool result_custom = is_valid_ipv4_address(input);
    report_test_result(stats, name, input, expected, result_inet,
                       result_custom);
}

/**
 * Runs an IPv6 test case against both validators and reports the outcome.
 */
void test_case_ipv6(test_stats * stats, const char *name,
                    const char *input, bool expected)
{
    bool result_inet = is_valid_ipv6_inet(input);
    bool result_custom = is_valid_ipv6_address(input);
    report_test_result(stats, name, input, expected, result_inet,
                       result_custom);
}

/**
 * Executes the IPv4 regression suite covering valid cases, edge cases, and
 * adversarial input.
 */
void run_ipv4_tests(test_stats * stats)
{
    /* Standard valid addresses */
    test_case_ipv4(stats, "IPv4: Minimum address", "0.0.0.0", true);
    test_case_ipv4(stats, "IPv4: Maximum address", "255.255.255.255", true);
    test_case_ipv4(stats, "IPv4: Localhost", "127.0.0.1", true);
    test_case_ipv4(stats, "IPv4: Private network", "192.168.1.1", true);
    test_case_ipv4(stats, "IPv4: Public IP", "8.8.8.8", true);

    /* Leading zeros - allowed in our implementation */
    // test_case_ipv4(stats, "IPv4: Leading zeros 1", "192.001.002.003", true);
    // test_case_ipv4(stats, "IPv4: Leading zeros 2", "010.020.030.040", true);
    // test_case_ipv4(stats, "IPv4: All zeros", "000.000.000.000", true);

    /* Edge cases */
    test_case_ipv4(stats, "IPv4: Max octets", "255.255.255.254", true);
    test_case_ipv4(stats, "IPv4: Mixed values", "1.2.3.4", true);

    /* Out of range */
    test_case_ipv4(stats, "IPv4 Invalid: Octet > 255", "256.1.1.1", false);
    test_case_ipv4(stats, "IPv4 Invalid: Large octet", "999.1.1.1", false);
    test_case_ipv4(stats, "IPv4 Invalid: All high", "300.300.300.300", false);

    /* Wrong number of octets */
    test_case_ipv4(stats, "IPv4 Invalid: Too few octets", "192.168.1", false);
    test_case_ipv4(stats, "IPv4 Invalid: Too many octets", "192.168.1.1.1",
                   false);
    test_case_ipv4(stats, "IPv4 Invalid: Single octet", "192", false);

    /* Empty/missing parts */
    test_case_ipv4(stats, "IPv4 Invalid: Empty octet", "192.168..1", false);
    test_case_ipv4(stats, "IPv4 Invalid: Trailing dot", "192.168.1.1.", false);
    test_case_ipv4(stats, "IPv4 Invalid: Leading dot", ".192.168.1.1", false);
    test_case_ipv4(stats, "IPv4 Invalid: Empty string", "", false);
    test_case_ipv4(stats, "IPv4 Invalid: NULL", NULL, false);

    /* Invalid characters */
    test_case_ipv4(stats, "IPv4 Invalid: Letter in octet", "192.168.a.1",
                   false);
    test_case_ipv4(stats, "IPv4 Invalid: Special chars", "192.168.1.1!", false);
    test_case_ipv4(stats, "IPv4 Invalid: Space", "192.168. 1.1", false);
    test_case_ipv4(stats, "IPv4 Invalid: Negative", "192.168.-1.1", false);

    /* Boundary attacks */
    test_case_ipv4(stats, "IPv4 Adversarial: 256 boundary", "255.255.255.256",
                   false);
    test_case_ipv4(stats, "IPv4 Adversarial: Overflow attempt",
                   "99999999999.1.1.1", false);

    /* Format confusion */
    test_case_ipv4(stats, "IPv4 Adversarial: Hex format", "0xC0.0xA8.0x01.0x01",
                   false);
    test_case_ipv4(stats, "IPv4 Adversarial: Octal-like", "0777.0777.0777.0777",
                   false);

    /* Injection attempts */
    test_case_ipv4(stats, "IPv4 Adversarial: SQL injection",
                   "192'; DROP TABLE--", false);
    test_case_ipv4(stats, "IPv4 Adversarial: Script injection", "192<script>",
                   false);

    /* Unicode/encoding tricks */
    test_case_ipv4(stats, "IPv4 Adversarial: Unicode digits",
                   "１９２.１６８.１.１", false);

    /* Multiple dots */
    test_case_ipv4(stats, "IPv4 Adversarial: Double dots", "192..168.1.1",
                   false);
    test_case_ipv4(stats, "IPv4 Adversarial: Triple dots", "192...168.1.1",
                   false);

    /* Trailing dot */
    test_case_ipv4(stats, "IPv4 Invalid: Trailing dot variant", "255.1.1.0.",
                   false);
    test_case_ipv4(stats, "IPv4 Invalid: Trailing dot max", "255.255.255.255.",
                   false);
}

/**
 * Executes the IPv6 regression suite covering valid cases, edge cases, and
 * adversarial input.
 */
void run_ipv6_tests(test_stats * stats)
{
    /* Standard format */
    test_case_ipv6(stats, "IPv6: Full format",
                   "2001:0db8:0000:0000:0000:0000:0000:0001", true);
    test_case_ipv6(stats, "IPv6: Compressed zeros", "2001:db8::1", true);
    test_case_ipv6(stats, "IPv6: All zeros", "::", true);
    test_case_ipv6(stats, "IPv6: Loopback", "::1", true);

    /* Leading zeros */
    test_case_ipv6(stats, "IPv6: With leading zeros",
                   "2001:0db8:0001:0000:0000:0ab9:C0A8:0102", true);
    test_case_ipv6(stats, "IPv6: No leading zeros",
                   "2001:db8:1:0:0:ab9:c0a8:102", true);

    /* Mixed case */
    test_case_ipv6(stats, "IPv6: Lowercase",
                   "2001:0db8:85a3:0000:0000:8a2e:0370:7334", true);
    test_case_ipv6(stats, "IPv6: Uppercase",
                   "2001:0DB8:85A3:0000:0000:8A2E:0370:7334", true);
    test_case_ipv6(stats, "IPv6: Mixed case",
                   "2001:0dB8:85a3:0000:0000:8A2e:0370:7334", true);

    /* IPv4-mapped IPv6 */
    test_case_ipv6(stats, "IPv6: IPv4-mapped 1", "::ffff:192.0.2.128", true);
    test_case_ipv6(stats, "IPv6: IPv4-mapped 2", "::ffff:c000:0280", true);
    test_case_ipv6(stats, "IPv6: IPv4-compatible", "::192.0.2.128", true);

    /* Compression at different positions */
    test_case_ipv6(stats, "IPv6: Compression at start", "::8a2e:0370:7334",
                   true);
    test_case_ipv6(stats, "IPv6: Compression at end", "2001:db8::", true);
    test_case_ipv6(stats, "IPv6: Compression in middle",
                   "2001:db8::8a2e:0370:7334", true);

    /* Link-local */
    test_case_ipv6(stats, "IPv6: Link-local", "fe80::1", true);
    test_case_ipv6(stats, "IPv6: Link-local full",
                   "fe80:0000:0000:0000:0204:61ff:fe9d:f156", true);

    /* Triple colon and invalid compression */
    test_case_ipv6(stats, "IPv6 Invalid: Triple colon", "2001:db8:::1", false);
    test_case_ipv6(stats, "IPv6 Invalid: Quad colon", "2001::::1", false);
    test_case_ipv6(stats, "IPv6 Invalid: Multiple compressions", "2001::db8::1",
                   false);

    /* Too many groups */
    test_case_ipv6(stats, "IPv6 Invalid: Too many groups", "1:2:3:4:5:6:7:8:9",
                   false);
    test_case_ipv6(stats, "IPv6 Invalid: No compression with 9",
                   "2001:0db8:0000:0000:0000:0000:0000:0000:0001", false);

    /* Invalid characters */
    test_case_ipv6(stats, "IPv6 Invalid: Invalid hex",
                   "2001:0db8:0g00:0000:0000:0000:0000:0001", false);
    test_case_ipv6(stats, "IPv6 Invalid: Special char",
                   "2001:0db8:0000:0000:0000:0000:0000:0001!", false);

    /* Group too long */
    test_case_ipv6(stats, "IPv6 Invalid: Group > 4 digits",
                   "2001:0db8:00000:0000:0000:0000:0000:0001", false);
    test_case_ipv6(stats, "IPv6 Invalid: Very long group",
                   "20011:0db8:0000:0000:0000:0000:0000:0001", false);

    /* Empty/NULL */
    test_case_ipv6(stats, "IPv6 Invalid: Empty string", "", false);
    test_case_ipv6(stats, "IPv6 Invalid: NULL", NULL, false);

    /* Malformed IPv4 suffix */
    test_case_ipv6(stats, "IPv6 Invalid: Bad IPv4 suffix", "::ffff:999.0.2.128",
                   false);
    test_case_ipv6(stats, "IPv6 Invalid: IPv4 wrong position",
                   "2001:db8:192.168.1.1::", false);

    /* Single colon issues */
    test_case_ipv6(stats, "IPv6 Invalid: Single colon start", ":2001:db8::1",
                   false);
    test_case_ipv6(stats, "IPv6 Invalid: Single colon end", "2001:db8::1:",
                   false);

    /* Compression bypass attempts */
    test_case_ipv6(stats, "IPv6 Adversarial: Full no compress",
                   "2001:0db8:0000:0000:0000:0000:0000:0001", true);
    test_case_ipv6(stats, "IPv6 Adversarial: Valid with trailing ::",
                   "2001:0db8:0001:0002:0003:0004:0005::", true);

    /* Case confusion */
    test_case_ipv6(stats, "IPv6 Adversarial: Mixed extreme",
                   "aBcD:EfGh:0000:0000:0000:0000:0000:0001", false);

    /* Boundary overflows */
    test_case_ipv6(stats, "IPv6 Adversarial: Hex overflow",
                   "ffffffff:0:0:0:0:0:0:1", false);

    /* IPv4 confusion */
    test_case_ipv6(stats, "IPv6 Adversarial: IPv4 only", "192.168.1.1", false);
    test_case_ipv6(stats, "IPv6 Adversarial: Mixed wrong", "2001:db8:192.168.1",
                   false);

    /* Colon edge cases */
    test_case_ipv6(stats, "IPv6 Adversarial: Only colons", ":::", false);
    test_case_ipv6(stats, "IPv6 Adversarial: Many colons", "::::::::", false);
    test_case_ipv6(stats, "IPv6 Adversarial: Alternating", ":1:2:3:4:5:6:7:8",
                   false);

    /* Empty groups */
    test_case_ipv6(stats, "IPv6 Adversarial: Empty groups", "2001::db8:::1",
                   false);

    /* Unicode tricks */
    test_case_ipv6(stats, "IPv6 Adversarial: Unicode colon", "2001:db8∶:1",
                   false);

    /* Injection attempts */
    test_case_ipv6(stats, "IPv6 Adversarial: Script tag", "2001:db8<script>::1",
                   false);
    test_case_ipv6(stats, "IPv6 Adversarial: SQL", "'; DROP TABLE--", false);

    /* IPv4 vs IPv6 confusion */
    test_case_ipv6(stats, "Edge: Short string", "1", false);
    test_case_ipv6(stats, "Edge: Just dots", "...", false);
    test_case_ipv6(stats, "Edge: Just colons IPv6", ":", false);

    /* Whitespace */
    test_case_ipv6(stats, "Edge: IPv4 with spaces", " 192.168.1.1", false);
    test_case_ipv6(stats, "Edge: IPv4 trailing space", "192.168.1.1 ", false);
    test_case_ipv6(stats, "Edge: IPv6 with space", " ::1", false);

    /* Very long strings */
    test_case_ipv6(stats, "Edge: Very long invalid",
                   "1111111111111111111111111111111111111111111111111111111111",
                   false);

    /* Unicode/encoding tricks */
    test_case_ipv6(stats, "IPv4 Adversarial: Unicode digits",
                   "１９２.１６８.１.１", false);
}

/**
 * Prints per-family pass/fail counts and aggregated totals after suite
 * execution.
 */
void print_summary(const test_stats * ipv4_stats, const test_stats * ipv6_stats)
{
    int total = ipv4_stats->total + ipv6_stats->total;
    int passed = ipv4_stats->passed + ipv6_stats->passed;

    printf("IPv4 totals %d:%d %d\n", ipv4_stats->total, ipv4_stats->passed,
           ipv4_stats->total - ipv4_stats->passed);
    printf("IPv6 totals %d:%d %d\n", ipv6_stats->total, ipv6_stats->passed,
           ipv6_stats->total - ipv6_stats->passed);
    printf("Combined %d:%d %d\n", total, passed, total - passed);
}

/**
 * Explains command-line options for running regression suites or single-address
 * checks.
 */
void print_usage(const char *prog)
{
    printf("Usage: %s [-4 <address>] [-6 <address>] [-h]\n", prog);
    printf("       No arguments: run IPv4 and IPv6 regression suites.\n");
}

/**
 * Entry point that parses CLI arguments and triggers regression runs or
 * individual validations.
 */
int main(int argc, char *argv[])
{
    int opt;
    const char *ipv4_single = NULL;
    const char *ipv6_single = NULL;

    while ((opt = getopt(argc, argv, "4:6:h")) != -1) {
        switch (opt) {
        case '4':
            ipv4_single = optarg;
            break;
        case '6':
            ipv6_single = optarg;
            break;
        case 'h':
            print_usage(argv[0]);
            return 0;
        default:
            print_usage(argv[0]);
            return 1;
        }
    }

    if (optind < argc) {
        print_usage(argv[0]);
        return 1;
    }

    if (ipv4_single == NULL && ipv6_single == NULL) {
        test_stats ipv4_stats = { 0, 0 };
        test_stats ipv6_stats = { 0, 0 };
        run_ipv4_tests(&ipv4_stats);
        run_ipv6_tests(&ipv6_stats);
        print_summary(&ipv4_stats, &ipv6_stats);
        return 0;
    }

    if (ipv4_single != NULL) {
        bool custom = is_valid_ipv4_address(ipv4_single);
        bool reference = is_valid_ipv4_inet(ipv4_single);
        printf("Custom IPv4 validator: %s is %s\n", ipv4_single,
               custom ? "valid" : "invalid");
        printf("inet_pton reference: %s is %s\n", ipv4_single,
               reference ? "valid" : "invalid");
    }

    if (ipv6_single != NULL) {
        bool custom = is_valid_ipv6_address(ipv6_single);
        bool reference = is_valid_ipv6_inet(ipv6_single);
        printf("Custom IPv6 validator: %s is %s\n", ipv6_single,
               custom ? "valid" : "invalid");
        printf("inet_pton reference: %s is %s\n", ipv6_single,
               reference ? "valid" : "invalid");
    }

    return 0;
}
