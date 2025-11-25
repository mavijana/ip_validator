# Use Ubuntu as base image
FROM ubuntu:22.04

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install clang and build essentials
RUN apt-get update && \
    apt-get install -y \
    clang \
    make \
    && rm -rf /var/lib/apt/lists/*

# Set clang as the default compiler
ENV CC=clang

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the project
RUN make clean && make

# Default command to show available executables
CMD ["sh", "-c", "echo 'Available executables:' && ls -l ipv4_validator ipv6_validator"]
