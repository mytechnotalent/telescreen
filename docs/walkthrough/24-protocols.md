# Walkthrough 24: Protocol Deep-Dives

**ONVIF, UPnP, DHCP, and DNS in the lab**

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

## What Extra Protocols a Camera Speaks

Beyond RTSP/HTTP, a real camera speaks **ONVIF** (device discovery/control), **UPnP** (NAT
hole-punching), and the router half speaks **DHCP** and **DNS**. Each is a surface.

## 1. ONVIF (SOAP over HTTP)

ONVIF is **SOAP**: XML envelopes over HTTP, on a device-management port.

```xml
<soap:Envelope xmlns:soap="http://www.w3.org/2003/05/soap-envelope">
  <soap:Body>
    <GetDeviceInformation xmlns="http://www.onvif.org/ver10/device/wsdl"/>
  </soap:Body>
</soap:Envelope>
```

| ONVIF verb | purpose |
| ---------- | ------- |
| `GetDeviceInformation` | model, firmware, serial |
| `GetCapabilities` | what the device supports |
| `GetStreamUri` | **the RTSP URL** |
| `GetProfiles` | stream profiles |

### The security lesson

A real-world family flaw: `GetStreamUri` returns an RTSP URL **with credentials embedded**,
so an unauthenticated `GetStreamUri` leaks them. In the lab, the ONVIF surface is a
teaching point: discovery should not leak secrets, and control operations should require
auth.

```bash
# a discovery probe (WS-Discovery is UDP multicast 239.255.255.250:3702)
# a management call (SOAP POST)
curl -s -X POST http://192.168.50.1/onvif/device_service \
     -H 'Content-Type: application/soap+xml' \
     --data-binary @soap.xml
```

## 2. UPnP (NAT Hole-Punching)

UPnP-IGD lets a device **ask the router to open an inbound port**. That is how a camera
becomes reachable from the WAN **without any port-forwarding by the user**.

```
device -> router: AddPortMapping(ExternalPort=554, InternalPort=554, ...)
router -> device: OK
```

| term | meaning |
| ---- | ------- |
| SSDP | the discovery protocol (UDP 1900) |
| IGD | the port-mapping service |
| `AddPortMapping` | open an inbound hole |

**The security lesson:** automatic inbound holes are a **silent exposure**. A defended
device should not punch holes without consent, and an operator should know that SSDP is
chatty.

```bash
# SSDP discovery
printf 'M-SEARCH * HTTP/1.1\r\nHOST:239.255.255.250:1900\r\nMAN:"ssdp:discover"\r\nMX:1\r\nST:ssdp:all\r\n\r\n' | nc -u 239.255.255.250 1900
```

## 3. DHCP (the Router Half)

`dnsmasq` hands out leases on the AP interface (`docs/24`):

```
client -> DHCPDISCOVER (broadcast)
router -> DHCPOFFER
client -> DHCPREQUEST
router -> DHCPACK   (lease: IP, netmask, gateway, DNS)
```

Watch a lease:

```bash
# on the RP5
journalctl -u dnsmasq -f      # or the daemon's log
cat /var/lib/misc/dnsmasq.leases
```

A lease is the "router proof" in `CTF-XX-R.md`: the client got an address from the device.

## 4. DNS (the Router Half)

`dnsmasq` also resolves for clients. Two behaviours to know: it **caches** upstream
answers, and it can be told to resolve a name locally. The **detection** angle (`docs/28`):
a device that resolves and connects to a fixed external name is beaconing-friendly.

```bash
dig @192.168.50.1 example.com
```

## 5. The Discovery Surface (SSDP + mDNS)

Beyond UPnP, devices often answer **mDNS** (`_http._tcp.local`) as well. Every discovery
protocol is **unauthenticated by design** - that is the point of discovery - so the lesson
is: discovery must expose only what is safe to expose.

## 6. The Protocol Map

| protocol | port | auth | risk |
| -------- | ---- | ---- | ---- |
| RTSP | 554 | none by default | stream + parser bug |
| HTTP | 80 | weak/empty | remote root |
| ONVIF | mgmt port | weak | info leak, control |
| SSDP/UPnP | 1900 | none | silent inbound holes |
| DHCP | 67/68 | none | the router's job |
| DNS | 53 | none | the router's job |
| mDNS | 5353 | none | discovery leak |

## Exercises

1. Send an SSDP `M-SEARCH` and record the device's response.
2. Post an ONVIF `GetCapabilities` and read the reply.
3. Capture a DHCP four-way handshake on the AP and label each packet.
4. Resolve a name through the device and confirm the cache.
5. List every port the device listens on (`ss -lntup` on the RP5) and match the table.

## Reference

- `docs/13`, `docs/24`, `docs/28`
- ONVIF Core Specification; UPnP IGD; RFC 2131 (DHCP); RFC 1035 (DNS)
