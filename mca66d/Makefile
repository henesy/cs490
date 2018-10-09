build: mca66d.go 
	GOPATH=$$(pwd) go build mca66d.go 

install: 
	sudo ./mca66d install
	sudo service mca66d start

uninstall:
	sudo service mca66d stop
	sudo ./mca66d remove

clean: 
	rm mca66d
	rm testclient

test: testclient.go
	go build testclient.go

rebuild: uninstall build install

