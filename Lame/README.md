# Lame

"Lame" is an easy machine hosted on HackTheBox. A Linux file-share server with outdated software, resulting in two known vulnerabilities, one of which being exploitable. Let's dive in!

## Server enumeration

After connecting to HackTheBox using the correct .ovpn file and starting the machine, I proceeded to ping the supplied IP address to make sure the server was live.

For convenience sake, I make a habit of assigning the ip address of the machine to a shell variable, that way I can use it without losing freedom of my clipboard.

![Pinging the target for assurance]()

As we can see, the target is indeed live.

