# Volume 15: `system()` Sites

**every command template and its taint**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you do not have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## What You'll Learn

- Why `system()` is the single most dangerous call in embedded C
- How to find every `system()` site in a stripped binary
- The **taint** method: trace each `%s` back to its source
- Which sites are fixed and which are attacker-reachable

## 1. Why `system()` Is Dangerous

`system("ping " + user_input)` runs a **shell**. Any shell metacharacter in the input
(`;`, `` ` ``, `$()`, `|`) is a new command, run as **root**. The program intended to run
`ping`, but ran `ping; rm -rf /`.

## 2. Find Every Site

In source: search for `system(`. In a stripped binary: look for the import and its call
sites.

```bash
# source
grep -rn 'system(' src/
# binary
readelf -s teled | grep -w system
objdump -d teled | grep -B1 'system'
```

## 3. The Templates

Real embedded command templates look like this:

```
ping %s -c 5 > /tmp/out
ifconfig %s down
unzip -o %s -d /
tar -xvzf %s -C /
cp -f %s %s
rm -rf %s
chmod a+x %s
```

## 4. The Taint Method

For each `sprintf(buf, template, X); system(buf)`, trace **X** back to its origin:

```
request param -> config value -> sprintf -> system
```

If **X** is **network-controlled** (a request parameter, an uploaded filename, an SSID,
a hostname), it is an **injection**. If **X** is a **constant** (a fixed path), it is
not - but the file's *contents* may still matter (`tar -C /`, `docs/16` B3).

## 5. Classify the Sites

| site | parameter | verdict |
| ---- | --------- | ------- |
| `ping %s` | hostname | **injection** if reachable |
| `ifconfig %s` | interface name | **injection** if from config |
| `unzip -o %s -d /` | archive path | fixed path; **contents** matter |
| `tar -xvzf %s -C /` | archive path | fixed path; **contents** matter |
| `rm -rf %s` | path | injection if tainted |
| `chmod a+x %s` | path | injection if tainted |

## 6. The Fix

Never build a shell string from data. Use `execve` with an **argument vector** so no
shell parses the input:

```c
/* WRONG */
system("ping %s", host);                     /* shell parses host */

/* RIGHT */
char *argv[] = { "ping", "-c", "5", host, NULL };
execve("/bin/ping", argv, environ);          /* no shell */
```

## 7. Lab: Find and Taint

1. Find every `system()` call in the daemon (source or binary).
2. For each, write its command template and its parameter.
3. Classify each as fixed / contents-only / injection.
4. For one injection, write a one-line exploit path.

## Reference

- `docs/14` (dispatcher), `docs/16` (backdoors), `docs/28` (blue team)
