package service

import (
	"bldd/pkg"
	"fmt"
	"os"
	"path/filepath"
)

func LoopFiles(path string) error {
	files, err := os.ReadDir(path)
	if err != nil {
		return err
	}

	for _, file := range files {
		if file.IsDir() {
			err := LoopDirs(filepath.Join(path, file.Name()))
			if err != nil {
				return err
			}
		} else {
			filePath := filepath.Join(path, file.Name())
			dat, err := os.ReadFile(filePath)

			if err != nil {
				// Log the error and continue with the next file
				fmt.Fprintf(os.Stderr, "Error reading file %s: %v\n", filePath, err)
				continue
			}

			isElf, err := pkg.IdentifyElf(dat)
			if err != nil {
				fmt.Fprintf(os.Stderr, "Error identifying ELF in file %s: %v\n", filePath, err)
				continue
			}

			if isElf {
				libraries, err := pkg.FindSharedLibraries(dat)
				if err != nil {
					fmt.Fprintf(os.Stderr, "Error finding shared libraries in file %s: %v\n", filePath, err)
					continue
				}

				for _, library := range libraries {
					elfMap[library] = ElfWithSharedInfo{
						Objects: append(elfMap[library].Objects, filePath),
						Arch:    pkg.GetBinaryArchitecture(dat),
					}
				}
			}
		}
	}
	return nil
}

func LoopDirs(path string) error {
	files, err := os.ReadDir(path)
	if err != nil {
		return err
	}

	for _, file := range files {
		if file.IsDir() {
			err := LoopDirs(filepath.Join(path, file.Name()))
			if err != nil {
				return err
			}
		} else {
			err := LoopFiles(path)
			if err != nil {
				return err
			}
		}
	}
	return nil
}
