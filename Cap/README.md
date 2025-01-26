# Cap
**Cap** is a Linux machine serving a web app, relying on url-based indentification, allowing for trivial manipulation and extraction of an adjacent user's network traffic capture. Analysis of the traffic reveals valid SSH credentials. Enumeration of the user's environment leads us to a program with root-aquiring abilities which can be leveraged to escalate privileges to root.

## Port enumeration
Fistly, I needed to know what I was working with - what services were running on the system.

I conducted 2 scans:
* quick - general port scan, find which ports are open;
* version - version and nmap default scripts scan for finding the version and base access I had on the services.

### Quick scan

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Cap]$ sudo nmap -sS -T5 -oN quick-scan.txt $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-01-25 14:47 EST
Nmap scan report for 10.10.10.245
Host is up (0.14s latency).
Not shown: 997 closed tcp ports (reset)
PORT   STATE SERVICE
21/tcp open  ftp
22/tcp open  ssh
80/tcp open  http

Nmap done: 1 IP address (1 host up) scanned in 3.70 seconds
```

As shown, the server is hosting ftp, ssh and http service.

What I also deduced, is that the server is most probably not behind a firewall, in contrast to [Lame](https://github.com/sz3kz/cybersecurity-blog/tree/main/Lame).
