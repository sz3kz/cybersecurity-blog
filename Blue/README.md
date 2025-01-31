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
