
undefined8 FUN_00401ec0(long param_1,ulong param_2)

{
  undefined8 uVar1;
  
  if (param_1 == 0 || param_2 < 4) {
    return 0;
  }
  uVar1 = FUN_00401de0(param_1 + 4,param_2 - 4);
  return uVar1;
}

