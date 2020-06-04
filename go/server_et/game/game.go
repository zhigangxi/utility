package main

import (
	"log"
	"os"
	game "server/game/server"
)

func main() {
	if len(os.Args) < 2 {
		log.Printf("Please input config file name\n")
		return
	}
	game.GetGameServer().Start(os.Args[1])
}
