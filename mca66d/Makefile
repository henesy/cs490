build: main.go tcp_handler.go ws_handler.go
	GOPATH=$$(pwd) go build 

install: 
	sudo ./mca66d install
	sudo service mca66d start

uninstall:
	sudo service mca66d stop
	sudo ./mca66d remove

clean: 
	rm mca66d
	rm testclient/testclient

test: testclient.go
	cd testclient
	go build testclient.go

rebuild: uninstall build install

