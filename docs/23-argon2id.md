# Volume 23: Argon2id

**password-based key derivation**

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

- Why a password must never become a key directly
- What **memory-hard** means and why it defeats GPUs
- How to choose Argon2id parameters
- Where it fits in the hardened stack

## 1. The Rule

> A password has low entropy. Never feed it to a cipher. Run it through a
> **password-hashing KDF** first.

A plain hash (`SHA-256(password)`) is fast, so an attacker with a GPU tests billions of
candidates per second. A **memory-hard** KDF makes each guess cost a lot of memory,
which GPUs cannot parallelise cheaply.

## 2. Argon2id

Argon2 won the Password Hashing Competition. The `id` variant mixes data-dependent and
data-independent access, resisting both side-channel and GPU attacks. Its cost is set by
three parameters:

| parameter | meaning |
| --------- | ------- |
| iterations | time cost |
| memory (KiB) | memory cost - the key defence |
| parallelism | lanes |

## 3. Choose Parameters

Pick values that cost an **attacker** far more than your device:

```
iterations  >= 3
memory      >= 64 MiB (tune to the device)
parallelism >= 1
```

On a device, measure: the derivation must fit your boot time budget. On an RP5
Cortex-A76, tens of MiB is comfortable.

## 4. Use It

```c
uint8_t key[32];
uint8_t salt[16];       /* random per password store */
argon2id_hash_raw(iterations, memory_kib, parallelism,
                  password, password_len, salt, sizeof(salt), key, sizeof(key));
```

Then seal with the key (`docs/18`).

## 5. Salt and Storage

- Generate a **random salt** per password (never a fixed one).
- Store the salt and parameters beside the verifier.
- Wipe the derived key when done.

## 6. Where It Fits

| job | primitive |
| --- | --------- |
| password -> key | **Argon2id** |
| key agreement | X25519 + HKDF (`docs/21`) |
| data | AES-256-GCM (`docs/19`) |
| identity | Ed25519 (`docs/22`) |

Use Argon2id for the **human** secret (a login password, a config passphrase); use
X25519/HKDF for **machine** secrets (session keys).

## Labs

1. Derive a key from a passphrase with two different salts; show the keys differ.
2. Raise the memory parameter and measure the time increase.
3. Explain why `SHA-256(password)` is not acceptable as a key.
4. Name the two places Argon2id is used in the hardened TELESCREEN.

## Reference

- Argon2 specification (RFC 9106)
- `docs/21` (HKDF), `docs/22` (Ed25519)
