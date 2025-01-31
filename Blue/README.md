# Blue
**Blue** is an easy HackTheBox Windows box. Its simplicity lies in an SMBv1 service, vulnerable to the [EternalBlue](https://nordvpn.com/blog/what-is-eternalblue/) exploit. 

## Enumerating ports
Because of my impatience I typically conduct 2 scans:
* quick scan, which gives me initial info about the ports open on the machine
* version scan, which informs me of the versions of software on the machine

### Pinging for confirmation
After spawning the machine I pinged it first to confirm it being accessible on the network:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ ping -c 5 $IP
PING 10.10.10.40 (10.10.10.40) 56(84) bytes of data.
64 bytes from 10.10.10.40: icmp_seq=1 ttl=127 time=113 ms
64 bytes from 10.10.10.40: icmp_seq=2 ttl=127 time=121 ms
64 bytes from 10.10.10.40: icmp_seq=3 ttl=127 time=191 ms
64 bytes from 10.10.10.40: icmp_seq=4 ttl=127 time=176 ms
64 bytes from 10.10.10.40: icmp_seq=5 ttl=127 time=188 ms

--- 10.10.10.40 ping statistics ---
5 packets transmitted, 5 received, 0% packet loss, time 4002ms
rtt min/avg/max/mdev = 112.942/157.924/191.158/33.790 ms
```

### Scanning

First I fished for open ports:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ sudo nmap -sS -T5 -O -oN scan-quick.txt $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-01-31 05:50 EST
Nmap scan report for 10.10.10.40
Host is up (0.11s latency).
Not shown: 991 closed tcp ports (reset)
PORT      STATE SERVICE
135/tcp   open  msrpc
139/tcp   open  netbios-ssn
445/tcp   open  microsoft-ds
49152/tcp open  unknown
49153/tcp open  unknown
49154/tcp open  unknown
49155/tcp open  unknown
49156/tcp open  unknown
49157/tcp open  unknown
Device type: general purpose
Running: Microsoft Windows 2008|7|Vista|8.1
OS CPE: cpe:/o:microsoft:windows_server_2008:r2 cpe:/o:microsoft:windows_7 cpe:/o:microsoft:windows_vista cpe:/o:microsoft:windows_8.1
OS details: Microsoft Windows Vista SP2 or Windows 7 or Windows Server 2008 R2 or Windows 8.1
Network Distance: 2 hops

OS detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 13.00 seconds
```


With this initial information I did a more thorough scan:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ sudo nmap -sS -T5 -sV -sC -p 135,139,445,49152,49153,49154,49155,49156,49157 -oN scan-version.txt $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-01-31 06:00 EST
Nmap scan report for 10.10.10.40
Host is up (0.20s latency).

PORT      STATE SERVICE      VERSION
135/tcp   open  msrpc        Microsoft Windows RPC
139/tcp   open  netbios-ssn  Microsoft Windows netbios-ssn
445/tcp   open  microsoft-ds Windows 7 Professional 7601 Service Pack 1 microsoft-ds (workgroup: WORKGROUP)
49152/tcp open  msrpc        Microsoft Windows RPC
49153/tcp open  msrpc        Microsoft Windows RPC
49154/tcp open  msrpc        Microsoft Windows RPC
49155/tcp open  msrpc        Microsoft Windows RPC
49156/tcp open  msrpc        Microsoft Windows RPC
49157/tcp open  msrpc        Microsoft Windows RPC
Service Info: Host: HARIS-PC; OS: Windows; CPE: cpe:/o:microsoft:windows

Host script results:
| smb2-security-mode:
|   2:1:0:
|_    Message signing enabled but not required
|_clock-skew: mean: 4s, deviation: 2s, median: 2s
| smb-os-discovery:
|   OS: Windows 7 Professional 7601 Service Pack 1 (Windows 7 Professional 6.1)
|   OS CPE: cpe:/o:microsoft:windows_7::sp1:professional
|   Computer name: haris-PC
|   NetBIOS computer name: HARIS-PC\x00
|   Workgroup: WORKGROUP\x00
|_  System time: 2025-01-31T11:01:18+00:00
| smb-security-mode:
|   account_used: guest
|   authentication_level: user
|   challenge_response: supported
|_  message_signing: disabled (dangerous, but default)
| smb2-time:
|   date: 2025-01-31T11:01:19
|_  start_date: 2025-01-31T10:47:07

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 74.79 seconds
```

[msrpc](https://superuser.com/questions/616098/what-is-rpc-and-why-is-it-so-important) - Microsoft Windows RPC, this service is largely not so important to me as it is not really used to host anything. To my mind, MS Windows RPC is a service that allows applications on the Windows machine to communicate with eachother. While RPC can also be used to send commands to the machine, which would warrant a code execution check, I had my money on the other services present.

[netbios-ssn](https://medium.com/@chavanyashwardhan/difference-between-netbios-smb-1cd74ec02fdd) - From my understanding, it is a windows service that allows another machine to connect to the one running it. It gives every computer a NetBIOS name similar to their hostname(here HARIS-PC\x00). With NetBIOS, a computer can refer to a target computer via its NetBIOS name, which by resultion is translated into an IP address. This can also host an SMB service as _SMB over NetBIOS_.

[microsoft-ds](https://en.wikipedia.org/wiki/Server_Message_Block) - this is an SMB service that allows sharing files on the network. Seeing this service on 445 means SMB can be run _over TCP/IP_ directly also.

From the scans I gathered the following:
* there is no firewall present (quick scann detected closed ports)
* OS: Windows 7 SP 1
* hostname:"haris-PC"
* SMB file sharing is enabled on the system

## Enumerating SMB
I predicted the SMB server would be the most important thing to test, so I tested it first.

I scanned the _SMB over TCP/IP_ for the version of SMB on the machine:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ nmap -p 445 --script smb-protocols $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-01-31 06:13 EST
Nmap scan report for 10.10.10.40
Host is up (0.19s latency).

PORT    STATE SERVICE
445/tcp open  microsoft-ds

Host script results:
| smb-protocols:
|   dialects:
|     NT LM 0.12 (SMBv1) [dangerous, but default]
|     2:0:2
|_    2:1:0

Nmap done: 1 IP address (1 host up) scanned in 4.82 seconds
```

Seeing SMBv1, I immediately remebered the [EternalBlue](https://en.wikipedia.org/wiki/EternalBlue) exploit this version might be suseptible to.
This would also line up with the OS version, as [the Microsoft Bulletin covering EternalBlue](https://learn.microsoft.com/en-us/security-updates/SecurityBulletins/2017/ms17-010) deems similar if not identical versions of windows to be vulnerable.
I ran another nmap scan, this time check if this vulnerability is exploitable here:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ nmap -p 445 --script smb-vuln-ms17-010 $IP
Starting Nmap 7.95 ( https://nmap.org ) at 2025-01-31 06:16 EST
Nmap scan report for 10.10.10.40
Host is up (0.23s latency).

PORT    STATE SERVICE
445/tcp open  microsoft-ds

Host script results:
| smb-vuln-ms17-010:
|   VULNERABLE:
|   Remote Code Execution vulnerability in Microsoft SMBv1 servers (ms17-010)
|     State: VULNERABLE
|     IDs:  CVE:CVE-2017-0143
|     Risk factor: HIGH
|       A critical remote code execution vulnerability exists in Microsoft SMBv1
|        servers (ms17-010).
|
|     Disclosure date: 2017-03-14
|     References:
|       https://blogs.technet.microsoft.com/msrc/2017/05/12/customer-guidance-for-wannacrypt-attacks/
|       https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2017-0143
|_      https://technet.microsoft.com/en-us/library/security/ms17-010.aspx

Nmap done: 1 IP address (1 host up) scanned in 2.99 seconds
```

The machine is vulnerable!
