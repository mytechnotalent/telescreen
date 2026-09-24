
char FUN_004025e0(long param_1,ulong param_2,int param_3,void *param_4,ulong param_5)

{
  char cVar1;
  long lVar2;
  
  if (param_3 == 1) {
    lVar2 = 0;
  }
  else if (param_3 == 2) {
    lVar2 = 1;
  }
  else if (param_3 == 3) {
    lVar2 = 2;
  }
  else {
    if (param_3 != 4) {
      return (param_2 < 0x1000000 && (param_4 != (void *)0x0 && param_1 != 0)) + '\x01';
    }
    lVar2 = 3;
  }
  cVar1 = '\x01';
  if (param_4 != (void *)0x0 && param_1 != 0) {
    cVar1 = '\x02';
    if (param_2 < 0x1000000) {
      return cVar1;
    }
    if ((uint)(&DAT_00402858)[lVar2 * 6] <= param_5) {
      memcpy(param_4,(void *)(param_1 + (ulong)(uint)(&DAT_00402854)[lVar2 * 6]),
             (ulong)(uint)(&DAT_00402858)[lVar2 * 6]);
      return '\0';
    }
  }
  return cVar1;
}

