# Volume 14: HTTP Dispatcher

**the embedded server and its internal routes**

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

- That the web server is the **application**, not a separate daemon
- That the `*.cgi` routes are **internal code**, not files
- How to find the dispatcher by its strings
- Why this concentrates the attack surface

## 1. The Server Is the Application

On the TELESCREEN the HTTP server is not BusyBox `httpd` and not Apache. It is the
**application** (`teled`) listening on TCP 80. This concentrates the entire web surface
into one process running as **root**.

## 2. Internal Routes, Not Files

The web tree contains only a couple of real files; **every other `*.cgi` is an internal
route** handled inside the application. So `GET /restore.cgi`, `GET /param.cgi`, and
`GET /upgrade.cgi` do not hit the filesystem - they dispatch into a handler function.

```
GET /param.cgi?cmd=set...   -> dispatcher -> handler -> system(...)
GET /restore.cgi            -> dispatcher -> handler -> tar -xvzf ... -C /
```

## 3. Find the Dispatcher

In a stripped binary you find the dispatcher by its **route strings**:

```bash
strings -n4 teled | grep -E '\.cgi'
# param.cgi
# restore.cgi
# backup.cgi
# upgrade.cgi
```

Then cross-reference the string to the function that holds it (Ghidra xref, or
`axt` in radare2). That function is the dispatcher.

## 4. The CGI Pattern

The classic embedded pattern:

```
1. the request names a route (param.cgi) and parameters (?cmd=set...)
2. the dispatcher looks up the handler
3. the handler builds a response, often by running a shell command
```

Step 3 is where `docs/15` lives: every `system()` site the dispatcher can reach.

## 5. Why It Concentrates Risk

| factor | effect |
| ------ | ------ |
| server == application == root | one bug = root |
| routes are code, not files | no `httpd.conf` to reason about |
| config setters call `system()` | network input becomes shell input |

## 6. The Defensive Shape

A defended TELESCREEN would:

- serve static content from a **non-root** helper;
- register routes explicitly with **typed** parameters;
- **never** build a shell command from a request (`docs/15`).

## Labs

1. List the `*.cgi` strings in the daemon and identify the dispatcher function.
2. Explain why "the routes are not files" changes how you attack it.
3. Trace one route from request to handler and name the dangerous step.

## Reference

- `docs/15` (system() sites), `docs/16` (backdoors), `docs/28` (blue team)
