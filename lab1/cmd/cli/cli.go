package cli

import (
	"flag"
	"fmt"
	"os"
	"path/filepath"
)

type Config struct {
	LibraryPaths  []string
	ScanDirectory string
	ReportFormat  string
	ShowHelp      bool
}

func ParseCommandLine() (*Config, error) {
	cfg := &Config{}

	flag.StringVar(&cfg.ScanDirectory, "dir", ".", "Directory to scan for executables")
	flag.StringVar(&cfg.ReportFormat, "output", "output.txt", "Report output path")
	help := flag.Bool("help", false, "Show help message")
	h := flag.Bool("h", false, "Show help message (shorthand)")

	flag.Parse()

	cfg.ShowHelp = *help || *h
	if cfg.ShowHelp {
		return cfg, nil
	}

	info, err := os.Stat(cfg.ScanDirectory)
	if err != nil {
		return nil, fmt.Errorf("invalid scan directory: %w", err)
	}
	if !info.IsDir() {
		return nil, fmt.Errorf("not a directory: %s", cfg.ScanDirectory)
	}

	cfg.ScanDirectory, err = filepath.Abs(cfg.ScanDirectory)
	if err != nil {
		return nil, fmt.Errorf("failed to get absolute path: %w", err)
	}

	cfg.LibraryPaths = flag.Args()
	if len(cfg.LibraryPaths) == 0 {
		return nil, fmt.Errorf("no library files specified")
	}

	return cfg, nil
}

func ShowUsage() {
	appName := filepath.Base(os.Args[0])
	fmt.Printf("bldd - Backward ldd: Find executables that use specified shared libraries\n\n")
	fmt.Printf("Usage: %s [options] library1 [library2...]\n\n", appName)
	fmt.Printf("Options:\n")
	flag.PrintDefaults()
	fmt.Printf("\nExamples:\n")
	fmt.Printf("  %s -dir=/usr/bin libbz2.so.1.0 liblzma.so.5\n", appName)
}
