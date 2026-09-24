# Appendix J - Function-by-Function Reverse Engineering

This appendix is the complete reverse-engineering record of the stripped
target `firmware/teled.stripped`.  For every one of the 42 application
functions it gives the address, the meaningless label Ghidra shows, what
the function really is, the call graph around it extracted from the real
machine code, the evidence that resolves it, and its decompiled body.

Everything here is reproducible from `firmware/teled.stripped` alone plus
the instructor's `firmware/teled.unstripped` answer key.

> **The four resolution rules** (see `ghidra/RESOLUTION_MAP.md`):
>
> - **R1** exact address match against the unstripped twin (certain),
> - **R2** `.plt` stub -> `JUMP_SLOT` relocation -> import name (certain),
> - **R3** the `.plt` PLT0 lazy resolver (certain),
> - **R4** phantom/overlapping function on alignment padding (certain).

---

## `0x00401120` - `quarter_round`  (aead)

- **Ghidra shows:** `FUN_00401120` (a stripped binary has no names).
- **Resolved name:** `quarter_round`
- **Module:** `src/aead.c`
- **Role:** Execute one ChaCha quarter-round.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `hchacha_rounds`
- **Calls:** _(leaf function)_

```c

void FUN_00401120(uint *param_1,uint *param_2,uint *param_3,uint *param_4)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = *param_1;
  uVar2 = *param_2;
  *param_1 = uVar1 + uVar2;
  uVar1 = uVar1 + uVar2 ^ *param_4;
  uVar1 = uVar1 >> 0x10 | uVar1 << 0x10;
  *param_4 = uVar1;
  uVar1 = uVar1 + *param_3;
  *param_3 = uVar1;
  uVar1 = uVar1 ^ *param_2;
  uVar1 = uVar1 >> 0x14 | uVar1 << 0xc;
  *param_2 = uVar1;
  uVar1 = uVar1 + *param_1;
  *param_1 = uVar1;
  uVar1 = uVar1 ^ *param_4;
  uVar1 = uVar1 >> 0x18 | uVar1 << 8;
  *param_4 = uVar1;
  uVar1 = uVar1 + *param_3;
  *param_3 = uVar1;
  uVar1 = uVar1 ^ *param_2;
  *param_2 = uVar1 >> 0x19 | uVar1 << 7;
  return;
}
```

## `0x004011a0` - `hchacha_rounds`  (aead)

- **Ghidra shows:** `FUN_004011a0` (a stripped binary has no names).
- **Resolved name:** `hchacha_rounds`
- **Module:** `src/aead.c`
- **Role:** Run ten HChaCha20 double rounds over the state.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (2):** `aead_seal`, `aead_open`
- **Calls (1):** `quarter_round`

```c

void FUN_004011a0(long param_1)

{
  long lVar1;
  long lVar2;
  long lVar3;
  long lVar4;
  long lVar5;
  long lVar6;
  uint uVar7;
  undefined8 extraout_x8;
  undefined8 extraout_x8_00;
  undefined8 extraout_x9;
  undefined8 extraout_x9_00;
  undefined8 extraout_x10;
  undefined8 extraout_x10_00;
  undefined8 extraout_x11;
  undefined8 extraout_x11_00;
  undefined8 extraout_x12;
  undefined8 extraout_x12_00;
  undefined8 extraout_x13;
  undefined8 extraout_x13_00;
  undefined8 extraout_x14;
  undefined8 extraout_x14_00;
  undefined8 extraout_x15;
  undefined8 extraout_x15_00;
  undefined8 extraout_x18;
  undefined8 extraout_x18_00;
  
  lVar1 = param_1 + 0x14;
  lVar2 = param_1 + 0x24;
  lVar3 = param_1 + 0x30;
  lVar4 = param_1 + 4;
  lVar5 = param_1 + 0x10;
  lVar6 = param_1 + 0x20;
  uVar7 = 10;
  do {
    FUN_00401120(param_1,lVar5,lVar6,lVar3);
    FUN_00401120(lVar4,lVar1,lVar2,extraout_x18);
    FUN_00401120(extraout_x15,extraout_x14,extraout_x13,extraout_x12);
    FUN_00401120(extraout_x11,extraout_x10,extraout_x9,extraout_x8);
    FUN_00401120(param_1,lVar1,extraout_x13_00,extraout_x8_00);
    FUN_00401120(lVar4,extraout_x14_00,extraout_x9_00,lVar3);
    FUN_00401120(extraout_x15_00,extraout_x10_00,lVar6,extraout_x18_00);
    FUN_00401120(extraout_x11_00,lVar5,lVar2,extraout_x12_00);
    uVar7 = uVar7 - 1 & 0xff;
  } while (uVar7 != 0);
  return;
}
```

## `0x004012c0` - `hchacha_state`  (aead)

- **Ghidra shows:** `FUN_004012c0` (a stripped binary has no names).
- **Resolved name:** `hchacha_state`
- **Module:** `src/aead.c`
- **Role:** Build the initial HChaCha20 state from key and nonce.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (2):** `aead_seal`, `aead_open`
- **Calls:** _(leaf function)_

```c

void FUN_004012c0(char *param_1,long param_2,undefined4 *param_3)

{
  undefined8 uVar1;
  long lVar2;
  
  uVar1 = s_expand_32_byte_k_stream_004027c0._8_8_;
  lVar2 = 0;
  *(undefined8 *)param_1 = s_expand_32_byte_k_stream_004027c0._0_8_;
  *(undefined8 *)(param_1 + 8) = uVar1;
  do {
    *(undefined4 *)(param_1 + lVar2 + 0x10) = *(undefined4 *)(param_2 + lVar2);
    lVar2 = lVar2 + 4;
  } while (lVar2 != 0x20);
  *(undefined4 *)(param_1 + 0x30) = *param_3;
  *(undefined4 *)(param_1 + 0x34) = param_3[1];
  *(undefined4 *)(param_1 + 0x38) = param_3[2];
  *(undefined4 *)(param_1 + 0x3c) = param_3[3];
  return;
}
```

## `0x00401320` - `gcm_dec_final`  (aead)

- **Ghidra shows:** `FUN_00401320` (a stripped binary has no names).
- **Resolved name:** `gcm_dec_final`
- **Module:** `src/aead.c`
- **Role:** Decrypt the payload and verify the GCM tag.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `aead_open`
- **Calls (3):** `EVP_DecryptFinal_ex@plt`, `EVP_DecryptUpdate@plt`, `EVP_CIPHER_CTX_ctrl@plt`

