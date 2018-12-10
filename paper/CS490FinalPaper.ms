.LP
MCA66 Audio Controller
.LP
Sean Hinchee
.LP
Dr. Jeremy Sheaffer
.LP
Iowa State University
.LP
Fall 2018
.LP
.LP
Abstract	1
.LP
30,000 leagues under the C	1
.LP
Go forth and prosper	4
.LP
Socket is to WebSocket as Java is to JavaScript	5
.LP
Conclusion	7
.LP
References	8
.LP
.SH
Abstract
.LP
.LP
Over the course of the semester with Dr. Sheaffer [1] I have developed
a revised interface to the mca66 audio controller which is intended to
be utilized via a Raspberry Pi. Generally, the final goal was to work towards
developing a functional visual, remote, user interface application for
said controller.
.SH
30,000 leagues under the C
.LP
.LP
The first development hurdle was that of providing an abstraction at the
control program level which allows a user to easily perform all functions
available to the audio controller manually.
.LP
.LP
The codebase was split into two portions, a library named libmca66 and
a control program frontend named cntl. Both projects are developed in C
with adherence to roughly the GNU99 standard.
.LP
.LP
Routines which interact directly with the controller are isolated to libmca66
with a handful of exported functions in mca66.h:
.LP
.LP
process_command() allows the user to call a given command which uses the
provided zone value (if any, a lack of zone is represented as nozone, see
the table below) and a command which are concatenated and passed over the
pre-established serial connection. This function returns 0 on a success.
.LP
.LP
int process_command(zone z, command c);
.LP
.LP
init_audio() is intended to be called before any subsequent calls to process_command() as
it establishes a serial connection and performs necessary setup command
calls, if any. This function returns 0 on a success.
.LP
.LP
int init_audio(char* device, int debugf);
.LP
.LP
In addition to these functions, the command and zone types are exported
as typedef entries to enum command and enum zone respectively. The tables
for these types are as follows:
.LP
.LP
enum zone {
.LP
	nozone,		// 0
.LP
	zone1 = 2,	// 2
.LP
	zone2 = 26,
.LP
	zone3 = 50,
.LP
	zone4 = 74,
.LP
	zone5 = 98,
.LP
	zone6 = 122,
.LP
};
.LP
.LP
enum command {
.LP
	cmd_all_on,
.LP
	cmd_all_off,
.LP
	cmd_set_input_ch1 = 0,
.LP
	cmd_set_input_ch2,
.LP
	cmd_set_input_ch3,
.LP
	cmd_set_input_ch4,
.LP
	cmd_set_input_ch5,
.LP
	cmd_set_input_ch6,
.LP
	cmd_volume_up,
.LP
	cmd_volume_down,
.LP
	cmd_power_on,
.LP
	cmd_power_off,
.LP
	cmd_mute_toggle,
.LP
	cmd_bass_up,
.LP
	cmd_bass_down,
.LP
cmd_treble_up,
.LP
cmd_treble_down,
.LP
	cmd_balance_right,
.LP
	cmd_balance_left,
.LP
	cmd_part_mode_input_ch1,
.LP
	cmd_part_mode_input_ch2,
.LP
	cmd_part_mode_input_ch3,
.LP
	cmd_part_mode_input_ch4,
.LP
	cmd_part_mode_input_ch5,
.LP
	cmd_part_mode_input_ch6,
.LP
	cmd_query_zone_state
.LP
};
.LP
.LP
Cntl is a proof of concept program which utilizes the libmca66 library
to control the mca66 via a simple language implemented manually inside
cntl. Generally, the language adheres to the following format:
.LP
.LP
Power all zones on:
.LP
.LP
$ ./cntl POWER 0 ON
.LP
.LP
Unmute zone 2:
.LP
.LP
$ ./cntl MUTE 2 OFF
.LP
.LP
Query the state of zone 1:
.LP
.LP
$ ./cntl QUERY 1
.LP
.LP
Set channel 2 as the input channel for zone 1:
.LP
.LP
$ ./cntl INPUT 1 2
.LP
.LP
Cntl has a two options available from the command line and necessitates
the provision of a command to function. The -D option enables chatty/debug
output to be displayed, which is disabled by default. The -t option allows
the specification of a serial device other than /dev/ttyUSB0 to be passed
to libmca66.
.LP
.LP
Although this rework from a tool named mca66cntl to cntl featured a quality
of life improvement of not having to recompile the program to change which
operation it could perform, it was not sufficient as an interface for a
user interface to utilize remotely. As such, a socketed solution was developed
which is described in the following section.
.SH
Go forth and prosper
.LP
.LP
In regards to satisfying the desire for a remote user interface for the
controller, a socketed solution was developed. Go [8] was selected as the
language of choice due to its ease of use in regards to creating programs
which perform asynchronous actions, operate on socketed connections, and
Go's ability to create statically linked and cross-compiled binaries.
.LP
.LP
The program mca66d is a Go program which builds to a statically linked
binary which can be cross-compiled for a Raspberry Pi. Mca66d has two dependencies,
Gorilla's WebSocket library [7] and Takama's Daemon library [4], both of
which are vendored with the cs490 source distribution [3]. These libraries
provide functionality for WebSocket connections and Systemd-compatible
daemon functionality, respectively and do not pull in further dependencies.
.LP
.LP
Mca66d listens for the same control language as cntl and calls cntl with
the proper arguments and writes an error or "Ok." over the connection.
Initially, the daemon was only able to handle tcp socket connections, but
when it was determined that a web interface would an end goal of the project,
a WebSocket handler and a slight rework of the code base resulted. Commands,
errors, etc. are logged to /var/log/mca66.log. The daemon is intended to
be installed as a system service and as such, expects to have the relevant
permissions for such a service.
.LP
.LP
The command language implementation is similar to, but is implemented independently
of, cntl's command language and is compartmentalized inside the docmd() function
within the main.go file:
.LP
.LP
func docmd(parts []string) (out []byte, err error) {
.LP
.LP
By default, the daemon listens on the port tcp/8664 for socket connections
and tcp/8665 for WebSocket connections. As the daemon is run automatically
and is installed as a static binary, there are no command line flags. The
daemon could easily be extended to support a configuration file, but presently
must be recompiled and reinstalled to change options.
.LP
.LP
Thanks to Takama's daemon library, mca66d supports a variety of daemon
commands:
.LP
.LP
	if len(os.Args) > 1 {
.LP
		command := os.Args[1]
.LP
		switch command {
.LP
			case "install":
.LP
				return (*s).Install()
.LP
			case "remove":
.LP
				return (*s).Remove()
.LP
			case "start":
.LP
				return (*s).Start()
.LP
			case "stop":
.LP
				return (*s).Stop()
.LP
			case "status":
.LP
				return (*s).Status()
.LP
			default:
.LP
				return usage, nil
.LP
}
.LP
}
.LP
.LP
These can be called in the standard GNU manner as follows:
.LP
.LP
To stop the daemon:
.LP
.LP
$ sudo service mca66d stop
.LP
.LP
To install the daemon:
.LP
.LP
$ sudo ./mca66d install
.LP
.LP
To uninstall the daemon:
.LP
.LP
$ sudo ./mca66d remove
.LP
.LP
Mca66d is a reasonably complete piece of software for its service, acting
as a frontend for calling cntl over a remote, or locally socketed, connection.
Moving forward with the project, a user interface had to be selected that
targeted mca66d as to allow the visual interface to be run on a remote
device, if desired.
.SH
Socket is to WebSocket as Java is to JavaScript
.LP
.LP
After a period of deliberation which involved examining the existing user
interface written by Dr. Sheaffer in C targeting GTK 2.x and documentation
for Android applications, the conclusion was reached that it would be easier
to write a self-contained web application with limited dependencies than
it would be to maintain two separate user interface code bases.
.LP
.LP
Bootstrap [5] was chosen as the library to add tiling functionality to
the interface and JQuery [6] was selected as the library to provide various
quality of life and asynchronous operations to the environment. These dependencies
are vendored and the web page does not require a functioning internet connection
to connect. Bootstrap in particular allows the web interface to easily
adapt to different sized screens automatically, making it ideal for phones
and tablets as potential control points for the user interface.
.LP
.LP
The WebSocket implementation proved to be a distinct challenge to reach
functionality that was similar to the socket interface previously implemented
in the daemon. That is, it was desired to have Read-Evaluate-Print-Loop-like
functionality within the connection. Commands are to be sent, evaluated,
and the response should be available for the client.
.LP
.LP
Mca66d provides an http server which immediately attempts to promote [9]
the connection to WebSocket, a superset of the http protocol. Connections
are not encrypted as no sensitive information is transacted, and a heartbeat
is established prior to successful connection.
.LP
.LP
The heartbeat problem proved to be a roadblock for around half a week until
a comfortable solution was reached. On the client side, most mainstream
web browsers will adhere to the WebSocket RFC [10] and close connections
if a heartbeat fails. Implementation details include a keep-alive time
limit on the server of a, potentially temporary, 168 hour period. This
timer is reset each time the page is refreshed as the WebSocket is re-connected.
.LP
.LP
The keepAlive() function handles this and utilizes an asynchronous closure
that runs in its own goroutine [11] which responds to Ping messages sent
from the browser client with a Pong message as per below:
.LP
.LP
func keepAlive(c *websocket.Conn, timeout time.Duration) {
.LP
	lastResponse := time.Now()
.LP
	c.SetPongHandler(func(msg string) error {
.LP
	lastResponse = time.Now()
.LP
	return nil
.LP
	})
.LP
.LP
	go func() {
.LP
	for {
.LP
		err := c.WriteMessage(websocket.PingMessage, []byte("keepalive"))
.LP
		if err != nil {
.LP
			return
.LP
		}
.LP
		time.Sleep(timeout/2)
.LP
		if(time.Now().Sub(lastResponse) > timeout) {
.LP
			c.Close()
.LP
			return
.LP
		}
.LP
	}
.LP
	}()
.LP
}
.LP
.LP
Once established, however, the WebSocket connection is stable and is able
to read/write back and forth over the connection. There is a debug text
field available in the web interface which takes commands in the same language
that mca66d expects. This text is cleared when sent and is written as a
series of bytes to ease the server-side listener implementation. At present,
no JSON is transferred between the server and client, keeping complexity
relatively low.
.LP
.LP
Presently, the web interface is very primitive and lacks control functionality
outside of the debug console. Configuration is set inside the monolithic
index.html file that compartmentalizes the interface as hard values. These
values could be edited via a web REPL or by editing the file itself and
refreshing. Either way, configuration could be more dynamic, but is existent.
.SH
Conclusion
.LP
.LP
Although a fully functional user interface was not achieved, the foundation
for its successful production has been laid and such a project could be
finalized in an efficient, and relatively straightforward, manner. I covered
a variety of new skills over the course of the semester which included
diving into web development for the first time involving any depth to its
exploration and worked on a daemon service which I had not previously worked
with to any major extent.
.LP
.LP
Several pursuits over the course of the semester did not pan out. Among
these include, but are not limited to:
.LP
.IP \(bu
A control language implementation shared between the controller and daemon
written in yacc(1) and lex(1).
.LP
.IP \(bu
This was scrapped after experimenting with lex(1) and yacc(1) for around
a week and a half and the infrastructure was deemed too time consuming
to implement idiomatically.
.LP
.IP \(bu
An Android mobile application as a user interface.
.LP
.IP \(bu
This was replaced with the pursuit of the WebSocket-based web user interface.
.LP
.IP \(bu
A universal web interface with no dependencies.
.LP
.IP \(bu
Development of a dependency-less web interface proved to be a very taxing
undertaking, and the desired dependencies were selected and vendored appropriately
as a point of compromise. Total dependencies to date number two.
.LP
.LP
.LP
Overall, I learned a great deal and would be more than happy to come back
and see the project to further development and refinement.
.SH
References
.LP
.LP
[1] Dr. Jeremy Sheaffer -- https://www.info.iastate.edu/individuals/info/258507/Sheaffer-Jeremy
.LP
.LP
[2] The provided code samples from the mca66 OEM -- https://github.com/henesy/cs490/tree/master/htd
.LP
.LP
[3] Sean Hinchee's CS490 repository -- https://github.com/henesy/cs490
.LP
.LP
[4] Takama's daemon library for Go -- https://github.com/takama/daemon
.LP
.LP
[5] JavaScript Bootstrap tiling interface library -- http://getbootstrap.com/
.LP
.LP
[6] JavaScript JQuery -- https://developers.google.com/speed/libraries/
.LP
.LP
[7] Gorilla's WebSocket library for Go -- https://github.com/gorilla/websocket
.LP
.LP
[8] The Go Programming Language -- http://golang.org/
.LP
.LP
[9] WebSocket promotion -- https://developer.mozilla.org/en-US/docs/Web/HTTP/Protocol_upgrade_mechanism
.LP
.LP
[10] WebSocket RFC -- https://tools.ietf.org/html/rfc6455
.LP
.LP
[11] Goroutines -- https://tour.golang.org/concurrency/1
.LP

