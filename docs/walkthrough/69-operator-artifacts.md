# Walkthrough 69: Operator Artifacts

**distinguishing a vendor backdoor from a third party's leftovers**

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

## The Problem

You find something suspicious in a device: a config with a shell command, an extra CGI, a
marker file. Is it a **vendor backdoor** or someone else's leftover? Getting this **wrong**
is a serious professional failure - you can accuse a vendor of shipping a backdoor that was
actually a prior owner's test. This volume is the discipline for telling them apart.

## 1. The Three Possibilities

| possibility | example | how to tell |
| ----------- | ------- | ----------- |
| **Vendor** | shipped in the factory image | present in a pristine factory image |
| **Operator** | added by a prior owner/pentest | **absent** from the factory image; matches a common test pattern |
| **Unknown** | could be either | cannot decide without a factory image - **label it** |

## 2. The Proof: A Pristine Image

The only conclusive test is a **factory image** diff:

```
factory image  vs  your image
   differences  ->  candidate operator artifacts
   identical    ->  vendor
```

Without a factory image, you **cannot** conclude vendor intent. Say so.

## 3. The Camera Lesson

In the PROJECT camera teardown, a config file (`wifi.conf`) contained shell commands (dump
the flash, start a server, touch a marker). It was tempting to call it a **vendor backdoor**.
It was **the prior operator's own modification** - proven by diffing against the vendor
reference and by the file being writable and appended after the fact.

**The two CGIs (`a.cgi`, `d.cgi`) remained `[unproven]`** because no pristine image was
available. That is the honest position: *suspicious, origin unresolved.*

## 4. Indicators

| indicator | leans vendor | leans operator |
| --------- | ------------ | -------------- |
| present in a factory image | **vendor** | |
| absent from every factory image | | **operator** |
| matches a vendor's known debug feature | leans vendor | |
| matches a common test/pentest payload | | leans operator |
| a marker like `pwned` / `owned` | | **operator** |
| writable config appended at runtime | | leans operator |
| shipped default credentials | leans vendor | |
| a signature over it | leans vendor | |

## 5. The Labeling Rule

Every finding gets a provenance label:

```
[VENDOR]    present in the factory image, or documented by the vendor
[OPERATOR]  absent from the factory image, or matches a known test pattern
[UNPROVEN]  cannot be decided without a pristine image
```

**Never** upgrade `[UNPROVEN]` to `[VENDOR]` because it "looks like a backdoor". Looking like
one is not being one.

## 6. The Method

```
1. Acquire a pristine factory image (vendor download, or an untouched unit).
2. Diff per region (docs/walkthrough/27).
3. Classify each difference.
4. Label every finding [VENDOR]/[OPERATOR]/[UNPROVEN].
5. For [UNPROVEN], state exactly what evidence would resolve it.
```

## 7. Why It Matters

- **Legally.** Accusing a vendor of a backdoor is defamatory if wrong.
- **Technically.** An operator artifact tells you the device was **touched**, which is its
  own finding (a prior compromise).
- **Professionally.** An analyst who labels claims carefully is trusted; one who overclaims
  is not.

## 8. The Operator Artifact as a Finding

An operator artifact is not "nothing". It means:

- **someone had access** (physical or root);
- **the device may be compromised** (not just buggy);
- **the incident** may be a **prior intrusion**, not a vendor issue.

So it flips the story from "the vendor ships a backdoor" to "**this unit was compromised by
someone with access**" - a different, and often more important, finding.

## 9. The Template

```
FINDING   : a shell payload in conf/<file>
PROVENANCE: [OPERATOR]
EVIDENCE  : absent from the factory image (sha256 diff); matches a common test pattern
IMPACT    : the device was modified by a party with root/physical access
RESOLUTION: reflash the factory image; investigate how access was obtained
```

## Exercises

1. Take a device you modified and diff it against a factory image.
2. Label each difference.
3. For one `[UNPROVEN]` finding, write what would resolve it.
4. Rewrite an overclaiming finding with correct labels.

## Reference

- `docs/walkthrough/26` (forensics), `docs/walkthrough/27` (diff)
- `docs/16` (backdoors), `NATION-STATE-REVIEW.md`