```c

bool FUN_00401320(EVP_CIPHER_CTX *param_1,uchar *param_2,uchar *param_3)

{
  int iVar1;
  int iVar2;
  int local_4;
  
  local_4 = 0;
  iVar1 = EVP_DecryptUpdate(param_1,param_2,&local_4,param_3,0x30);
  iVar2 = local_4;
  if (iVar1 == 1) {
    iVar1 = EVP_CIPHER_CTX_ctrl(param_1,0x11,0x10,param_3 + 0x30);
    if (iVar1 == 1) {
      iVar2 = EVP_DecryptFinal_ex(param_1,param_2 + iVar2,&local_4);
      return iVar2 == 1;
    }
  }
  return false;
}
```

## `0x004013c0` - `aead_init`  (aead)

- **Ghidra shows:** `FUN_004013c0` (a stripped binary has no names).
- **Resolved name:** `aead_init`
- **Module:** `src/aead.c`
- **Role:** Initialize the AEAD subsystem.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `main`
- **Calls:** _(leaf function)_

```c

undefined8 FUN_004013c0(uint param_1)

{
  if (1 < param_1) {
    return 0;
  }
  DAT_00420140 = param_1;
  DAT_00420144 = 0;
  DAT_00420150 = 0;
  DAT_00420158 = 0;
  DAT_00420160 = 0;
  DAT_00420168 = 1;
  return 1;
}
```

## `0x00401400` - `aead_nonce_size`  (aead)

- **Ghidra shows:** `FUN_00401400` (a stripped binary has no names).
- **Resolved name:** `aead_nonce_size`
- **Module:** `src/aead.c`
- **Role:** Return the nonce size required by the active backend.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined8 FUN_00401400(void)

{
  undefined8 uVar1;
  
  uVar1 = 0;
  if (((DAT_00420168 & 1) != 0) && (uVar1 = 0xc, DAT_00420140 != 0)) {
    uVar1 = 0x18;
  }
  return uVar1;
}
```

## `0x0040142c` - `aead_seal`  (aead)

- **Ghidra shows:** `FUN_0040142c` (a stripped binary has no names).
- **Resolved name:** `aead_seal`
- **Module:** `src/aead.c`
- **Role:** Seal a plaintext payload into ciphertext plus tag.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `beacon_seal`
- **Calls (10):** `EVP_EncryptUpdate@plt`, `EVP_EncryptInit_ex@plt`, `EVP_CIPHER_CTX_new@plt`, `EVP_CIPHER_CTX_free@plt`, `EVP_chacha20_poly1305@plt`, `EVP_CIPHER_CTX_ctrl@plt`, `EVP_aes_256_gcm@plt`, `EVP_EncryptFinal_ex@plt`, `hchacha_rounds`, `hchacha_state`

```c

undefined4 FUN_0040142c(uchar *param_1,uchar *param_2,uchar *param_3,uchar *param_4)

{
  undefined5 uVar1;
  undefined5 uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  EVP_CIPHER_CTX *pEVar7;
  EVP_CIPHER *pEVar8;
  undefined4 uVar9;
  int local_64;
  undefined8 local_60;
  undefined8 uStack_58;
  undefined1 uStack_50;
  undefined1 uStack_4f;
  undefined1 uStack_4e;
  undefined1 uStack_4d;
  undefined1 uStack_4c;
  undefined1 uStack_4b;
  undefined1 uStack_4a;
  undefined1 uStack_49;
  undefined1 uStack_48;
  undefined1 uStack_47;
  undefined1 uStack_46;
  undefined1 uStack_45;
  undefined1 uStack_44;
  undefined1 uStack_43;
  undefined1 uStack_42;
  undefined1 uStack_41;
  int local_40;
  undefined4 local_3c;
  undefined4 uStack_38;
  undefined1 uStack_34;
  undefined1 local_10;
  undefined1 uStack_f;
  undefined1 uStack_e;
  undefined1 uStack_d;
  undefined1 uStack_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if ((DAT_00420168 & 1) == 0) {
    return 1;
  }
  if (param_1 == (uchar *)0x0 || param_2 == (uchar *)0x0) {
    return 1;
  }
  if (param_3 == (uchar *)0x0 || param_4 == (uchar *)0x0) {
    return 1;
  }
  if (DAT_00420140 == 0) {
    DAT_00420150 = *(undefined8 *)param_2;
    DAT_00420144 = 1;
    DAT_00420158 = CONCAT44(DAT_00420158._4_4_,*(undefined4 *)(param_2 + 8));
    pEVar7 = EVP_CIPHER_CTX_new();
    if (pEVar7 == (EVP_CIPHER_CTX *)0x0) {
      return 3;
    }
    pEVar8 = EVP_aes_256_gcm();
    iVar6 = EVP_EncryptInit_ex(pEVar7,pEVar8,(ENGINE *)0x0,(uchar *)0x0,(uchar *)0x0);
    if (((iVar6 == 1) && (iVar6 = EVP_CIPHER_CTX_ctrl(pEVar7,9,0xc,(void *)0x0), iVar6 == 1)) &&
       (iVar6 = EVP_EncryptInit_ex(pEVar7,(EVP_CIPHER *)0x0,(ENGINE *)0x0,param_1,param_2),
       iVar6 == 1)) {
      local_40 = 0;
      iVar6 = EVP_EncryptUpdate(pEVar7,param_4,&local_40,param_3,0x30);
      if (iVar6 == 1) {
        local_40 = 0;
        iVar6 = EVP_EncryptFinal_ex(pEVar7,param_4 + 0x30,&local_40);
        if (iVar6 == 1) {
          iVar6 = EVP_CIPHER_CTX_ctrl(pEVar7,0x10,0x10,param_4 + 0x30);
          uVar9 = 0;
          if (iVar6 == 1) goto LAB_0040167c;
        }
      }
    }
    uVar9 = 3;
LAB_0040167c:
    EVP_CIPHER_CTX_free(pEVar7);
    return uVar9;
  }
  DAT_00420150 = *(undefined8 *)param_2;
  DAT_00420158 = *(undefined8 *)(param_2 + 8);
  DAT_00420160 = *(undefined8 *)(param_2 + 0x10);
  DAT_00420144 = 1;
  FUN_004012c0(&local_40,param_1,param_2);
  FUN_004011a0();
  uVar1 = CONCAT14((undefined1)local_3c,local_40);
  local_40 = 0;
  uVar3 = (uint)local_3c >> 8;
  uStack_4b = (undefined1)((uint)_uStack_c >> 8);
  uVar4 = (uint)local_3c >> 0x18;
  uVar5 = (uint)local_3c >> 0x10;
  uVar2 = CONCAT14(uStack_34,uStack_38);
  uStack_49 = (undefined1)((uint)_uStack_c >> 0x18);
  uStack_4a = (undefined1)((uint)_uStack_c >> 0x10);
  local_3c = (undefined4)*(undefined8 *)(param_2 + 0x10);
  uStack_38 = (undefined4)((ulong)*(undefined8 *)(param_2 + 0x10) >> 0x20);
  uStack_47 = (undefined1)((uint)local_8 >> 8);
  uStack_48 = (undefined1)local_8;
  uStack_45 = (undefined1)((uint)local_8 >> 0x18);
  uStack_46 = (undefined1)((uint)local_8 >> 0x10);
  uStack_43 = (undefined1)((uint)local_4 >> 8);
  uStack_44 = (undefined1)local_4;
  uStack_41 = (undefined1)((uint)local_4 >> 0x18);
  uStack_42 = (undefined1)((uint)local_4 >> 0x10);
  uStack_58 = CONCAT17((char)((uint)_uStack_34 >> 0x18),
                       CONCAT16((char)((uint)_uStack_34 >> 0x10),
                                CONCAT15((char)((uint)_uStack_34 >> 8),uVar2)));
  local_60 = CONCAT17((char)uVar4,CONCAT16((char)uVar5,CONCAT15((char)uVar3,uVar1)));
  uStack_50 = local_10;
  uStack_4f = uStack_f;
  uStack_4e = uStack_e;
  uStack_4d = uStack_d;
  uStack_4c = uStack_c;
  pEVar7 = EVP_CIPHER_CTX_new();
  if (pEVar7 == (EVP_CIPHER_CTX *)0x0) {
    return 3;
  }
  pEVar8 = (EVP_CIPHER *)EVP_chacha20_poly1305();
  iVar6 = EVP_EncryptInit_ex(pEVar7,pEVar8,(ENGINE *)0x0,(uchar *)0x0,(uchar *)0x0);
  if (((iVar6 == 1) && (iVar6 = EVP_CIPHER_CTX_ctrl(pEVar7,9,0xc,(void *)0x0), iVar6 == 1)) &&
     (iVar6 = EVP_EncryptInit_ex(pEVar7,(EVP_CIPHER *)0x0,(ENGINE *)0x0,(uchar *)&local_60,
                                 (uchar *)&local_40), iVar6 == 1)) {
    local_64 = 0;
    iVar6 = EVP_EncryptUpdate(pEVar7,param_4,&local_64,param_3,0x30);
    if (iVar6 == 1) {
      local_64 = 0;
      iVar6 = EVP_EncryptFinal_ex(pEVar7,param_4 + 0x30,&local_64);
      if ((iVar6 == 1) && (iVar6 = EVP_CIPHER_CTX_ctrl(pEVar7,0x10,0x10,param_4 + 0x30), iVar6 == 1)
         ) {
        uVar9 = 0;
        goto LAB_004015e4;
      }
    }
  }
  uVar9 = 3;
LAB_004015e4:
  EVP_CIPHER_CTX_free(pEVar7);
  return uVar9;
}
```

## `0x004017e4` - `aead_open`  (aead)

- **Ghidra shows:** `FUN_004017e4` (a stripped binary has no names).
- **Resolved name:** `aead_open`
- **Module:** `src/aead.c`
- **Role:** Open a sealed payload, verifying the tag before releasing plaintext.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (9):** `EVP_DecryptInit_ex@plt`, `EVP_CIPHER_CTX_new@plt`, `EVP_CIPHER_CTX_free@plt`, `EVP_chacha20_poly1305@plt`, `EVP_CIPHER_CTX_ctrl@plt`, `EVP_aes_256_gcm@plt`, `hchacha_rounds`, `hchacha_state`, `gcm_dec_final`

```c

