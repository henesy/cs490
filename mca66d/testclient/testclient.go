package main

import (
	"fmt"
	"net"
	"flag"
	"os"
	"io"
)


/* Test client interpreter to talk to tcp servers */
func main() {
	var port string
	var addr string
	var size uint

	flag.StringVar(&port, "p", ":8664", "port to dial to")
	flag.StringVar(&addr, "h", "localhost", "host to dial to")
	// It would be nice to make this variable and setable by an escape key
	flag.UintVar(&size, "w", 1024, "size in bytes of individual writes to server")
	flag.Parse()

	conn, err := net.Dial("tcp", addr + port)
	if err != nil {
		fmt.Fprintln(os.Stderr, "Error dialing: ", err)
		os.Exit(1)
	}

	// Read lines until EOF (ctrl+d) is sent
	var readerr error
	for readerr != io.EOF {
		buf := make([]byte, size)
		str := ""

		fmt.Print("\n> ")
		_, readerr = fmt.Scanln(&str)

		// Truncate if need be
		for p, v := range str {
			if uint(p) >= size {
				break
			}
			buf[p] = byte(v)
		}


		_, err = conn.Write(buf)
		if err != nil {
			fmt.Fprintln(os.Stderr, "Error writing: ", err)
		}
		_, err = conn.Read(buf)
		if err != nil {
			fmt.Fprintln(os.Stderr, "Error reading: ", err)
		}
		fmt.Println("\n", string(buf))
	}
}

