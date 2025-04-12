package pkg

import (
	"bytes"
	"debug/elf"
	"fmt"
)

// ELF magic number
var elfMagic = []byte{0x7f, 'E', 'L', 'F'}

// checks by bytes that it's elf file
func IdentifyElf(dat []byte) (bool, error) {
	magic := dat[:4]

	return string(magic) == string(elfMagic), nil
}

func readString(data []byte, offset int) (string, error) {
	if offset < 0 || offset >= len(data) {
		return "", fmt.Errorf("offset out of bounds")
	}

	end := offset
	for end < len(data) && data[end] != 0 {
		end++
	}

	return string(data[offset:end]), nil
}

// List all shared libraries that this elf uses by parsing elf structure
func FindSharedLibraries(elfData []byte) ([]string, error) {
	f, err := elf.NewFile(bytes.NewReader(elfData))
	if err != nil {
		return nil, fmt.Errorf("failed to parse ELF file: %v", err)
	}
	defer f.Close()

	libs, err := f.DynString(elf.DT_NEEDED)
	if err != nil {
		return nil, fmt.Errorf("failed to get shared libraries: %v", err)
	}
	return libs, nil
}

func GetBinaryArchitecture(elfData []byte) string {
	f, err := elf.NewFile(bytes.NewReader(elfData))
	if err != nil {
		panic(fmt.Errorf("failed to parse ELF file: %v", err))
	}
	defer f.Close()
	switch f.FileHeader.Machine {
	case elf.EM_386:
		return "x86"
	case elf.EM_X86_64:
		return "x86_64"
	case elf.EM_ARM:
		return "arm"
	case elf.EM_AARCH64:
		return "aarch64"
	case elf.EM_MIPS:
		return "mips"
	case elf.EM_PPC:
		return "powerpc"
	case elf.EM_RISCV:
		return "riscv"
	case elf.EM_SPARC:
		return "sparc"
	default:
		return "unknown"
	}
}
