
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

