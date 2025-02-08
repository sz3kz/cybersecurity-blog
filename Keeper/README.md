# Keeper
**Keeper** is an Easy HackTheBox machine. It hosts a ticket website set up without changing default credentials. After a bit of looking, ssh credentials for a normal user can be found. SSHing into the machine reveals a zip file containing a dump of a vulnerable KeePass application, allowing us to recover the key for the database. In the database the root's putty ssh key file, which after translating into OpenSSH format gives us root access to the machine.

## Enumerating ports
After making sure I can access the machine:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Keeper]$ IP="10.10.11.227"
(sz3kz@kali)~{tun0:10.10.14.19}~[Keeper]$ ping -c 5 $IP
PING 10.10.11.227 (10.10.11.227) 56(84) bytes of data.
64 bytes from 10.10.11.227: icmp_seq=1 ttl=63 time=163 ms
64 bytes from 10.10.11.227: icmp_seq=2 ttl=63 time=142 ms
64 bytes from 10.10.11.227: icmp_seq=3 ttl=63 time=374 ms
64 bytes from 10.10.11.227: icmp_seq=4 ttl=63 time=197 ms
64 bytes from 10.10.11.227: icmp_seq=5 ttl=63 time=264 ms

--- 10.10.11.227 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 4009ms
rtt min/avg/max/mdev = 142.216/227.800/373.661/83.746 ms
```

I conducted a quick scan of the machines ports and a deeper scan of the ports that were detected open.

The quick scan:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Keeper]$ sudo nmap -sS -T5 -O -oN quick-scan.txt $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-02-05 15:48 EST
Nmap scan report for keeper.htb (10.10.11.227)
Host is up (0.11s latency).
Not shown: 998 closed tcp ports (reset)
PORT   STATE SERVICE
22/tcp open  ssh
80/tcp open  http
Device type: general purpose
Running: Linux 4.X|5.X
OS CPE: cpe:/o:linux:linux_kernel:4 cpe:/o:linux:linux_kernel:5
OS details: Linux 4.15 - 5.19
Network Distance: 2 hops

OS detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 3.72 seconds
```

The deeper scan:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Keeper]$ sudo nmap -sV -sC -T5 -p 22,80 -oN version-scan.txt $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-02-05 15:49 EST
Nmap scan report for keeper.htb (10.10.11.227)
Host is up (0.18s latency).

PORT   STATE SERVICE VERSION
22/tcp open  ssh     OpenSSH 8.9p1 Ubuntu 3ubuntu0.3 (Ubuntu Linux; protocol 2.0)
| ssh-hostkey:
|   256 35:39:d4:39:40:4b:1f:61:86:dd:7c:37:bb:4b:98:9e (ECDSA)
|_  256 1a:e9:72:be:8b:b1:05:d5:ef:fe:dd:80:d8:ef:c0:66 (ED25519)
80/tcp open  http    nginx 1.18.0 (Ubuntu)
|_http-title: Site doesn't have a title (text/html).
|_http-server-header: nginx/1.18.0 (Ubuntu)
Service Info: OS: Linux; CPE: cpe:/o:linux:linux_kernel

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 11.83 seconds
```

Summary of the scans:
* target OS -> Linux 4.15 - 5.19
* no firewall (closed tcp ports)
* 22 -> ssh
  * version not vulnerable
* 80 -> http
  * nginx/1.18
  * vulnerable to crashes -> not helpful for rooting
