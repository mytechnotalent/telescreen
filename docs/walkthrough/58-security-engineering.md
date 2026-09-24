# Walkthrough 58: Security Engineering

**building the defended device, not just finding the bugs**

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

## From Finding to Building

Finding a bug is one skill; **engineering a system without them** is another. This volume
is the builder's counterpart to the offensive path (`docs/walkthrough/29`).

## 1. Threat Modeling First

Before code, ask:

```
1. What are the assets?              (docs/walkthrough/49)
2. Who are the adversaries?
3. Where are the trust boundaries?
4. What could cross each boundary?
5. What control stops each crossing?
```

A device designed without this ships the six defects (`docs/16`).

## 2. Secure by Design

| principle | application |
| --------- | ----------- |
| least privilege | the web server is not root |
| fail closed | a failed auth denies, never allows |
| complete mediation | every request is checked |
| no default secrets | no factory password |
| no shell from data | `execve`, not `system` |
| validate at the boundary | parse strictly; reject |
| defence in depth | several controls per asset |

## 3. Secure Coding (the lab's rules)

The code in this repo already follows the defensive rules:

- **bounded copies** (`snprintf` + fit check);
- **checked lengths** before use;
- **no `system()`** in the app;
- **tag-first** AEAD open;
- **secret** keys from HKDF;
- **wipe** secrets after use;
- **100% coverage** so no branch is untested.

## 4. Secure Boot and Update

| control | effect |
| ------- | ------ |
| signed images | cannot boot a modified image |
| verified update | cannot install a modified image |
| anti-rollback | cannot downgrade to a known-bad image |

The primitives are in `identity.c` (`docs/22`) and the update path is `docs/26`.

## 5. Crypto Engineering

| do | do not |
| -- | ------ |
| X25519 + HKDF for session keys | derive keys from public data |
| AES-GCM / XChaCha20-Poly1305 | roll your own AEAD |
| a fresh nonce, always | reuse a nonce |
| wipe keys after use | leave keys in memory |
| standard vectors in tests | test only round-trips |

## 6. Privacy by Design

A camera that routes sees the social graph. Engineering must decide:

- **route by default?** or only with consent;
- **beacon by default?** or off;
- **collect what?** minimum necessary;
- **retain how long?** as short as possible.

These are **design** decisions, not bugs - and they matter more than any CVE
(`docs/walkthrough/25`).

## 7. The Build and the Pipeline

| practice | tool |
| -------- | ---- |
| a standard | the auditors |
| tests | `run_tests.py` |
| coverage | `check_coverage.py` |
| hardening flags | `-fstack-protector-strong -D_FORTIFY_SOURCE=2` |
| CI | the workflow |

A defect that a gate would catch ships only if the gate is missing.

## 8. Assuring the Result

| step | question |
| ---- | -------- |
| design review | are the boundaries and controls explicit? |
| code review | is every copy bounded? every key secret? |
| testing | is coverage 100%? |
| fuzzing | do the parsers survive random input? |
| red team | can the offensive path still succeed? |

## 9. The Engineering Checklist

```
[ ] threat model written
[ ] least privilege everywhere
[ ] no default secrets
[ ] no shell from input
[ ] bounded copies, checked lengths
[ ] secret keys, fresh nonces, wiped buffers
[ ] signed images + verified updates
[ ] privacy defaults: route/beacon off
[ ] the gates are green in CI
[ ] offensive path re-tested after each change
```

## 10. The Mindset

Security engineering is **building so that the boring bugs cannot happen**. The offensive
path found six; the engineering path removes the *class* of each. A device is secure not
because it has no bugs, but because its **boundaries are explicit and its controls are in
depth**.

## Exercises

1. Write a threat model for the TELESCREEN (`docs/walkthrough/49` is the template).
2. Apply the checklist to your own device and find the gaps.
3. Remove one defect **class** (not one instance) from the lab.
4. Re-run the offensive path and show it now fails.

## Reference

- `docs/16`, `docs/22`, `docs/26`, `docs/28`, `docs/walkthrough/30`, `docs/walkthrough/49`
