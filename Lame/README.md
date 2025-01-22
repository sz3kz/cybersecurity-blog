# Lame

"Lame" is an easy machine hosted on HackTheBox. A Linux file-share server with outdated software, resulting in two known vulnerabilities, one of which being exploitable. Let's dive in!

## Server enumeration

After connecting to HackTheBox using the correct .ovpn file and starting the machine, I proceeded to ping the supplied IP address to make sure the server was live.

For convenience sake, I make a habit of assigning the ip address of the machine to a shell variable, that way I can use it without losing freedom of my clipboard.

![Pinging the target for assurance]()

As we can see, the target is indeed live.

I did 2 scans:
 - A quick scan that quickly shows me how many and which ports are open on the machine;
 - A version scan, showing me each detail about a service that nmap could find.

### Quick Scan
![Quickscanning]()

As we can see, there are 4 ports open on the machine. The ftp and ssh ports are self-explanatory. 

The other two - 139 and 445 - are ports characteristic of SMB over NetBIOS and SMB over TCP/IP, respectively. The most important thing is that there exist some SMB service on the machine too.

Although nmap couldn't identify what OS is running on the system, we can see that it is most probably some Linux distro.

For the future I will note that the other ports are labeled by nmap to be "filtered" as in "there is something blocking connection". This will be important a bit later.
