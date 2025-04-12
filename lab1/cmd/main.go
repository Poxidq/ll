package main

import (
	"bldd/cmd/cli"
	customErrors "bldd/cmd/errors"
	"bldd/service"
)

func main() {
	config, err := cli.ParseCommandLine()
	if err != nil {
		customErrors.HandleErrorAndExit(customErrors.UsageError(err))
		return
	}

	if config.ShowHelp {
		cli.ShowUsage()
		return
	}

	service.Run(config)
}