undefined4 FUN_004017e4(uchar *param_1,uchar *param_2,long param_3,undefined8 *param_4)

{
  undefined5 uVar1;
  undefined5 uVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  EVP_CIPHER_CTX *pEVar7;
  EVP_CIPHER *pEVar8;
  undefined8 local_60;
  undefined8 uStack_58;
  undefined1 uStack_50;
  undefined1 uStack_4f;
  undefined1 uStack_4e;
  undefined1 uStack_4d;
  undefined1 uStack_4c;
  undefined1 uStack_4b;
  undefined1 uStack_4a;
  undefined1 uStack_49;
  undefined1 uStack_48;
  undefined1 uStack_47;
  undefined1 uStack_46;
  undefined1 uStack_45;
  undefined1 uStack_44;
  undefined1 uStack_43;
  undefined1 uStack_42;
  undefined1 uStack_41;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 uStack_38;
  undefined1 uStack_34;
  undefined1 local_10;
  undefined1 uStack_f;
  undefined1 uStack_e;
  undefined1 uStack_d;
  undefined1 uStack_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if ((DAT_00420168 & 1) == 0) {
    return 1;
  }
  if ((param_1 == (uchar *)0x0 || param_2 == (uchar *)0x0) ||
     (param_3 == 0 || param_4 == (undefined8 *)0x0)) {
    return 1;
  }
  if (DAT_00420140 == 0) {
    pEVar7 = EVP_CIPHER_CTX_new();
    if (pEVar7 == (EVP_CIPHER_CTX *)0x0) {
      return 3;
    }
    pEVar8 = EVP_aes_256_gcm();
    iVar6 = EVP_DecryptInit_ex(pEVar7,pEVar8,(ENGINE *)0x0,(uchar *)0x0,(uchar *)0x0);
    if ((((iVar6 == 1) && (iVar6 = EVP_CIPHER_CTX_ctrl(pEVar7,9,0xc,(void *)0x0), iVar6 == 1)) &&
        (iVar6 = EVP_DecryptInit_ex(pEVar7,(EVP_CIPHER *)0x0,(ENGINE *)0x0,param_1,param_2),
        iVar6 == 1)) && (iVar6 = FUN_00401320(pEVar7,param_4,param_3), iVar6 != 0)) {
      EVP_CIPHER_CTX_free(pEVar7);
      return 0;
    }
    EVP_CIPHER_CTX_free(pEVar7);
  }
  else {
    FUN_004012c0(&local_40,param_1,param_2);
    FUN_004011a0();
    uVar1 = CONCAT14((undefined1)local_3c,local_40);
    local_40 = 0;
    uVar3 = (uint)local_3c >> 8;
    uStack_4b = (undefined1)((uint)_uStack_c >> 8);
    uVar4 = (uint)local_3c >> 0x18;
    uVar5 = (uint)local_3c >> 0x10;
    uVar2 = CONCAT14(uStack_34,uStack_38);
    uStack_49 = (undefined1)((uint)_uStack_c >> 0x18);
    uStack_4a = (undefined1)((uint)_uStack_c >> 0x10);
    local_3c = (undefined4)*(undefined8 *)(param_2 + 0x10);
    uStack_38 = (undefined4)((ulong)*(undefined8 *)(param_2 + 0x10) >> 0x20);
    uStack_47 = (undefined1)((uint)local_8 >> 8);
    uStack_48 = (undefined1)local_8;
    uStack_45 = (undefined1)((uint)local_8 >> 0x18);
    uStack_46 = (undefined1)((uint)local_8 >> 0x10);
    uStack_43 = (undefined1)((uint)local_4 >> 8);
    uStack_44 = (undefined1)local_4;
    uStack_41 = (undefined1)((uint)local_4 >> 0x18);
    uStack_42 = (undefined1)((uint)local_4 >> 0x10);
    uStack_58 = CONCAT17((char)((uint)_uStack_34 >> 0x18),
                         CONCAT16((char)((uint)_uStack_34 >> 0x10),
                                  CONCAT15((char)((uint)_uStack_34 >> 8),uVar2)));
    local_60 = CONCAT17((char)uVar4,CONCAT16((char)uVar5,CONCAT15((char)uVar3,uVar1)));
    uStack_50 = local_10;
    uStack_4f = uStack_f;
    uStack_4e = uStack_e;
    uStack_4d = uStack_d;
    uStack_4c = uStack_c;
    pEVar7 = EVP_CIPHER_CTX_new();
    if (pEVar7 == (EVP_CIPHER_CTX *)0x0) {
      return 3;
    }
    pEVar8 = (EVP_CIPHER *)EVP_chacha20_poly1305();
    iVar6 = EVP_DecryptInit_ex(pEVar7,pEVar8,(ENGINE *)0x0,(uchar *)0x0,(uchar *)0x0);
    if (((iVar6 == 1) && (iVar6 = EVP_CIPHER_CTX_ctrl(pEVar7,9,0xc,(void *)0x0), iVar6 == 1)) &&
       ((iVar6 = EVP_DecryptInit_ex(pEVar7,(EVP_CIPHER *)0x0,(ENGINE *)0x0,(uchar *)&local_60,
                                    (uchar *)&local_40), iVar6 == 1 &&
        (iVar6 = FUN_00401320(pEVar7,param_4,param_3), iVar6 != 0)))) {
      EVP_CIPHER_CTX_free(pEVar7);
      return 0;
    }
    EVP_CIPHER_CTX_free(pEVar7);
  }
  *param_4 = 0;
  param_4[1] = 0;
  param_4[2] = 0;
  param_4[3] = 0;
  param_4[4] = 0;
  param_4[5] = 0;
  return 2;
}
```

## `0x00401ae0` - `beacon_weak_key`  (beacon)

- **Ghidra shows:** `FUN_00401ae0` (a stripped binary has no names).
- **Resolved name:** `beacon_weak_key`
- **Module:** `src/beacon.c`
- **Role:** Derive the Ministry "sealed" key from the public device UID.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (2):** `strlen@plt`, `crc32_le`

```c

