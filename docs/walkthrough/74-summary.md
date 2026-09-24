# Walkthrough 74: The One-Page Summary

**the entire course, compressed**

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

## The Device

A Raspberry Pi 5 running a real U-Boot -> Linux -> JFFS2 chain, laid out as **four
partitions**: `boot` (U-Boot), `bootargs` (environment), `kernel` (container), `rootfs`
(JFFS2).

## The Method

```
image -> magic -> carve -> boot chain -> rootfs -> app -> sinks -> crypto
   -> findings -> fix -> rebuild -> verify
```

## The Flaws

Six: config-sourced root exec, CGI command injection, archive-to-root, default credentials,
debug shell, public-ID key. Every one is a **missing boundary**.

## The Crypto

Get a **secret** key (X25519 + HKDF), keep **nonces unique**, use an **AEAD** (AES-GCM), sign
with **Ed25519**, hash passwords with **Argon2id**. Nothing is "unhackable"; key management is
everything.

## The Gates

```
C standard: 0 violations
Python standard: exit 0
Tests: 98 checks, 0 failures
Coverage: 100.00%
```

## The Discipline

**Trust the bytes. Follow the sink. Label your claims.** That is the whole of it.

## The Safety

Own hardware or written authorisation. Isolated network. Local collector. Disclose
responsibly. Never accuse a vendor without a pristine-image proof.

## The Call

Now do it on a device you have never seen.

## Reference

- `docs/walkthrough/70` (quick start), `docs/walkthrough/66` (master index)
