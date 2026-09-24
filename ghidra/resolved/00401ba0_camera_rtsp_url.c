
bool FUN_00401ba0(long param_1,uint param_2,char *param_3,ulong param_4)

{
  int iVar1;
  
  if ((param_1 == 0 || param_4 == 0) || param_3 == (char *)0x0) {
    return false;
  }
  iVar1 = snprintf(param_3,param_4,"%s://%s:%u%s",&DAT_004027d8,param_1,(ulong)(param_2 & 0xffff),
                   "/stream");
  return (ulong)(long)iVar1 < param_4;
}