void FUN_00401ae0(char *param_1,long param_2)

{
  size_t sVar1;
  undefined8 uVar2;
  long lVar3;
  
  lVar3 = 0;
  sVar1 = strlen(param_1);
  uVar2 = FUN_00401d80(0,param_1,sVar1);
  do {
    uVar2 = FUN_00401d80(uVar2,param_1,sVar1);
    *(char *)(param_2 + lVar3) = (char)uVar2;
    lVar3 = lVar3 + 1;
  } while (lVar3 != 0x20);
  return;
}
```

## `0x00401b4c` - `beacon_nonce`  (beacon)

- **Ghidra shows:** `FUN_00401b4c` (a stripped binary has no names).
- **Resolved name:** `beacon_nonce`
- **Module:** `src/beacon.c`
- **Role:** Build the beacon nonce from the sequence number.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

void FUN_00401b4c(undefined4 param_1,undefined4 *param_2)

{
  *param_2 = param_1;
  *(undefined8 *)(param_2 + 1) = 0;
  *(undefined8 *)(param_2 + 3) = 0;
  param_2[5] = 0;
  return;
}
```

## `0x00401b60` - `beacon_seal`  (beacon)

- **Ghidra shows:** `FUN_00401b60` (a stripped binary has no names).
- **Resolved name:** `beacon_seal`
- **Module:** `src/beacon.c`
- **Role:** Seal one beacon payload with the hardened AEAD key.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `teled_beacon`
- **Calls (1):** `aead_seal`

```c

bool FUN_00401b60(undefined8 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 local_18;
  undefined8 local_14;
  undefined8 uStack_c;
  undefined4 local_4;
  
  local_14 = 0;
  uStack_c = 0;
  local_4 = 0;
  local_18 = param_2;
  iVar1 = FUN_0040142c(param_1,&local_18);
  return iVar1 == 0;
}
```

## `0x00401ba0` - `camera_rtsp_url`  (camera)

- **Ghidra shows:** `FUN_00401ba0` (a stripped binary has no names).
- **Resolved name:** `camera_rtsp_url`
- **Module:** `src/camera.c`
- **Role:** Build the RTSP URL a client (or VLC) connects to.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (1):** `snprintf@plt`

```c

bool FUN_00401ba0(long param_1,uint param_2,char *param_3,ulong param_4)

{
  int iVar1;
  
  if ((param_1 == 0 || param_4 == 0) || param_3 == (char *)0x0) {
    return false;
  }
  iVar1 = snprintf(param_3,param_4,"%s://%s:%u%s",&DAT_004027d8,param_1,(ulong)(param_2 & 0xffff),
                   "/stream");
  return (ulong)(long)iVar1 < param_4;
}
```

## `0x00401c08` - `camera_mjpeg_url`  (camera)

- **Ghidra shows:** `FUN_00401c08` (a stripped binary has no names).
- **Resolved name:** `camera_mjpeg_url`
- **Module:** `src/camera.c`
- **Role:** Build the MJPEG URL a browser connects to.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (1):** `snprintf@plt`

```c

bool FUN_00401c08(long param_1,uint param_2,char *param_3,ulong param_4)

{
  int iVar1;
  
  if ((param_1 == 0 || param_4 == 0) || param_3 == (char *)0x0) {
    return false;
  }
  iVar1 = snprintf(param_3,param_4,"%s://%s:%u%s",&DAT_00402808,param_1,(ulong)(param_2 & 0xffff),
                   "/video.mjpg");
  return (ulong)(long)iVar1 < param_4;
}
```

