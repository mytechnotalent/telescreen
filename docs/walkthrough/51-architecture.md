# Walkthrough 51: Reference Architecture

**the device as a layered system**

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

## The Layers

```
+------------------------------------------------------------+
|  application  : teled (beacon, camera, router, dispatcher) |
+------------------------------------------------------------+
|  libraries    : aead, kex, identity, beacon, camera, ...   |
+------------------------------------------------------------+
|  c library    : libc / OpenSSL                             |
+------------------------------------------------------------+
|  kernel       : Linux 6.6 (DT, mtdparts, JFFS2)            |
+------------------------------------------------------------+
|  bootloader   : U-Boot 2024.07 (env, loads kernel)         |
+------------------------------------------------------------+
|  first stage  : VideoCore firmware (closed)                |
+------------------------------------------------------------+
|  storage      : microSD/NVMe, four regions (JFFS2 etc.)    |
+------------------------------------------------------------+
```

Each layer is a volume. Reading upward is the boot (`docs/05`-`docs/13`); reading downward
is a request or a beacon (`docs/walkthrough/23`).

## Layer 1: Storage

Four fixed regions (`docs/02`): U-Boot, env, kernel container, JFFS2 rootfs. The **format**
is the contract; the **silicon** is not.

## Layer 2: First Stage

Closed (VideoCore on the RP5, BootROM on a camera). Its job: read the head of storage into
SRAM and jump (`docs/05`, `docs/06`).

## Layer 3: Bootloader

U-Boot (`docs/06`): reads the environment (`docs/07`), probes storage, loads the kernel by
offset. **Where secure boot would live** (`docs/walkthrough/38`).

## Layer 4: Kernel

Linux (`docs/08`-`docs/10`): parses the DT (`docs/09`), the command line, `mtdparts`, and
mounts the rootfs. It is **generic**; the DT and cmdline make it this board.

## Layer 5: C Library

libc + OpenSSL. The AEAD, KEX, and identity modules call into it. **The boundary where the
lab depends on a third party** (`docs/walkthrough/11`, `docs/walkthrough/12`).

## Layer 6: Libraries (the lab's modules)

| module | layer role |
| ------ | ---------- |
| `crc` | integrity |
| `aead` | confidentiality + integrity |
| `kex` | key agreement |
| `identity` | signatures |
| `beacon` | the telemetry message |
| `collector` | the sink |
| `camera` | the URL surface |
| `partition`/`env`/`container`/`jffs2` | format readers |

## Layer 7: Application

`teled` (`docs/walkthrough/34`): the composition. It owns the loops (beacon, camera,
router, dispatcher) and calls the libraries.

## The Cross-Cutting Concerns

| concern | where |
| ------- | ----- |
| integrity | `crc` (accidental), `aead` (adversarial) |
| confidentiality | `aead` |
| authenticity | `identity` |
| secrecy | `kex` + key management |
| least privilege | **missing** (the app runs as root) |
| logging | the console + the collector |

**Least privilege is the gap:** a defended device would run the web server and the camera
pipeline with fewer privileges than the network config.

## The Data Planes

```
control plane : the HTTP dispatcher, the config setters     (docs/14)
data plane    : the RTSP stream, the router                 (docs/24, docs/25)
management    : the beacon, the update path                 (docs/17, docs/26)
```

An attack on the **control plane** (a config setter) is worse than one on the **data plane**
(streaming): the control plane changes configuration.

## The Reference Architecture, Generalized

Any IP device is:

```
storage -> first stage -> bootloader -> kernel -> rootfs -> app
                                                        /  |  \
                                                  control data management
```

Fill each box for your device and you have its architecture. Then find the **sinks** and the
**keys** (`docs/15`, `docs/17`).

## Exercises

1. Draw this diagram for a device you own, filling each layer.
2. Mark the trust boundaries.
3. Name one control per layer.
4. Identify the layer with the most risk.

## Reference

- `docs/02`, `docs/10`, `docs/13`, `docs/walkthrough/34`
- `docs/walkthrough/49` (the security model)
