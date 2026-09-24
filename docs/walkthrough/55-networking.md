# Walkthrough 55: Networking Fundamentals

**the network a firmware engineer must know**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## Why Networking

A camera is a **network device**. Its bugs are reachable over the network; its privacy story
is a network story. This volume is the network knowledge the rest of the course assumes.

## 1. The Stack

```
+-----------------------------------+
| application   : RTSP, HTTP, ONVIF |
+-----------------------------------+
| transport     : TCP / UDP         |
+-----------------------------------+
| network       : IP                |
+-----------------------------------+
| link          : Ethernet / Wi-Fi  |
+-----------------------------------+
```

The lab's surfaces live at the **application** layer, carried over TCP/UDP.

## 2. IP and Addresses

| term | meaning |
| ---- | ------- |
| IP address | a host's layer-3 address |
| netmask | which bits are the network |
| gateway | where to send non-local traffic |
| private range | `10/8`, `172.16/12`, `192.168/16` |

The lab uses `192.168.50.0/24` for the AP.

## 3. TCP vs UDP

| | TCP | UDP |
| - | --- | --- |
| connection | yes | no |
| reliability | ordered, retransmit | best-effort |
| used by | HTTP, RTSP, ONVIF | DNS, SSDP, the beacon |

The beacon is **UDP**: a loss is fine; a periodic retry covers it. That also makes it easy
to detect by interval (`docs/28`).

## 4. Ports (the lab)

| port | service |
| ---- | ------- |
| 554 | RTSP |
| 80 | HTTP |
| 443 | HTTPS |
| 67/68 | DHCP |
| 53 | DNS |
| 1900 | SSDP |
| 5353 | mDNS |

`ss -lntup` on the RP5 lists what the device actually listens on.

## 5. NAT

NAT rewrites the source address of outbound packets so many LAN hosts share the WAN address.

```
client 192.168.50.20:40000 -> 1.1.1.1:80
   NAT
<wan-ip>:53000            -> 1.1.1.1:80
```

A camera behind NAT cannot be reached inbound unless the router forwards a port. That is why
vendors use a **relay** (the P2P class) or **UPnP** to punch a hole (`docs/walkthrough/24`).

## 6. DHCP

A client gets an address, mask, gateway, and DNS from a server:

```
DISCOVER -> OFFER -> REQUEST -> ACK
```

On the RP5, `dnsmasq` is the server for the AP (`docs/24`).

## 7. DNS

Names become addresses. The device resolves the vendor's names to reach the cloud. A
**detection** angle: a device that resolves and contacts a fixed name is beaconing-friendly
(`docs/28`).

## 8. The Handshake (TCP)

```
SYN -> SYN/ACK -> ACK        (connect)
... data ...
FIN -> ACK -> FIN -> ACK     (close)
```

A `CLOSE_WAIT` flood is a resource-exhaustion signal - and the lab's watchdog reacts to it
(the reboot-DoS class, `docs/28`).

## 9. The Traffic to Watch

| direction | what | why |
| --------- | ---- | --- |
| inbound | a request to 80/554 | the attack surface |
| outbound | the beacon | the privacy story |
| outbound | DHCP/DNS | the router's job |
| outbound | the cloud relay | the vendor path |

## 10. Capturing

```bash
# everything but noise, with timestamps
sudo tcpdump -i eth0 -n -ttt 'not arp and not port 22' | head
# only the beacon
sudo tcpdump -i eth0 -n 'udp and dst host <collector>' | head
# write a pcap, open in Wireshark
sudo tcpdump -i eth0 -w cap.pcap
```

## 11. The Model You Carry

```
device --- (AP) --- clients
   |
   +--- (WAN) --- upstream --- vendor relay
```

Every IP device is on some such diagram. Draw it, mark the boundaries, and the attack
surface is obvious.

## Exercises

1. Run `ss -lntup` and match each port to a service.
2. Capture the DHCP handshake and label each packet.
3. Capture the beacon and measure its interval and size.
4. Draw the network diagram for your own LAN and mark the device.

## Reference

- `docs/13`, `docs/24`, `docs/28`, `docs/walkthrough/20`, `docs/walkthrough/24`