## `0x00401c80` - `collector_init`  (collector)

- **Ghidra shows:** `FUN_00401c80` (a stripped binary has no names).
- **Resolved name:** `collector_init`
- **Module:** `src/collector.c`
- **Role:** Reset the collector accounting state.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00401c80(void)

{
  _DAT_00420170 = 0;
  DAT_00420178 = 0;
  return;
}
```

## `0x00401ca0` - `collector_accept`  (collector)

- **Ghidra shows:** `FUN_00401ca0` (a stripped binary has no names).
- **Resolved name:** `collector_accept`
- **Module:** `src/collector.c`
- **Role:** Accept or reject one beacon frame.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `teled_beacon`
- **Calls:** _(leaf function)_

```c

undefined8 FUN_00401ca0(long param_1,long param_2,undefined4 param_3)

{
  if (param_2 == 0x40 && param_1 != 0) {
    DAT_00420178 = param_3;
    DAT_00420170 = DAT_00420170 + 1;
    return 1;
  }
  DAT_00420174 = DAT_00420174 + 1;
  return 0;
}
```

## `0x00401ce4` - `collector_stats`  (collector)

- **Ghidra shows:** `FUN_00401ce4` (a stripped binary has no names).
- **Resolved name:** `collector_stats`
- **Module:** `src/collector.c`
- **Role:** Return a pointer to the collector accounting snapshot.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined4 * FUN_00401ce4(void)

{
  return &DAT_00420170;
}
```

## `0x00401d00` - `container_magic`  (container)

- **Ghidra shows:** `FUN_00401d00` (a stripped binary has no names).
- **Resolved name:** `container_magic`
- **Module:** `src/container.c`
- **Role:** Read the container magic from the header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined4 FUN_00401d00(undefined4 *param_1)

{
  return *param_1;
}
```

## `0x00401d08` - `container_length`  (container)

- **Ghidra shows:** `FUN_00401d08` (a stripped binary has no names).
- **Resolved name:** `container_length`
- **Module:** `src/container.c`
- **Role:** Read the container length field from the header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined4 FUN_00401d08(long param_1)

{
  return *(undefined4 *)(param_1 + 4);
}
```

## `0x00401d10` - `container_tag_ok`  (container)

- **Ghidra shows:** `FUN_00401d10` (a stripped binary has no names).
- **Resolved name:** `container_tag_ok`
- **Module:** `src/container.c`
- **Role:** Verify the "gziphead" tag in the container header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

bool FUN_00401d10(long param_1)

{
  return *(long *)(param_1 + 8) == 0x6461656870697a67;
}
```

## `0x00401d30` - `container_valid`  (container)

- **Ghidra shows:** `FUN_00401d30` (a stripped binary has no names).
- **Resolved name:** `container_valid`
- **Module:** `src/container.c`
- **Role:** Validate a vendor kernel container header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

bool FUN_00401d30(int *param_1,ulong param_2)

{
  if (param_1 == (int *)0x0 || param_2 < 0x10) {
    return false;
  }
  if (*param_1 != 0x1b8421) {
    return param_1 == (int *)0x0 || param_2 < 0x10;
  }
  return *(long *)(param_1 + 2) == 0x6461656870697a67;
}
```

## `0x00401d80` - `crc32_le`  (crc)

- **Ghidra shows:** `FUN_00401d80` (a stripped binary has no names).
- **Resolved name:** `crc32_le`
- **Module:** `src/crc.c`
- **Role:** Compute the reflected crc32_le checksum used by JFFS2.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (2):** `beacon_weak_key`, `jffs2_hdr_crc_valid`
- **Calls:** _(leaf function)_

```c

ulong FUN_00401d80(ulong param_1,byte *param_2,long param_3)

{
  byte *pbVar1;
  char cVar2;
  
  if (param_3 != 0) {
    pbVar1 = param_2 + param_3;
    do {
      cVar2 = '\b';
      param_1 = (ulong)((uint)param_1 ^ (uint)*param_2);
      do {
        cVar2 = cVar2 + -1;
        param_1 = (ulong)(-((uint)param_1 & 1) & 0xedb88320 ^ (uint)param_1 >> 1);
      } while (cVar2 != '\0');
      param_2 = param_2 + 1;
    } while (param_2 != pbVar1);
  }
  return param_1;
}
```

## `0x00401de0` - `crc32_uboot`  (crc)

- **Ghidra shows:** `FUN_00401de0` (a stripped binary has no names).
- **Resolved name:** `crc32_uboot`
- **Module:** `src/crc.c`
- **Role:** Compute the standard CRC32 used by the U-Boot environment.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `env_crc_valid`
- **Calls:** _(leaf function)_

```c

uint FUN_00401de0(byte *param_1,long param_2)

{
  byte *pbVar1;
  char cVar2;
  uint uVar3;
  
  if (param_2 != 0) {
    pbVar1 = param_1 + param_2;
    uVar3 = 0xffffffff;
    do {
      cVar2 = '\b';
      uVar3 = uVar3 ^ *param_1;
      do {
        cVar2 = cVar2 + -1;
        uVar3 = -(uVar3 & 1) & 0xedb88320 ^ uVar3 >> 1;
      } while (cVar2 != '\0');
      param_1 = param_1 + 1;
    } while (param_1 != pbVar1);
    return ~uVar3;
  }
  return 0;
}
```

## `0x00401e40` - `crc16_ccitt`  (crc)

- **Ghidra shows:** `FUN_00401e40` (a stripped binary has no names).
- **Resolved name:** `crc16_ccitt`
- **Module:** `src/crc.c`
- **Role:** Compute the CRC-16/CCITT-FALSE checksum of a byte buffer.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

uint FUN_00401e40(byte *param_1,long param_2)

{
  byte *pbVar1;
  uint uVar2;
  uint uVar3;
  char cVar4;
  uint uVar5;
  
  if (param_2 != 0) {
    pbVar1 = param_1 + param_2;
    uVar3 = 0xffff;
    do {
      uVar5 = (uint)*param_1;
      cVar4 = '\b';
      do {
        uVar2 = uVar3 ^ uVar5 << 8;
        uVar3 = (uVar3 & 0x7fff) << 1;
        cVar4 = cVar4 + -1;
        uVar5 = (uVar5 & 0x7f) << 1;
        if ((uVar2 & 0x8000) != 0) {
          uVar3 = uVar3 ^ 0x1021;
        }
      } while (cVar4 != '\0');
      param_1 = param_1 + 1;
    } while (pbVar1 != param_1);
    return uVar3;
  }
  return 0xffff;
}
```

## `0x00401ec0` - `env_compute_crc`  (env)

- **Ghidra shows:** `FUN_00401ec0` (a stripped binary has no names).
- **Resolved name:** `env_compute_crc`
- **Module:** `src/env.c`
- **Role:** Compute the environment CRC32 over the key/value blob.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined8 FUN_00401ec0(long param_1,ulong param_2)

{
  undefined8 uVar1;
  
  if (param_1 == 0 || param_2 < 4) {
    return 0;
  }
  uVar1 = FUN_00401de0(param_1 + 4,param_2 - 4);
  return uVar1;
}
```

