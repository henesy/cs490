package main

import (
	"net"
	"log"
	"os"
	"os/signal"
	"time"
	"syscall"
sc	"strconv"
	"os/exec"
	"fmt"

	// Makes life a lot easier and has no external deps
	"github.com/takama/daemon"
)


var dname string = "mca66d"
var port string = ":8664"
var logfile string = "/var/log/mca66.log"


/* Handle language translation -- given a string produce write-able outputs (and err) */
func docmd(parts []string) (out []byte, err error) {
		cmd := parts[0]		
		var zone int
		
		// Produce the zone from the parts
		mkzone := func() {
			// Check err later
			zone, _ = sc.Atoi(parts[1])
			if zone < 0 || zone > 7 {
				log.Print("Invalid zone from client: ", zone)
				out = []byte("Error: invalid zone\n")
				err = fmt.Errorf("Invalid zone")
			}
		}
		
		switch(cmd) {
			case "POWER":
				// POWER 1 ON
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "VOLUME":
				// VOLUME 1 UP
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "BASS":
				// BASS 1 UP
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "TREBLE":
				// TREBLE 1 UP
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "BALANCE":
				// BALANCE 1 LEFT
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "INPUT":
				mkzone()
				// INPUT 1 2
				// Set input channel to channel 2 for zone 1
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2], parts[3])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "QUERY":
				// QUERY 1
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone))
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone))
				out, err = cntl.CombinedOutput()
			case "MUTE":
				// MUTE 1 OFF
				// Unmute zone 1
				mkzone()
				cntl := exec.Command("cntl", cmd, sc.Itoa(zone), parts[2])
				log.Print("Running: ", cntl.Path, " ", cmd, " ", sc.Itoa(zone), " ", parts[2])
				out, err = cntl.CombinedOutput()
			case "STATUS":
				// STATUS
				// Get status points of zones
				cntl := exec.Command("cntl", cmd)
				log.Print("Running: ", cntl.Path, " ", cmd)
				out, err = cntl.CombinedOutput()
				log.Print("Status info: ", out)
			default:
				log.Print("Error, unknown command: ", cmd)
				out = []byte("Error, unknown command.\n")
				err = fmt.Errorf("Invalid command")
		}
		return
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
	go tcp_acceptor(l)
   	 
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
