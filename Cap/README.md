# Cap
**Cap** is a HackTheBox Linux machine serving a web app, relying on url-based indentification, allowing for trivial manipulation and extraction of an adjacent user's network traffic capture. Analysis of the traffic reveals valid SSH credentials. Enumeration of the user's environment leads us to a program with root-aquiring abilities which can be leveraged to escalate privileges to root.

## Enumerating ports
Fistly, I needed to know what I was working with - what services were running on the system.

I conducted 2 scans:
* quick - general port scan, find which ports are open;
* version - version and nmap default scripts scan for finding the version and base access I had on the services.

### Scanning - quick

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

As shown, the server is hosting ftp, ssh and an http service.

ASIDE: What I also deduced, is that the server is most probably not behind a firewall, in contrast to [Lame](https://github.com/sz3kz/cybersecurity-blog/tree/main/Lame).

### Scanning - version

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Cap]$ sudo nmap -sS -sV -sC -p 21,22,80 -T5 -oN version-scan.txt $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-01-25 14:50 EST
Nmap scan report for 10.10.10.245
Host is up (0.20s latency).

PORT   STATE SERVICE VERSION
21/tcp open  ftp     vsftpd 3.0.3
22/tcp open  ssh     OpenSSH 8.2p1 Ubuntu 4ubuntu0.2 (Ubuntu Linux; protocol 2.0)
| ssh-hostkey:
|   3072 fa:80:a9:b2:ca:3b:88:69:a4:28:9e:39:0d:27:d5:75 (RSA)
|   256 96:d8:f8:e3:e8:f7:71:36:c5:49:d5:9d:b6:a4:c9:0c (ECDSA)
|_  256 3f:d0:ff:91:eb:3b:f6:e1:9f:2e:8d:de:b3:de:b2:18 (ED25519)
80/tcp open  http    Gunicorn
|_http-server-header: gunicorn
|_http-title: Security Dashboard
Service Info: OSs: Unix, Linux; CPE: cpe:/o:linux:linux_kernel

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 12.43 seconds
```

From the scan I discovered versions of the services:
* `vsFTPd 3.0.3` is vulnerable to **CVE-2021-30047** - a remote DOS attack. This does not help in owning the system.
* While `OpenSSH 8.2p1` has a list of [vulnerabilities](https://nvd.nist.gov/vuln/search/results?cves=on&cpe_version=cpe:/a:openbsd:openssh:8.2p1), they typically already require access to the system or rely on an administrator connection to be occuring at the time of exploit. Keeping these in mind, I moved on.
* Did not find anything interesting about the `Gunicorn` http server.

## Enumerating services

### FTP
Normally, when anonymous access is allowed on a FTP server, nmap will detect that. In order to confirm disallowed anonymous access, I tried to manually connect to vsFTPd:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Cap]$ ftp anonymous@$IP
Connected to 10.10.10.245.
220 (vsFTPd 3.0.3)
331 Please specify the password.
Password:
530 Login incorrect.
ftp: Login failed
ftp> exit
221 Goodbye.
```

No anon access confirmed.

### SSH
I really had nothing to work with except maybe some brute-force attacks, so I decided to leave it.