## `0x00401ee0` - `env_crc_valid`  (env)

- **Ghidra shows:** `FUN_00401ee0` (a stripped binary has no names).
- **Resolved name:** `env_crc_valid`
- **Module:** `src/env.c`
- **Role:** Verify the leading environment CRC32.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (1):** `crc32_uboot`

```c

bool FUN_00401ee0(int *param_1,ulong param_2)

{
  int iVar1;
  int iVar2;
  
  if (param_1 == (int *)0x0 || param_2 < 4) {
    return false;
  }
  iVar1 = *param_1;
  iVar2 = FUN_00401de0(param_1 + 1,param_2 - 4);
  return iVar1 == iVar2;
}
```

## `0x00401f20` - `env_find`  (env)

- **Ghidra shows:** `FUN_00401f20` (a stripped binary has no names).
- **Resolved name:** `env_find`
- **Module:** `src/env.c`
- **Role:** Find a key in the environment blob and return its value.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (2):** `strlen@plt`, `memcmp@plt`

```c

long FUN_00401f20(long param_1,ulong param_2,char *param_3)

{
  int iVar1;
  size_t __n;
  ulong uVar2;
  
  if ((param_3 == (char *)0x0 || param_2 < 4) || param_1 == 0) {
    return 0;
  }
  __n = strlen(param_3);
  uVar2 = __n + 5;
  if (uVar2 < param_2) {
    do {
      if ((*(char *)(param_1 + -1 + uVar2) == '=') &&
         (iVar1 = memcmp((void *)(param_1 + -1 + (uVar2 - __n)),param_3,__n), iVar1 == 0)) {
        return param_1 + uVar2;
      }
      uVar2 = uVar2 + 1;
    } while (uVar2 < param_2);
  }
  return 0;
}
```

## `0x00401fe0` - `identity_pubkey`  (identity)

- **Ghidra shows:** `FUN_00401fe0` (a stripped binary has no names).
- **Resolved name:** `identity_pubkey`
- **Module:** `src/identity.c`
- **Role:** Derive the Ed25519 public key from a private seed.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (3):** `EVP_PKEY_free@plt`, `EVP_PKEY_new_raw_private_key@plt`, `EVP_PKEY_get_raw_public_key@plt`

```c

bool FUN_00401fe0(undefined8 param_1,undefined8 param_2)

{
  int iVar1;
  EVP_PKEY *pkey;
  long local_8;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_private_key(0x43f,0,param_1,0x20);
  local_8 = 0x20;
  if ((pkey != (EVP_PKEY *)0x0) &&
     (iVar1 = EVP_PKEY_get_raw_public_key(pkey,param_2,&local_8), iVar1 == 1)) {
    EVP_PKEY_free(pkey);
    return local_8 == 0x20;
  }
  EVP_PKEY_free(pkey);
  return false;
}
```

## `0x00402060` - `identity_sign`  (identity)

- **Ghidra shows:** `FUN_00402060` (a stripped binary has no names).
- **Resolved name:** `identity_sign`
- **Module:** `src/identity.c`
- **Role:** Sign a message with an Ed25519 private seed.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (6):** `EVP_DigestSign@plt`, `EVP_DigestSignInit@plt`, `EVP_PKEY_free@plt`, `EVP_PKEY_new_raw_private_key@plt`, `EVP_MD_CTX_new@plt`, `EVP_MD_CTX_free@plt`

```c

bool FUN_00402060(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  int iVar1;
  EVP_PKEY *pkey;
  EVP_MD_CTX *ctx;
  long local_8;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_private_key(0x43f,0,param_1,0x20);
  ctx = (EVP_MD_CTX *)EVP_MD_CTX_new();
  local_8 = 0x40;
  if (pkey != (EVP_PKEY *)0x0 && ctx != (EVP_MD_CTX *)0x0) {
    iVar1 = EVP_DigestSignInit(ctx,(EVP_PKEY_CTX **)0x0,(EVP_MD *)0x0,(ENGINE *)0x0,pkey);
    if (iVar1 != 1) {
      EVP_MD_CTX_free(ctx);
      EVP_PKEY_free(pkey);
      return false;
    }
    iVar1 = EVP_DigestSign(ctx,param_4,&local_8,param_2,param_3);
    if (iVar1 == 1) {
      EVP_MD_CTX_free(ctx);
      EVP_PKEY_free(pkey);
      return local_8 == 0x40;
    }
  }
  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  return false;
}
```

## `0x00402160` - `identity_verify`  (identity)

- **Ghidra shows:** `FUN_00402160` (a stripped binary has no names).
- **Resolved name:** `identity_verify`
- **Module:** `src/identity.c`
- **Role:** Verify an Ed25519 signature.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (6):** `EVP_DigestVerify@plt`, `EVP_DigestVerifyInit@plt`, `EVP_PKEY_free@plt`, `EVP_PKEY_new_raw_public_key@plt`, `EVP_MD_CTX_new@plt`, `EVP_MD_CTX_free@plt`

```c

bool FUN_00402160(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  bool bVar1;
  int iVar2;
  EVP_PKEY *pkey;
  EVP_MD_CTX *ctx;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_public_key(0x43f,0,param_1,0x20);
  ctx = (EVP_MD_CTX *)EVP_MD_CTX_new();
  if ((pkey == (EVP_PKEY *)0x0 || ctx == (EVP_MD_CTX *)0x0) ||
     (iVar2 = EVP_DigestVerifyInit(ctx,(EVP_PKEY_CTX **)0x0,(EVP_MD *)0x0,(ENGINE *)0x0,pkey),
     iVar2 != 1)) {
    bVar1 = false;
  }
  else {
    iVar2 = EVP_DigestVerify(ctx,param_4,0x40,param_2,param_3);
    bVar1 = iVar2 == 1;
  }
  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  return bVar1;
}
```

## `0x00402220` - `jffs2_magic`  (jffs2)

