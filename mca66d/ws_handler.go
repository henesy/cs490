package main

import (
	"net/http"
	"log"
	"time"

	// Extension library to stdlib that handles websockets nicely
	"github.com/gorilla/websocket"
)


/* Upgrades an http to websocket connection */
var upgrader = websocket.Upgrader{
    ReadBufferSize:  1024,
    WriteBufferSize: 1024,
}

/* Keeps a websocket connection alive */
func keepAlive(c *websocket.Conn, timeout time.Duration) {
    lastResponse := time.Now()
    c.SetPongHandler(func(msg string) error {
       lastResponse = time.Now()
       return nil
   })

   go func() {
     for {
        err := c.WriteMessage(websocket.PingMessage, []byte("keepalive"))
        if err != nil {
            return 
        }   
        time.Sleep(timeout/2)
        if(time.Now().Sub(lastResponse) > timeout) {
            c.Close()
            return
        }
    }
  }()
}

/* Continually read messages -- supposed to keep alive? */
func readLoop(c *websocket.Conn) {
    for {
        if _, _, err := c.NextReader(); err != nil {
            c.Close()
            break
        }
    }
}

/* Handle clients as they're passed from ws_acceptor */
func ws_handler(w http.ResponseWriter, r *http.Request) {
  	conn, err := upgrader.Upgrade(w, r, nil)
  	upgrader.CheckOrigin = func(r *http.Request) bool { return true }
    if err != nil {
        log.Println(err)
        return
    }
    
    // Keep socket open for up to a week(?)
	go keepAlive(conn, 168 * time.Hour)
    
    for {
	    _, p, err := conn.ReadMessage()
	    //messageType, p, err := conn.ReadMessage()
	    if err != nil {
	        log.Println(err)
	        return
	    }
	    //log.Println(p, messageType)
		log.Println(string(p))
    }
}

/* Accept websocket connections as they come in */
func ws_acceptor() {
	http.HandleFunc("/socket", ws_handler)
    err := http.ListenAndServe(":8665", nil)
    if err != nil {
        panic("ListenAndServe: " + err.Error())
    }
}
