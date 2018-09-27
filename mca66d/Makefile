daemon: build

build: mca66d.go 
	GOPATH=$$(pwd) go build mca66d.go 

install: daemon
	sudo ./mca66d install

uninstall:
	sudo service mca66d stop
	sudo ./mca66d remove

clean: 
	rm mca66d
