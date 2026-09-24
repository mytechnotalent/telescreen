
bool FUN_00402240(long param_1)

{
  int iVar1;
  
  iVar1 = FUN_00401d80(0,param_1,8);
  return *(int *)(param_1 + 8) == iVar1;
}

