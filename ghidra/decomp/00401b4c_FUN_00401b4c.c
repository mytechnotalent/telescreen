
void FUN_00401b4c(undefined4 param_1,undefined4 *param_2)

{
  *param_2 = param_1;
  *(undefined8 *)(param_2 + 1) = 0;
  *(undefined8 *)(param_2 + 3) = 0;
  param_2[5] = 0;
  return;
}

