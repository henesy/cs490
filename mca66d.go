package main

import (
	"net"
	"log"
	"os"
	"os/signal"
	"time"
	"strings"
	"syscall"

	// Our only non-stdlib dependency, makes life a lot easier and has no further deps
	"github.com/takama/daemon"
)


var dname string = "mca66d"
var port string = ":8664"


/* Handle clients as they're passed from acceptor */
func handler(conn net.Conn) {
	// Sequentially read from the client one line at a time for commands
	for {
		buf := make([]byte, 1024)
		_, err := conn.Read(buf)
		if err != nil {
			log.Print("Error, connection failure: ", err)
			return
		}

		str := string(buf)
		parts := strings.Split(str, " ")

		// Each command will be two parts, [CMD args…] format
		if len(parts) < 2 {
			log.Print("Bad command format from client: ", str)
			conn.Write([]byte("Error: invalid command format"))
			continue
		}

		cmd := parts[0]
		//args := parts[1:]

		switch(cmd) {
			case "POWER":
				// Do power call, etc.
			default:
		}

		// Might be unnecessary
		time.Sleep(5 * time.Millisecond)
	}
}

/* Accept clients as they come in */
func acceptor(l net.Listener) {
	for {
		conn, err := l.Accept()
		if err != nil {
			log.Print("Error, attempted connection to listener failed: ", err)
			continue
		}
		go handler(conn)

		time.Sleep(50 * time.Millisecond)
	}
}

/* Supervise daemon duties */
func supervisor(s *daemon.Daemon) (string, error) {
	usage := "Usage: " + os.Args[0] + "  install | remove | start | stop | status"
	
	// If invoked with an argument, perform daemon duties
	if len(os.Args) > 1 {
		command := os.Args[1]
		switch command {
			case "install":
				return (*s).Install()
			case "remove":
				return (*s).Remove()
			case "start":
				return (*s).Start()
			case "stop":
				return (*s).Stop()
			case "status":
				return (*s).Status()
			default:
				return usage, nil
		}
	}
   	 
	// Pass signals over channel if received
	sig := make(chan os.Signal, 1)
	signal.Notify(sig, os.Interrupt, os.Kill, syscall.SIGTERM)
   	 
	// Open socket to listen for tcp connections (maybe this should be unix network sockets?)
	l, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatal("Error, listener: ", err)
	}
	go acceptor(l)
   	 
   	 for {
		select {
		case killSignal := <-sig:
			log.Print("Received interrupt signal: ", killSignal)
			log.Print("Listener going down: ", l.Addr())
			l.Close()
			if killSignal == os.Interrupt {
				return "Received os.Interrupt, going down", nil
			}
			return "Received interrupt, going down", nil
		default:
			// Shouldn't be a problem at this increment
			time.Sleep(5 * time.Millisecond)
		}
	}

	return usage, nil
}

/* A daemon to listen and make calls to mca66 */
func main() {
	service, err := daemon.New(dname, "Controller for mca66")
	if err != nil {
		log.Fatal("Error, daemonizition: ", err)
	}

	status, err := supervisor(&service)
	if err != nil {
		log.Fatal("Status: ", status, " ;; Error, supervisor failure: ", err)
	}
	log.Print(status)
}
