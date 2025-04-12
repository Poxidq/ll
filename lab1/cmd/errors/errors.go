package errors

import (
	"fmt"
	"os"
	"strings"
)

// ExitCode represents different types of application errors
type ExitCode int

const (
	ExitSuccess ExitCode = 0

	ExitUsageError ExitCode = 1

	ExitFileError ExitCode = 2

	ExitRuntimeError ExitCode = 3
)

// ErrorWithCode represents an error with an associated exit code
type ErrorWithCode struct {
	Err      error
	Code     ExitCode
	Messages []string
}

func NewError(err error, code ExitCode, messages ...string) *ErrorWithCode {
	return &ErrorWithCode{
		Err:      err,
		Code:     code,
		Messages: messages,
	}
}

func (e *ErrorWithCode) Error() string {
	if e.Err != nil {
		if len(e.Messages) > 0 {
			return fmt.Sprintf("%s: %v", strings.Join(e.Messages, ": "), e.Err)
		}
		return e.Err.Error()
	}
	return strings.Join(e.Messages, ": ")
}

func HandleErrorAndExit(err error) {
	if err == nil {
		return
	}

	exitCode := ExitRuntimeError

	if e, ok := err.(*ErrorWithCode); ok {
		exitCode = e.Code
	}

	fmt.Fprintf(os.Stderr, "Error: %v\n", err)

	os.Exit(int(exitCode))
}

func UsageError(err error, messages ...string) *ErrorWithCode {
	return NewError(err, ExitUsageError, messages...)
}

func FileError(err error, messages ...string) *ErrorWithCode {
	return NewError(err, ExitFileError, messages...)
}

func RuntimeError(err error, messages ...string) *ErrorWithCode {
	return NewError(err, ExitRuntimeError, messages...)
}
