
char FUN_00402560(int *param_1,ulong param_2)

{
  char cVar1;
  
  cVar1 = '\0';
  if (param_1 != (int *)0x0 && 3 < param_2) {
    cVar1 = '\x01';
    if ((*param_1 != -0x15fffaeb) &&
       (cVar1 = (*param_1 == 0x1b8421) + '\x02', (short)*param_1 == 0x1985)) {
      cVar1 = '\x04';
    }
  }
  return cVar1;
}

