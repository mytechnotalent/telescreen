
void FUN_004012c0(char *param_1,long param_2,undefined4 *param_3)

{
  undefined8 uVar1;
  long lVar2;
  
  uVar1 = s_expand_32_byte_k_stream_004027c0._8_8_;
  lVar2 = 0;
  *(undefined8 *)param_1 = s_expand_32_byte_k_stream_004027c0._0_8_;
  *(undefined8 *)(param_1 + 8) = uVar1;
  do {
    *(undefined4 *)(param_1 + lVar2 + 0x10) = *(undefined4 *)(param_2 + lVar2);
    lVar2 = lVar2 + 4;
  } while (lVar2 != 0x20);
  *(undefined4 *)(param_1 + 0x30) = *param_3;
  *(undefined4 *)(param_1 + 0x34) = param_3[1];
  *(undefined4 *)(param_1 + 0x38) = param_3[2];
  *(undefined4 *)(param_1 + 0x3c) = param_3[3];
  return;
}

