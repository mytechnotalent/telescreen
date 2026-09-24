
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

