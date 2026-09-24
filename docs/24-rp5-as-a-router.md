# Volume 24: RP5 as a Router

**AP, WAN, NAT, DHCP, and DNS**

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

## What You'll Learn

- How the TELESCREEN is a router, not just a camera
- The `eth0` (WAN) + `wlan0` (AP) topology
- The real software stack: `hostapd`, `dnsmasq`, `nftables`
- How to prove it with a client and a lease

## 1. The Premise

The TELESCREEN **routes**. It is the neighbourhood's Wi-Fi access point and gateway, so
every packet a client sends crosses Party (or your) hardware. This volume builds that.

## 2. Topology

```
        (WAN)                      TELESCREEN (RP5)                 (LAN / AP)
   [upstream]  <--- eth0 --->  [ routing + NAT ]  <--- wlan0 --->  [ clients ]
                                     |
                                     +--- teled (camera + beacon)
```

| interface | role | connects to |
| --------- | ---- | ----------- |
| `eth0` | **WAN / uplink** | the upstream network |
| `wlan0` | **LAN / AP** | client devices |

The RP5's on-board CYW43455 is AP-capable, so a stock RP5 does Wi-Fi + routing with no
extra radio.

## 3. The Stack

| job | tool |
| --- | ---- |
| access point | `hostapd` |
| DHCP + DNS | `dnsmasq` |
| NAT / firewall | `nftables` |
| interface config | `ip` / `ifconfig` |

## 4. Bring It Up

```bash
# 1) the AP
hostapd /etc/hostapd.conf -B            # wlan0 becomes the AP

# 2) DHCP + DNS
dnsmasq --interface=wlan0 --dhcp-range=192.168.50.10,192.168.50.200,12h

# 3) NAT: forward wlan0 -> eth0
nft add table ip nat
nft add chain ip nat postrouting '{ type nat hook postrouting priority 100; }'
nft add rule  ip nat postrouting oifname "eth0" masquerade

# 4) WAN address (DHCP client on eth0)
udhcpc -i eth0
```

## 5. Prove It (the graded step)

On a phone or laptop:

1. Join the SSID `TELESCREEN-XXXX`.
2. Confirm it received an IP in the AP range (a `dnsmasq` lease).
3. Load a page (traffic NATs out `eth0`).

If the client got a lease and reached the WAN, the router works. This is the "router
proof" in `CTF-XX-R.md`.

## 6. Why This Is the Attack Surface

Because the device routes, it sees the social graph: who associates, when, and to what.
That is the surveillance premise. It is also why the device is worth attacking:
compromise the router and you compromise every client behind it.

## 7. The Defensive Shape

A defended TELESCREEN would:

- isolate clients from each other (AP isolation);
- not route by default, or route only with consent;
- log nothing beyond what operations require.

## Labs

1. Bring up the AP, DHCP, and NAT; show a client obtaining a lease.
2. Capture a client packet on `eth0` and show it was NATed.
3. Explain why a single radio can be both STA and AP only awkwardly.
4. Name one way to isolate clients from each other.

## Reference

- `hostapd` / `dnsmasq` / `nftables` documentation
- `docs/13` (network bring-up), `docs/25` (camera), `docs/28` (blue team)
