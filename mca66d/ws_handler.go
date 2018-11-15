package main

import (
	"net/http"
	"log"

	// Extension library to stdlib that handles websockets nicely
	""
)


var upgrader = websocket.Upgrader{
    ReadBufferSize:  1024,
    WriteBufferSize: 1024,
}

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
    
	go readLoop(conn)
    
    for {
	    messageType, p, err := conn.ReadMessage()
	    if err != nil {
	        log.Println(err)
	        return
	    }
	    log.Println(p, messageType)
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
