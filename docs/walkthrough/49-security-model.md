# Walkthrough 49: The Security Model

**assets, adversaries, and the controls that close the gaps**

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

## The Model

A security model names what you protect, from whom, and with what. This volume is the lab's
model, and it generalises to any device.

## 1. The Assets

| asset | why it matters |
| ----- | -------------- |
| the firmware | code integrity, persistence |
| the config | credentials, keys, endpoints |
| the media | the video/audio the camera captures |
| the network position | the traffic the router sees |
| the device identity | what it can authenticate to |

## 2. The Adversaries

| adversary | access | goal |
| --------- | ------ | ---- |
| remote unauthenticated | a port | execute code |
| remote authenticated | the UI | escalate |
| local network | the LAN | intercept, pivot |
| cloud/relay | the P2P path | observe, impersonate |
| physical | the flash | total control |

## 3. The Trust Boundaries

```
   network  --->  [ app ]  --->  code execution
   cloud    --->  [ crypto ] --->  confidentiality
   physical --->  [ boot ] --->  image integrity
```

Each boundary is a control point. A boundary is "closed" when crossing it requires
something the adversary does not have.

## 4. The Controls

| boundary | control | primitive |
| -------- | ------- | --------- |
| network | auth before parse | a credential check |
| network | safe primitives | `execve`, bounds, `snprintf` |
| code | the sink discipline | no `system()` from input |
| crypto | secret keys | X25519 + HKDF |
| crypto | integrity | an AEAD tag |
| physical | signed images | Ed25519 + secure boot |
| physical | authenticated updates | signed update images |
| all | least privilege | drop root where possible |

## 5. The Gaps (the lab's state)

| gap | defect | fix |
| --- | ------ | --- |
| config sourced as root | B1 | parse, do not source |
| `system()` from input | B2 | `execve` |
| archive to `/` | B3 | validate entries |
| default/empty creds | B4 | require a password |
| debug shell | B5 | remove debug |
| public-ID key | B6 | X25519 + HKDF |
| no secure boot | — | sign and verify |

## 6. The Risk Statement

For each gap: **likelihood x impact**.

| gap | likelihood | impact | risk |
| --- | ---------- | ------ | ---- |
| B1 | low (root needed) | high | medium |
| B2 | high (network) | high | **critical** |
| B3 | medium | high | high |
| B4 | high | high | **critical** |
| B5 | medium | high | high |
| B6 | high | medium | high |
| no secure boot | low (physical) | high | high |

The **network** gaps (B2, B4) dominate because anyone can reach them.

## 7. Defence in Depth

No single control saves the device. The **stack** does:

```
1. no default creds          (B4)
2. auth before parse         (B2 surfaces)
3. no shell from input       (B2)
4. validate all paths        (B3)
5. remove debug              (B5)
6. secret keys + AEAD        (B6)
7. signed images + secure boot (physical)
8. least privilege
9. detection                 (docs/28)
```

If one fails, the next limits the damage.

## 8. The Security Model as a Table

| asset | adversary | boundary | control |
| ----- | --------- | -------- | ------- |
| media | remote unauth | network | auth before parse |
| config | authenticated | app | sink discipline |
| identity | relay | crypto | secret keys |
| firmware | physical | boot | signed images |
| traffic | LAN | crypto | AEAD |

Fill this table for **any** device and you have its security model.

## Exercises

1. Fill the model table for a device you own.
2. Rank its gaps by risk.
3. Write the defence-in-depth stack for it.
4. Name the one control that would most reduce its risk.

## Reference

- `docs/01`, `docs/16`, `docs/17`, `docs/28`, `docs/walkthrough/30`
