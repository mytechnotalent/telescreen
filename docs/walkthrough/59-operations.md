# Walkthrough 59: Deployment and Operations

**shipping a fleet, and keeping it safe**

***
**LEGAL DISCLAIMER:**
The information, tools, and code provided in this repository and course are strictly for educational, research, and defensive purposes only. 

You are explicitly prohibited from using any materials contained herein to access, test, modify, or exploit any device, network, or system that you do not own 100% or for which you have explicit, documented, and legally binding authorization to interact with.

By using this repository and course, you acknowledge and agree that:

1. Any illegal, unauthorized, or malicious use of this information is solely your responsibility.
2. The author(s) and contributor(s) of this repository and course shall not be held liable for any damages, legal repercussions, criminal charges, or unauthorized actions resulting from the use, misuse, or abuse of the contents herein.
3. You will comply with all applicable local, state, national, and international laws regarding cybersecurity and computer fraud.

**IF YOU DO NOT AGREE WITH THESE TERMS, DO NOT USE THIS REPOSITORY AND COURSE.**

***

## Why Operations

A device is a product only when it is **deployed and maintained**. Most device compromises in
the wild are operations failures: defaults never changed, firmware never updated, a single
weak unit on a network exposing the fleet. This volume is the operations lens.

## 1. Provisioning

| step | control |
| ---- | ------- |
| first boot | force a credential (no default) |
| config | load a per-device config, not a shared one |
| keys | provision a per-device identity (`docs/22`) |
| update channel | point at a signed source |

The single most important provisioning control: **no shared default password**.

## 2. The Update Pipeline

```
build -> sign -> publish -> device verifies -> apply -> verify
```

| step | control |
| ---- | ------- |
| build | reproducible, from source |
| sign | Ed25519 over the image (`docs/22`) |
| publish | over TLS, with a version |
| verify | the device checks the signature before applying (`docs/27`) |
| apply | atomically; keep the old image for rollback |
| verify | confirm the new version |

An unsigned update channel is the **supply-chain** class (`docs/walkthrough/45`).

## 3. Fleet Management

| concern | practice |
| ------- | -------- |
| inventory | know every device and its firmware version |
| config | managed, not hand-edited per unit |
| monitoring | detect the beacon and the backdoors (`docs/28`) |
| isolation | segment cameras and IoT from the corporate LAN |
| decommission | wipe and destroy, do not resell |

## 4. Monitoring

What to watch, at fleet scale:

| signal | means |
| ------ | ----- |
| a device contacting a new endpoint | possible compromise |
| a firmware hash mismatch | a modified image |
| an unexpected listener | a backdoor |
| a config change at boot | an operator artifact |
| unusual outbound volume | exfiltration |

## 5. Incident Response

```
1. detect    : an alert from monitoring
2. contain   : isolate the device (network segment)
3. investigate: dump the flash, hash it, diff against known-good
4. eradicate : reflash the known-good image
5. recover   : restore config, re-provision keys
6. learn     : update the monitoring rules
```

The **flash dump + hash + diff** (`docs/04`, `docs/walkthrough/27`) is the core of step 3.

## 6. The Security Lifecycle

```
design -> build -> test -> deploy -> monitor -> update -> retire
   |       |        |        |         |         |         |
 model   gates    fuzz    harden   detect    sign      wipe
```

A device is not "done" at deploy; it is done at retire.

## 7. The Operations Checklist

```
[ ] no shared default credentials
[ ] per-device identity and keys
[ ] signed, verifiable updates
[ ] an inventory of versions
[ ] monitoring for the beacon and backdoors
[ ] network segmentation
[ ] an incident-response run-book
[ ] a secure decommission process
```

## 8. The Hard Truth

Most of the devices compromised in the field were not broken by a novel zero-day. They were
**never updated**, **kept a default password**, or **sat on a flat network**. Operations is
where security is won or lost at scale.

## Exercises

1. Write the deployment plan for a fleet of 100 TELESCREENs.
2. Write the incident-response run-book for a suspected beacon.
3. Design a monitoring rule for the six defects.
4. Write the decommission procedure.

## Reference

- `docs/27`, `docs/28`, `docs/walkthrough/30`, `docs/walkthrough/49`
