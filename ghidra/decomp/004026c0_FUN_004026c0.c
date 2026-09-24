
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004026c0(char *param_1,undefined8 *param_2)

{
  size_t __n;
  
  _DAT_00420180 = 0;
  uRam0000000000420188 = 0;
  _DAT_00420190 = 0;
  uRam0000000000420198 = 0;
  _DAT_004201a0 = 0;
  uRam00000000004201a8 = 0;
  _DAT_004201b0 = 0;
  uRam00000000004201b8 = 0;
  DAT_004201c0 = 0;
  __n = strlen(param_1);
  if (0x1f < __n) {
    __n = 0x1f;
  }
  memcpy(&DAT_00420180,param_1,__n);
  _DAT_004201a0 = *param_2;
  uRam00000000004201a8 = param_2[1];
  uRam00000000004201b8 = param_2[3];
  _DAT_004201b0 = param_2[2];
  FUN_00401c80();
  return;
}

