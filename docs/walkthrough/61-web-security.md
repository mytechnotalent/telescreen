# Walkthrough 61: Web Security for Firmware

**the web UI is the biggest attack surface on a camera**

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

## Why the Web UI

A camera's web UI is usually the **largest** piece of attack surface: it is on by default, it
runs as **root**, and it accepts input from the network. Most of the lab's defects are web
defects.

## 1. The Embedded Web Server

Unlike a normal web app, an embedded server is:

- **the application** (no separate daemon, `docs/14`);
- **root**;
- **small** (a hand-written parser, not nginx).

A hand-written parser is where pre-auth bugs live.

## 2. The Request Path

```
socket -> parse request line -> parse headers -> route -> handler -> response
```

Every step is a potential sink. The parser is the classic source of pre-auth memory bugs
(an over-long request line).

## 3. The Route Model

On the lab, routes are **internal** (`docs/14`), not files:

```
GET /param.cgi   -> handler
GET /restore.cgi -> handler
```

There is no `.htaccess`, no file serving rules - just code. That concentrates the surface
into the app.

## 4. The Classic Defects

| defect | lab | class |
| ------ | --- | ----- |
| command injection | B2 | `system()` from a param |
| archive to root | B3 | extraction to `/` |
| auth bypass | B4 | empty/default creds |
| pre-auth overflow | (V1 class) | unbounded parse |
| info leak | B3-style | read any file |

## 5. Injection

```c
sprintf(buf, "ping %s", host);     /* host = "1.1.1.1; id" */
system(buf);
```

The fix is not "sanitize" (blocklists fail); it is **do not invoke a shell**. Use `execve`
with a vector (`docs/15`).

## 6. Path Traversal

```
GET /restore.cgi  (archive with ../../etc/init.d/S99x)
```

The **fixed path** is safe; the **contents** are not. Validate every entry: reject `..`, reject
absolute paths, resolve within the target, and drop privileges.

## 7. Authentication

| mistake | consequence |
| ------- | ----------- |
| empty `.htpasswd` | no auth |
| default creds | trivial auth |
| auth **after** parse | pre-auth bugs |

The rule: **authenticate before parsing** anything attacker-controlled.

## 8. Session and State

An embedded UI often has no session model: it trusts a cookie or nothing. The lesson: any
config action must be authenticated **per request**, and a token must be unguessable.

## 9. The Defensive Web Checklist

```
[ ] authenticate before parse
[ ] no shell from input (execve)
[ ] validate and sandbox every path
[ ] no default credentials
[ ] bound every parser buffer
[ ] run the server with least privilege
[ ] no debug endpoints
[ ] rate-limit the expensive routes
```

## 10. Testing the Web Surface

```bash
# enumerate routes
for r in param.cgi restore.cgi backup.cgi upgrade.cgi; do
  curl -s -o /dev/null -w "$r %{http_code}\n" http://192.168.50.1/$r
done
# a benign injection probe (your own device)
curl -s "http://192.168.50.1/param.cgi?cmd=getlanguage"
# check auth posture
curl -sI http://192.168.50.1/
```

## 11. The Detection Angle

| signal | means |
| ------ | ----- |
| a request to a `restore`/`upgrade` route | a possible exploit |
| a `%00`/`../` in a URL | traversal attempt |
| a request with shell metacharacters | injection attempt |
| an unauthenticated config write | a broken control |

A reverse proxy or WAF in front of the camera can log these (`docs/28`).

## Exercises

1. Enumerate the lab's routes and their HTTP codes.
2. For B2, write the injection and the `execve` fix.
3. For B3, construct the traversal archive and the validation fix.
4. Write three detection rules for the web surface.

## Reference

- `docs/14`, `docs/15`, `docs/16`, `docs/28`
- OWASP Top 10; the OWASP IoT Top 10
