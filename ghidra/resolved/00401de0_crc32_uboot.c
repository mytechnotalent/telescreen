
uint FUN_00401de0(byte *param_1,long param_2)

{
  byte *pbVar1;
  char cVar2;
  uint uVar3;
  
  if (param_2 != 0) {
    pbVar1 = param_1 + param_2;
    uVar3 = 0xffffffff;
    do {
      cVar2 = '\b';
      uVar3 = uVar3 ^ *param_1;
      do {
        cVar2 = cVar2 + -1;
        uVar3 = -(uVar3 & 1) & 0xedb88320 ^ uVar3 >> 1;
      } while (cVar2 != '\0');
      param_1 = param_1 + 1;
    } while (param_1 != pbVar1);
    return ~uVar3;
  }
  return 0;
}

