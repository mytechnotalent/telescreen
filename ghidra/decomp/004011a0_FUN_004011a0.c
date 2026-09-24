
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

