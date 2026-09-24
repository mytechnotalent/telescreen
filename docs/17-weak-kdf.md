# Volume 17: Weak KDF

**why a key from a public identifier is not encryption**

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

- The difference between **obfuscation** and **encryption**
- The real `beacon.c` weak key and why it fails
- How to recover the key and decrypt captured telemetry
- The correct replacement (`docs/18`-`docs/23`)

## 1. The Rule

> **A cipher is only as strong as the secrecy of its key.** If the key is a deterministic
> function of data the attacker already has, the scheme provides **zero
> confidentiality** - no matter which cipher is named.

## 2. The Weak Schedule

```c
void beacon_weak_key(const char *uid, uint8_t out[BEACON_KEY_SIZE]) {
    size_t ulen = strlen(uid);
    uint32_t seed = crc32_le(0u, (const uint8_t *)uid, ulen);
    size_t i;
    for (i = 0u; i < BEACON_KEY_SIZE; ++i) {
        seed = crc32_le(seed, (const uint8_t *)uid, ulen);
        out[i] = (uint8_t)(seed & 0xFFu);
    }
}
```

The key is a function of **`uid`**, which is **public** (printed on the device, in the
config, broadcast in discovery).

## 3. Recover It

The tooling ships the recovered schedule:

```bash
$ python3 scripts/weak_decrypt.py --uid SSAT-468547-FEEBD
da506e04af00c6f40394d2cd2295bfc8682e8b9f9e9b844cea50c08d5f483141
```

Anyone who knows the UID derives the same 32 bytes. Then they decrypt every beacon.

## 4. Why "but it uses AES" Is Not an Answer

| question | matters? |
| -------- | -------- |
| Is the cipher strong? | yes, but not sufficient |
| **Where does the key come from?** | **this decides everything** |
| Is the nonce unique? | yes, or the scheme breaks |

A strong cipher with a public-derived key is a lock with the key taped to the door.

## 5. The Correct Replacement

Replace the public-derived key with a **secret** key and add integrity:

```
key = HKDF( X25519(priv, peer), salt, info )     ; a real session key
seal = AES-256-GCM(key, nonce, plaintext)        ; confidentiality + integrity
```

See `docs/21` (X25519/HKDF), `docs/19` (AES-GCM), `docs/18` (AEAD basics).

## 6. The Test

Two devices with **different** UIDs must derive **different** keys, and neither key must
be derivable from the other's UID. The `test_beacon_weak_key_differs` case checks the
first property; it also demonstrates the flaw, because the derivation is still public.

## Labs

1. Derive the weak key for two different UIDs; show they differ.
2. Explain in 200 words why a public-UID key provides no confidentiality.
3. Replace `beacon_weak_key` with an HKDF-based key and show the test still passes.

## Reference

- `src/beacon.c`, `scripts/weak_decrypt.py`
- `docs/18` (AEAD), `docs/21` (X25519/HKDF)
