package main

import (
	"net"
	"log"
	"time"
	"strings"
)

/* Handle clients as they're passed from tcp_acceptor -- tcp */
func tcp_handler(conn net.Conn) {
	// Write errors in a cleaner manner
	werr := func(msg string) {
		conn.Write([]byte("Error: " + msg + "\n"))
		time.Sleep(5 * time.Millisecond)
	}

	// Sequentially read from the client one line at a time for commands
	for {
		buf := make([]byte, 1024)
		_, err := conn.Read(buf)
		if err != nil {
			log.Print("Error, connection failure: ", err)
			break
		}

		str := string(buf)
		parts := strings.Fields(str)

		// Should be replaced with better sanitization
		if len(parts) > 4 {
			log.Print("Bad command format from client: ", str)
			werr("invalid command format")
			continue
		}

		out, err := docmd(parts)
		if err != nil {
			werr(string(out))
			continue
		}
		
		if err != nil {
			log.Print("Error, couldn't run: ", err)
			werr("command failed: " + err.Error())
			continue
		}
		conn.Write([]byte("Ok.\n"))

		// Might be unnecessary
		time.Sleep(5 * time.Millisecond)
	}
}

/* Accept clients as they come in -- tcp */
func tcp_acceptor(l net.Listener) {
	for {
		conn, err := l.Accept()
		if err != nil {
			log.Print("Error, attempted connection to listener failed: ", err)
			continue
		}
		go tcp_handler(conn)

		time.Sleep(50 * time.Millisecond)
	}
}
