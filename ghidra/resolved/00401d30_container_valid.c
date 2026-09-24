
bool FUN_00401d30(int *param_1,ulong param_2)

{
  if (param_1 == (int *)0x0 || param_2 < 0x10) {
    return false;
  }
  if (*param_1 != 0x1b8421) {
    return param_1 == (int *)0x0 || param_2 < 0x10;
  }
  return *(long *)(param_1 + 2) == 0x6461656870697a67;
}

