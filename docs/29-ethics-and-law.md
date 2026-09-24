# Volume 29: Ethics and Law

**responsible research, disclosure, and the law**

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

- The authorisation rule and why it is not optional
- Proportionality in offensive research
- Responsible disclosure
- The difference between a lab and a real-world operation

## 1. The Authorisation Rule

> Only test hardware you **own 100%** or for which you have **explicit, documented,
> legally binding authorisation**.

"That device is on my network" is not ownership. "The vendor said students can" is not
authorisation for *your* unit. Get it in writing, or work only on your own bench.

## 2. Proportionality

Offensive technique is justified only within **lawful, proportionate, authorised**
bounds:

| question | honest answer |
| -------- | ------------- |
| Do I own it or have written authorisation? | required |
| Is the action necessary and proportionate? | required |
| Could it harm a third party? | then stop |

## 3. Responsible Disclosure

If you find a real defect:

1. **Document** it precisely (mechanism, path, impact).
2. **Report** it to the vendor through a security contact.
3. **Do not** weaponise or publish exploit code before the vendor has a window to fix.
4. **Respect** the users: a camera bug is someone's home.

## 4. The Lab vs the World

Everything in this course is on **your** hardware, on an **isolated** network, with a
**local** collector. That is a lab. The moment you point it at a device you do not own,
it is not. The techniques are identical; the authorisation is the difference.

## 5. Respect the Builders

The engineers who built the silicon are **teachers, not villains**. The craft - a first
stage in SRAM, a hand-rolled filesystem, a working NAT stack - is admirable. The security
flaws are **design and priority** decisions, not incompetence. Criticise the design; learn
from the craft.

## 6. The Oath (a practical checklist)

```
[ ] I own the hardware, or I have written authorisation.
[ ] The network is isolated; the collector is local.
[ ] I have the golden image for rollback.
[ ] I will not attack a device I do not own.
[ ] I will disclose responsibly, not weaponise.
[ ] I will delete captured secrets after the lab.
```

## Labs

1. Write your own authorisation statement for the device you are testing.
2. Describe the disclosure process for a defect you found.
3. Argue, in 300 words, the proportionality of offensive security research.
4. Name one thing this course taught you to **defend**, not just to break.

## Reference

- `docs/01` (threat model), `docs/28` (blue team)
- Your jurisdiction's computer-misuse law
