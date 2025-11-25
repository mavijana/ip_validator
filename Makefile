# Makefile for IPv4/IPv6 Address Validator
CC = clang
CFLAGS = -Wall -Wextra -Werror -std=c11 -pedantic -O2 -D_POSIX_C_SOURCE=200809L
LDFLAGS = 

# Source files
VALIDATOR_SRC = ip_validator.c
DEMO_SRC = demo.c

VALIDATOR_OBJ = $(VALIDATOR_SRC:.c=.o)
DEMO_OBJ = $(DEMO_SRC:.c=.o)

HEADERS = ip_validator.h

# Executables
DEMO_TARGET = demo

.PHONY: all clean rundemo demo docker format help

# Default target
all: $(DEMO_TARGET)

# Link demo executable
$(DEMO_TARGET): $(VALIDATOR_OBJ) $(DEMO_OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

# Run demo
rundemo: $(DEMO_TARGET)
	@./$(DEMO_TARGET)


# Clean build artifacts
clean:
	rm -f $(VALIDATOR_OBJ) $(DEMO_OBJ) $(TEST_TARGET) $(DEMO_TARGET)

# Build and exercise the Dockerized demo
docker:
	docker build -t ipv-validator .
	docker run --rm ipv-validator ./demo -6 2001:db8::1
	docker run --rm ipv-validator ./demo -4 192.168.1.1
	docker run --rm ipv-validator ./demo

# Format source
INDENT_FLAGS = -linux -i4 -ci4 -nut -ts4 -l80 -sob
FORMAT = indent $(INDENT_FLAGS)
SRC := $(VALIDATOR_SRC) $(DEMO_SRC) $(HEADERS)
format:
	@echo "Applying indent..."
	@for f in $(SRC); do \
		echo "  $$f"; \
		$(FORMAT) $$f; \
	done


# Show help
help:
	@echo "IPv4/IPv6 Validator Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build the demo executable"
	@echo "  make demo     - Same as 'make'"
	@echo "  make rundemo  - Run the demo locally"
	@echo "  make clean    - Remove compiled files"
	@echo "  make docker   - Build and run Docker demo"
	@echo "  make format   - Apply indent formatting"
	@echo "  make help     - Show this help message"
