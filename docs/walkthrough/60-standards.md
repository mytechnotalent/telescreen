# Walkthrough 60: Standards and Compliance

**the frameworks a device must satisfy**

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

## Why Standards

Engineering answers "does it work". Standards answer "is it acceptable". For a camera, the
relevant frameworks are security baselines, privacy law, and the standards the device
claims to speak. This volume maps the lab to them.

## 1. Security Frameworks

| framework | relevance to a camera |
| --------- | --------------------- |
| **OWASP IoT Top 10** | the classic IoT weaknesses |
| **NIST IR 8259 / 8425** | IoT device security baseline |
| **IEC 62443** | industrial control / device security |
| **ETSI EN 303 645** | consumer IoT baseline |
| **CIS Benchmarks** | hardened configuration |

### OWASP IoT Top 10, mapped

| item | lab defect |
| ---- | ---------- |
| weak/default passwords | B4 |
| insecure network services | B2, RTSP |
| insecure ecosystem interfaces | B1 |
| lack of secure update | the unsigned update |
| use of insecure components | the vendor SDK |
| insufficient privacy protection | the beacon |
| insecure data transfer/storage | B6 |
| lack of device management | `docs/walkthrough/59` |
| insecure default settings | defaults |
| lack of physical hardening | no secure boot |

Nearly every item maps to a lab defect. That is the point: the lab is a worked example of the
IoT Top 10.

## 2. ETSI EN 303 645 (Consumer IoT)

Key provisions and the lab's state:

| provision | lab |
| --------- | --- |
| no universal default passwords | **fails** (B4) |
| implement a vulnerability-disclosure policy | `docs/29` |
| keep software updated | the unsigned update |
| securely store sensitive data | **fails** (B6) |
| communicate securely | **fails** (B6) |
| minimise exposed attack surface | **fails** (B2) |
| ensure software integrity | **fails** (no secure boot) |

The lab is deliberately non-compliant so it can teach the provisions.

## 3. Privacy Law

| regime | obligation |
| ------ | ---------- |
| GDPR | lawful basis, data minimisation, security |
| CCPA/CPRA | notice, rights, security |
| others | sector-specific (health, finance) |

A camera that **routes** and **beacons** without consent engages all of these. The
**design** answer is privacy by default (`docs/walkthrough/58`).

## 4. The Protocols' Standards

| claim | standard |
| ----- | -------- |
| "ONVIF" | the ONVIF Core Specification |
| "RTSP" | RFC 2326 |
| "TLS" | the TLS RFCs |
| "DHCP" | RFC 2131 |
| "DNS" | RFC 1035 |

A device that **claims** a standard but implements it loosely is both non-compliant and
vulnerable (the parser-bug class).

## 5. The Crypto Standards

| primitive | standard |
| --------- | -------- |
| AES-GCM | NIST SP 800-38D |
| ChaCha20-Poly1305 | RFC 8439 |
| X25519 | RFC 7748 |
| HKDF | RFC 5869 |
| Ed25519 | RFC 8032 |
| Argon2 | RFC 9106 |

The lab pins its tests to these vectors (`docs/walkthrough/15`) - the practical meaning of
"standards compliance".

## 6. The Compliance Checklist

```
[ ] no universal default passwords       (ETSI, OWASP)
[ ] secure update with signature         (ETSI, OWASP)
[ ] secure storage and transport         (ETSI, OWASP)
[ ] minimal exposed surface              (ETSI)
[ ] vulnerability disclosure policy      (ETSI)
[ ] privacy by default                   (GDPR/CCPA)
[ ] implements claimed protocols to spec (RFCs)
[ ] uses standard crypto with vectors    (NIST/RFCs)
```

## 7. The Auditor's View

An auditor reads the design, the code, and the evidence:

- **design**: are the boundaries and controls documented?
- **code**: does it follow the standard?
- **evidence**: are the tests and vectors present?

This repo provides all three: the volumes (design), the audited code (standard), and the
RFC-vector tests (evidence). That is what makes it an **auditable** artifact.

## Exercises

1. Map each OWASP IoT item to a lab defect and a fix.
2. Mark the ETSI provisions the lab fails.
3. Write a vulnerability-disclosure policy for the lab.
4. Assemble the evidence an auditor would want.

## Reference

- `docs/16`, `docs/17`, `docs/28`, `docs/29`, `docs/walkthrough/30`
- OWASP IoT Top 10; ETSI EN 303 645; NIST IR 8259; RFCs
