# bldd - Backward ldd

> bldd is a command-line tool designed to scan directories for executables and identify which shared libraries they depend on. It generates a report categorizing executables by architecture and shared libraries.

## Installation

```bash
go build -o bldd ./cmd/main.go
```

## Usage

```bash
./bldd [options] library1 [library2...]
./bldd -h
bldd - Backward ldd: Find executables that use specified shared libraries

Usage: main [options] library1 [library2...]

Options:
  -dir string
    	Directory to scan for executables (default ".")
  -h	Show help message 
  -help
    	Show help message
  -output string
    	Report output path (default "output.txt")

Examples:
  main -dir=/usr/bin libssl.so.3
```

Example output:
```
---------- x86_64 ----------
liblzma.so.5 (35 execs)
-> /usr/bin/binwalk
-> /usr/bin/debuginfod
...

libbz2.so.1.0 (24 execs)
-> /usr/bin/base64conv
-> /usr/bin/binwalk
...
```
