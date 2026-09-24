
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

