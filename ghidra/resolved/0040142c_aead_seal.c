
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