- **Ghidra shows:** `FUN_00402220` (a stripped binary has no names).
- **Resolved name:** `jffs2_magic`
- **Module:** `src/jffs2.c`
- **Role:** Read the little-endian magic from a node header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined2 FUN_00402220(undefined2 *param_1)

{
  return *param_1;
}
```

## `0x00402228` - `jffs2_type`  (jffs2)

- **Ghidra shows:** `FUN_00402228` (a stripped binary has no names).
- **Resolved name:** `jffs2_type`
- **Module:** `src/jffs2.c`
- **Role:** Read the little-endian node type from a node header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined2 FUN_00402228(long param_1)

{
  return *(undefined2 *)(param_1 + 2);
}
```

## `0x00402230` - `jffs2_totlen`  (jffs2)

- **Ghidra shows:** `FUN_00402230` (a stripped binary has no names).
- **Resolved name:** `jffs2_totlen`
- **Module:** `src/jffs2.c`
- **Role:** Read the little-endian total length from a node header.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined4 FUN_00402230(long param_1)

{
  return *(undefined4 *)(param_1 + 4);
}
```

## `0x00402240` - `jffs2_hdr_crc_valid`  (jffs2)

- **Ghidra shows:** `FUN_00402240` (a stripped binary has no names).
- **Resolved name:** `jffs2_hdr_crc_valid`
- **Module:** `src/jffs2.c`
- **Role:** Verify the crc32_le header CRC of a node.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (1):** `crc32_le`

```c

bool FUN_00402240(long param_1)

{
  int iVar1;
  
  iVar1 = FUN_00401d80(0,param_1,8);
  return *(int *)(param_1 + 8) == iVar1;
}
```

## `0x00402280` - `kex_hkdf`  (kex)

- **Ghidra shows:** `FUN_00402280` (a stripped binary has no names).
- **Resolved name:** `kex_hkdf`
- **Module:** `src/kex.c`
- **Role:** Derive key material with HKDF-SHA256.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (3):** `memcpy@plt`, `HMAC@plt`, `EVP_sha256@plt`

```c

undefined8
FUN_00402280(uchar *param_1,size_t param_2,void *param_3,int param_4,void *param_5,size_t param_6,
            long param_7,ulong param_8)

{
  void *__dest;
  EVP_MD *pEVar1;
  uchar *puVar2;
  undefined8 uVar3;
  size_t __n;
  size_t __n_00;
  char cVar4;
  ulong uVar5;
  uint local_ac;
  uchar auStack_a8 [32];
  uchar auStack_88 [32];
  uint local_68 [26];
  
  local_68[0] = 0;
  pEVar1 = EVP_sha256();
  puVar2 = HMAC(pEVar1,param_3,param_4,param_1,param_2,auStack_a8,local_68);
  uVar3 = 0;
  if (puVar2 != (uchar *)0x0) {
    if (param_8 != 0) {
      __n = 0;
      uVar5 = 0;
      cVar4 = '\x01';
      do {
        memcpy(local_68,auStack_88,__n);
        memcpy((void *)((long)local_68 + __n),param_5,param_6);
        local_ac = 0;
        *(char *)((long)local_68 + param_6 + __n) = cVar4;
        pEVar1 = EVP_sha256();
        __n_00 = param_8 - uVar5;
        puVar2 = HMAC(pEVar1,auStack_a8,0x20,(uchar *)local_68,param_6 + __n + 1,auStack_88,
                      &local_ac);
        if (0x20 < __n_00) {
          __n_00 = 0x20;
        }
        __dest = (void *)(param_7 + uVar5);
        if (puVar2 == (uchar *)0x0) {
          return 0;
        }
        cVar4 = cVar4 + '\x01';
        uVar5 = uVar5 + __n_00;
        memcpy(__dest,auStack_88,__n_00);
        __n = 0x20;
      } while (uVar5 < param_8);
    }
    uVar3 = 1;
  }
  return uVar3;
}
```

## `0x004023d0` - `kex_x25519`  (kex)

- **Ghidra shows:** `FUN_004023d0` (a stripped binary has no names).
- **Resolved name:** `kex_x25519`
- **Module:** `src/kex.c`
- **Role:** Compute the X25519 shared secret.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (8):** `EVP_PKEY_derive_set_peer@plt`, `EVP_PKEY_CTX_new@plt`, `EVP_PKEY_free@plt`, `EVP_PKEY_new_raw_public_key@plt`, `EVP_PKEY_new_raw_private_key@plt`, `EVP_PKEY_derive@plt`, `EVP_PKEY_CTX_free@plt`, `EVP_PKEY_derive_init@plt`

```c

bool FUN_004023d0(undefined8 param_1,undefined8 param_2,uchar *param_3)

{
  int iVar1;
  EVP_PKEY *pkey;
  EVP_PKEY *peer;
  EVP_PKEY_CTX *ctx;
  size_t local_8;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_private_key(0x40a,0,param_1,0x20);
  peer = (EVP_PKEY *)EVP_PKEY_new_raw_public_key(0x40a,0,param_2,0x20);
  if (pkey == (EVP_PKEY *)0x0 || peer == (EVP_PKEY *)0x0) {
    ctx = (EVP_PKEY_CTX *)0x0;
    local_8 = 0x20;
  }
  else {
    ctx = EVP_PKEY_CTX_new(pkey,(ENGINE *)0x0);
    local_8 = 0x20;
    if (((ctx != (EVP_PKEY_CTX *)0x0) && (iVar1 = EVP_PKEY_derive_init(ctx), iVar1 == 1)) &&
       (iVar1 = EVP_PKEY_derive_set_peer(ctx,peer), iVar1 == 1)) {
      iVar1 = EVP_PKEY_derive(ctx,param_3,&local_8);
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(pkey);
      EVP_PKEY_free(peer);
      if (iVar1 != 1) {
        return false;
      }
      return local_8 == 0x20;
    }
  }
  EVP_PKEY_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  EVP_PKEY_free(peer);
  return false;
}
```

## `0x00402500` - `part_desc_for`  (partition)

- **Ghidra shows:** `FUN_00402500` (a stripped binary has no names).
- **Resolved name:** `part_desc_for`
- **Module:** `src/partition.c`
- **Role:** Return the descriptor for a partition kind.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined * FUN_00402500(int param_1)

{
  undefined *puVar1;
  
  if (param_1 == 1) {
    return &DAT_00402850;
  }
  if (param_1 != 2) {
    if (param_1 != 3) {
      puVar1 = &DAT_00402898;
      if (param_1 != 4) {
        puVar1 = (undefined *)0x0;
      }
      return puVar1;
    }
    return &DAT_00402880;
  }
  return &DAT_00402868;
}
```

