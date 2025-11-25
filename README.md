# IPv4/IPv6 Validator

## Overview

This project provides standalone validators for IPv4 and IPv6 text representations along with a regression harness. The harness checks the custom validators against `inet_pton`, so you can exercise the full regression suite or validate individual addresses from the command line.

## Building

Prerequisites:

- `clang` (or any C11 compliant compiler)
- `make`

Build the demo executable:

```bash
make
```

Clean generated artifacts:

```bash
make clean
```

## Running the Demo

Execute the full IPv4 and IPv6 regression suites:

```bash
./demo
```

Validate a single IPv4 address:

```bash
./demo -4 192.168.1.1
```

Validate a single IPv6 address:

```bash
./demo -6 2001:db8::1
```

Combine both flags to check one IPv4 and one IPv6 address in the same run. Use `-h` to display usage information.

## Docker Workflow

Build the container image and run the sample validation commands defined in the `Makefile`:

```bash
make docker
```

You can also interact with the container manually:

```bash
docker build -t ipv-validator .
docker run --rm ipv-validator ./demo -4 192.168.1.1
```

## Formatting

Apply the configured `indent` formatting to source and header files:

```bash
make format
```

## Project Layout

- `ip_validator.c` / `ip_validator.h` — IPv4 and IPv6 validation routines
- `demo.c` — regression harness and CLI interface
- `Makefile` — build, run, and maintenance targets
- `Dockerfile` — containerized build and demo runner
