package main

import (
	"net"
	"log"
	"os"
	"os/signal"
	"os/exec"
	"time"
	"strings"
	"syscall"
sc	"strconv"

	// Our only non-stdlib dependency, makes life a lot easier and has no further deps
	"github.com/takama/daemon"
)


var dname string = "mca66d"
var port string = ":8664"
var logfile string = "/var/log/mca66.log"


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
		parts := strings.Fields(str)

		// Each command will be two parts, [CMD args…]
		if len(parts) > 4 {
			//log.Print("Len parts: ", len(parts))
			log.Print("Bad command format from client: ", str)
			conn.Write([]byte("Error: invalid command format\n"))
			continue
		}

		cmd := parts[0]
		// Check err later
		zone, _ := sc.Atoi(parts[1])
		if zone < 0 || zone > 7 {
			log.Print("Invalid zone from client: ", zone)
			conn.Write([]byte("Error: invalid zone\n"))
			continue
		}
		
		var out []byte

		// Need more strict parsing rules
		switch(cmd) {
			case "POWER":
				// POWER 1 ON
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "VOLUME":
				// VOLUME 1 UP
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "BASS":
				// BASS 1 UP
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "TREBLE":
				// TREBLE 1 UP
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "BALANCE":
				// BALANCE 1 LEFT
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "INPUT":
				// INPUT 1 -2
				// UN-Set input channel to channel 2 for zone 1 ;; a leading - and + are used, rather than using positive and negative values (parse as len=2 str)
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2], parts[3])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2], parts[3])
				out, err = cntl.CombinedOutput()
			case "QUERY":
				// QUERY 1
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "MUTE":
				// MUTE 1 OFF
				// Unmute
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone), parts[2])
				out, err = cntl.CombinedOutput()
			case "STATUS":
				// STATUS 1
				// Get status points of zones
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone))
				log.Print("Running: ", cntl.Path, cmd, sc.Itoa(zone))
				out, err = cntl.CombinedOutput()
				log.Print("Status info: ", out)
			default:
				conn.Write([]byte("Error, unknown command.\n"))
				continue
		}
		if err != nil {
			log.Print("Error, couldn't run: ", err)
			conn.Write([]byte("Error, command failed: " + err.Error() + "\n"))
			continue
		}
		conn.Write([]byte("Ok.\n"))
				

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
	f, err := os.OpenFile(logfile, os.O_RDWR | os.O_CREATE | os.O_APPEND, 0644)
	if err != nil {
		log.Fatal("Error, unable to open ", logfile, ": ", err)
	}
	defer f.Close()
	log.SetOutput(f)

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