## `0x00402560` - `part_identify`  (partition)

- **Ghidra shows:** `FUN_00402560` (a stripped binary has no names).
- **Resolved name:** `part_identify`
- **Module:** `src/partition.c`
- **Role:** Identify a partition by its magic bytes.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

char FUN_00402560(int *param_1,ulong param_2)

{
  char cVar1;
  
  cVar1 = '\0';
  if (param_1 != (int *)0x0 && 3 < param_2) {
    cVar1 = '\x01';
    if ((*param_1 != -0x15fffaeb) &&
       (cVar1 = (*param_1 == 0x1b8421) + '\x02', (short)*param_1 == 0x1985)) {
      cVar1 = '\x04';
    }
  }
  return cVar1;
}
```

## `0x004025c0` - `part_verify_layout`  (partition)

- **Ghidra shows:** `FUN_004025c0` (a stripped binary has no names).
- **Resolved name:** `part_verify_layout`
- **Module:** `src/partition.c`
- **Role:** Verify that a whole-flash image is large enough and consistent.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

long FUN_004025c0(ulong param_1)

{
  return (ulong)(param_1 < 0x1000000) << 1;
}
```

## `0x004025e0` - `part_carve`  (partition)

- **Ghidra shows:** `FUN_004025e0` (a stripped binary has no names).
- **Resolved name:** `part_carve`
- **Module:** `src/partition.c`
- **Role:** Copy one partition out of a whole-flash image.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls (1):** `memcpy@plt`

```c

char FUN_004025e0(long param_1,ulong param_2,int param_3,void *param_4,ulong param_5)

{
  char cVar1;
  long lVar2;
  
  if (param_3 == 1) {
    lVar2 = 0;
  }
  else if (param_3 == 2) {
    lVar2 = 1;
  }
  else if (param_3 == 3) {
    lVar2 = 2;
  }
  else {
    if (param_3 != 4) {
      return (param_2 < 0x1000000 && (param_4 != (void *)0x0 && param_1 != 0)) + '\x01';
    }
    lVar2 = 3;
  }
  cVar1 = '\x01';
  if (param_4 != (void *)0x0 && param_1 != 0) {
    cVar1 = '\x02';
    if (param_2 < 0x1000000) {
      return cVar1;
    }
    if ((uint)(&DAT_00402858)[lVar2 * 6] <= param_5) {
      memcpy(param_4,(void *)(param_1 + (ulong)(uint)(&DAT_00402854)[lVar2 * 6]),
             (ulong)(uint)(&DAT_00402858)[lVar2 * 6]);
      return '\0';
    }
  }
  return cVar1;
}
```

## `0x004026c0` - `teled_init`  (teled)

- **Ghidra shows:** `FUN_004026c0` (a stripped binary has no names).
- **Resolved name:** `teled_init`
- **Module:** `src/teled.c`
- **Role:** Initialize the daemon with a device UID and session key.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `main`
- **Calls (2):** `memcpy@plt`, `strlen@plt`

```c

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004026c0(char *param_1,undefined8 *param_2)

{
  size_t __n;
  
  _DAT_00420180 = 0;
  uRam0000000000420188 = 0;
  _DAT_00420190 = 0;
  uRam0000000000420198 = 0;
  _DAT_004201a0 = 0;
  uRam00000000004201a8 = 0;
  _DAT_004201b0 = 0;
  uRam00000000004201b8 = 0;
  DAT_004201c0 = 0;
  __n = strlen(param_1);
  if (0x1f < __n) {
    __n = 0x1f;
  }
  memcpy(&DAT_00420180,param_1,__n);
  _DAT_004201a0 = *param_2;
  uRam00000000004201a8 = param_2[1];
  uRam00000000004201b8 = param_2[3];
  _DAT_004201b0 = param_2[2];
  FUN_00401c80();
  return;
}
```

## `0x00402728` - `teled_beacon`  (teled)

- **Ghidra shows:** `FUN_00402728` (a stripped binary has no names).
- **Resolved name:** `teled_beacon`
- **Module:** `src/teled.c`
- **Role:** Seal and deliver one beacon to the local collector.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by (1):** `main`
- **Calls (2):** `beacon_seal`, `collector_accept`

```c

byte FUN_00402728(undefined8 param_1)

{
  byte bVar1;
  undefined1 auStack_40 [64];
  
  DAT_004201c0._0_4_ = (int)DAT_004201c0 + 1;
  bVar1 = FUN_00401b60(&DAT_004201a0,(int)DAT_004201c0,param_1,auStack_40);
  if ((bVar1 & 1) != 0) {
    DAT_004201c0._4_4_ = DAT_004201c0._4_4_ + 1;
    bVar1 = FUN_00401ca0(auStack_40,0x40,(int)DAT_004201c0);
  }
  return bVar1;
}
```

## `0x0040278c` - `teled_state`  (teled)

- **Ghidra shows:** `FUN_0040278c` (a stripped binary has no names).
- **Resolved name:** `teled_state`
- **Module:** `src/teled.c`
- **Role:** Return a pointer to the daemon state snapshot.
- **Evidence:** R1: exact nm match against the unstripped twin.
- **Called by:** _(entry points only)_
- **Calls:** _(leaf function)_

```c

undefined * FUN_0040278c(void)

{
  return &DAT_00420180;
}
```

---

## Reverse-engineering lessons this binary teaches

### Lesson 1 - Identical Code Folding (ICF)

`aead_open` contains two paths whose final step is byte-for-byte
identical.  The linker merged them, so the symbol table points two source
functions at the SAME address:

```
0000000000401320 t chacha_dec_final
0000000000401320 t gcm_dec_final
```

Two names, one address.  A reverse engineer must read the surrounding
code to decide which one is executing in a given path.

### Lesson 2 - Phantom functions on alignment padding

Modern toolchains align functions to 16 bytes and pad with `nop`.  Ghidra
can mistake the padding for the start of a small function, producing a
phantom that overlaps the real one:

- `0x00401b9c` is a phantom whose body is identical to `beacon_seal`
  (`0x00401ba0`).
- `0x004020dc` is a phantom whose body is identical to `identity_sign`
  (`0x004020e0`).

Always confirm a function's true entry with the call graph and the
prologue (`stp x29, x30, [sp, #-N]!`), never by Ghidra's guess alone.

### Lesson 3 - The dual PLT (with and without pointer authentication)

There are two `.plt` sections (`linux_x86_64`-style `.plt` plus `.plt.sec`),
so each imported library function appears twice.  Both thunks end in
`br x17`; the address they branch to is the GOT slot named by the import.

