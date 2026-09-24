
uint FUN_00401e40(byte *param_1,long param_2)

{
  byte *pbVar1;
  uint uVar2;
  uint uVar3;
  char cVar4;
  uint uVar5;
  
  if (param_2 != 0) {
    pbVar1 = param_1 + param_2;
    uVar3 = 0xffff;
    do {
      uVar5 = (uint)*param_1;
      cVar4 = '\b';
      do {
        uVar2 = uVar3 ^ uVar5 << 8;
        uVar3 = (uVar3 & 0x7fff) << 1;
        cVar4 = cVar4 + -1;
        uVar5 = (uVar5 & 0x7f) << 1;
        if ((uVar2 & 0x8000) != 0) {
          uVar3 = uVar3 ^ 0x1021;
        }
      } while (cVar4 != '\0');
      param_1 = param_1 + 1;
    } while (pbVar1 != param_1);
    return uVar3;
  }
  return 0xffff;
}

