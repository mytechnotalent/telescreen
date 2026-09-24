# Walkthrough 20: Network Protocols

**RTSP, HTTP, and the beacon, on the wire**

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

## The Three Services

| port | protocol | purpose |
| ---- | -------- | ------- |
| 554 | RTSP | camera video |
| 80 | HTTP | web UI + routes |
| (WAN) | UDP beacon | sealed telemetry to the collector |

## 1. RTSP - Real Time Streaming Protocol

RTSP is a **text** protocol (like HTTP) that negotiates a media session. The four methods
you meet:

| method | meaning |
| ------ | ------- |
| `OPTIONS` | what methods does the server support? |
| `DESCRIBE` | give me the session description (SDP) |
| `SETUP` | set up this stream (transport) |
| `PLAY` | start streaming |

A minimal session:

```
OPTIONS rtsp://192.168.50.1/ RTSP/1.0
CSeq: 1

RTSP/1.0 200 OK
CSeq: 1
Public: OPTIONS, DESCRIBE, SETUP, PLAY, TEARDOWN
```

```
DESCRIBE rtsp://192.168.50.1/stream RTSP/1.0
CSeq: 2

RTSP/1.0 200 OK
Content-Type: application/sdp

v=0
m=video 0 RTP/AVP 96
a=rtpmap:96 H264/90000
...
```

### The vulnerability class

A pre-auth RTSP server that parses the **URL** into a fixed buffer is the classic
remote-root bug: an over-long URL overflows a stack buffer before any authentication. The
TELESCREEN lab keeps the *shape* (`rtspauth="n"`) so the lesson lands (`docs/25`,
`docs/14`). The fix: authenticate **before** parsing, and never `strcpy` a URL into a
fixed buffer.

### Capture it

```bash
# connect and issue OPTIONS
printf 'OPTIONS rtsp://127.0.0.1/ RTSP/1.0\r\nCSeq: 1\r\n\r\n' | nc 127.0.0.1 554
# or watch the stream in VLC
vlc rtsp://127.0.0.1:554/stream
```

## 2. HTTP - the Web UI and the Routes

The web server is the application (`docs/14`). A config request:

```
GET /param.cgi?cmd=getlanguage HTTP/1.1
Host: 192.168.50.1
```

A dangerous request:

```
GET /restore.cgi HTTP/1.1
...
<the body is an archive>
```

The handler extracts the archive to `/` as root (`docs/16`, B3).

### Capture it

```bash
curl -v http://192.168.50.1/param.cgi?cmd=getlanguage
curl -s http://192.168.50.1/ | head
```

## 3. The Beacon

The beacon is a periodic outbound flow to the collector. It is a **sealed** frame:
`[ciphertext][tag]` (`docs/walkthrough/06`). On the wire it looks like random bytes - which
is why entropy analysis is the detection method (`docs/28`).

### Capture it

```bash
sudo tcpdump -i eth0 -n -ttt 'udp and not port 53' | head
```

Look for a fixed interval and a fixed destination.

## 4. The Router Path

When the device routes (`docs/24`), a client's packet is **NATed** out `eth0`. Capture on
both sides and watch the source address change:

```bash
# on wlan0: client 192.168.50.20 -> 1.1.1.1
# on eth0 : <wan-ip>        -> 1.1.1.1    (masqueraded)
```

## 5. The Detection Matrix

| signal | protocol | detection |
| ------ | -------- | --------- |
| no auth on media | RTSP | connect and play without credentials |
| route that execs | HTTP | find `system()` callers (`docs/15`) |
| periodic egress | beacon | inter-arrival histogram |
| high entropy | beacon | byte-entropy near 8 |
| NAT egress | all | compare wlan0 and eth0 captures |

## Exercises

1. Issue `OPTIONS` and `DESCRIBE` to the RTSP server; record the banner.
2. Fetch a `param.cgi` route and a suspect route; compare the responses.
3. Capture the beacon and measure its interval.
4. Capture a NATed client packet on both interfaces.

## Reference

- `docs/13`, `docs/14`, `docs/15`, `docs/24`, `docs/28`
- RFC 2326 (RTSP), RFC 7230 (HTTP)
