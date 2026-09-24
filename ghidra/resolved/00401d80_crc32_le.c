
ulong FUN_00401d80(ulong param_1,byte *param_2,long param_3)

{
  byte *pbVar1;
  char cVar2;
  
  if (param_3 != 0) {
    pbVar1 = param_2 + param_3;
    do {
      cVar2 = '\b';
      param_1 = (ulong)((uint)param_1 ^ (uint)*param_2);
      do {
        cVar2 = cVar2 + -1;
        param_1 = (ulong)(-((uint)param_1 & 1) & 0xedb88320 ^ (uint)param_1 >> 1);
      } while (cVar2 != '\0');
      param_2 = param_2 + 1;
    } while (param_2 != pbVar1);
  }
  return param_1;
}

