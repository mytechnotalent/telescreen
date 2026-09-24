
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

