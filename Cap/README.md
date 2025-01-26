# Cap
**Cap** is a Linux machine serving a web app, relying on url-based indentification, allowing for trivial manipulation and extraction of an adjacent user's network traffic capture. Analysis of the traffic reveals valid SSH credentials. Enumeration of the user's environment leads us to a program with root-aquiring abilities which can be leveraged to escalate privileges to root.

## Port enumeration
Fistly, I needed to know what I was working with - what services were running on the system.

I conducted 2 scans:
* quick - general port scan, find which ports are open;
* version - version and nmap default scripts scan for finding the version and base access I had on the services.
