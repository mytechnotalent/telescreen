
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

