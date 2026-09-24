
void FUN_00401120(uint *param_1,uint *param_2,uint *param_3,uint *param_4)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = *param_1;
  uVar2 = *param_2;
  *param_1 = uVar1 + uVar2;
  uVar1 = uVar1 + uVar2 ^ *param_4;
  uVar1 = uVar1 >> 0x10 | uVar1 << 0x10;
  *param_4 = uVar1;
  uVar1 = uVar1 + *param_3;
  *param_3 = uVar1;
  uVar1 = uVar1 ^ *param_2;
  uVar1 = uVar1 >> 0x14 | uVar1 << 0xc;
  *param_2 = uVar1;
  uVar1 = uVar1 + *param_1;
  *param_1 = uVar1;
  uVar1 = uVar1 ^ *param_4;
  uVar1 = uVar1 >> 0x18 | uVar1 << 8;
  *param_4 = uVar1;
  uVar1 = uVar1 + *param_3;
  *param_3 = uVar1;
  uVar1 = uVar1 ^ *param_2;
  *param_2 = uVar1 >> 0x19 | uVar1 << 7;
  return;
}

