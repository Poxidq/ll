package service

import (
	"bldd/cmd/cli"
	"fmt"
	"os"
	"slices"
	"sort"
)

// key -> shared library we are looking for
type ElfWithShared map[string]ElfWithSharedInfo

type ElfWithSharedInfo struct {
	Objects []string // path to executables
	Arch    string   // architecture x86 / x86-64 / armv7 / aarch64
}

var elfMap = make(ElfWithShared)

func Run(config *cli.Config) {
	err := LoopFiles(config.ScanDirectory)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error scanning directory: %v\n", err)
		return
	}

	// Prepare data for report generation
	reportData := make(map[string]map[string][]string)

	for library, info := range elfMap {
		if slices.Contains(config.LibraryPaths, library) {
			if reportData[info.Arch] == nil {
				reportData[info.Arch] = make(map[string][]string)
			}
			reportData[info.Arch][library] = info.Objects
		}
	}

	// Generate report
	reportFile, err := os.Create(config.ReportFormat)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error creating report file: %v\n", err)
		return
	}
	defer reportFile.Close()

	for arch, libraries := range reportData {
		fmt.Fprintf(reportFile, "---------- %s ----------\n", arch)
		type libraryInfo struct {
			Name      string
			Executors []string
			Count     int
		}
		var sortedLibraries []libraryInfo

		for lib, execs := range libraries {
			sortedLibraries = append(sortedLibraries, libraryInfo{
				Name:      lib,
				Executors: execs,
				Count:     len(execs),
			})
		}

		sort.Slice(sortedLibraries, func(i, j int) bool {
			return sortedLibraries[i].Count > sortedLibraries[j].Count
		})

		for _, libInfo := range sortedLibraries {
			fmt.Fprintf(reportFile, "%s (%d execs)\n", libInfo.Name, libInfo.Count)
			for _, exec := range libInfo.Executors {
				fmt.Fprintf(reportFile, "-> %s\n", exec)
			}
		}
	}
}
