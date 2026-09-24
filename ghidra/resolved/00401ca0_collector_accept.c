
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

