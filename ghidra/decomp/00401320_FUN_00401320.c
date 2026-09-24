
bool FUN_00401320(EVP_CIPHER_CTX *param_1,uchar *param_2,uchar *param_3)

{
  int iVar1;
  int iVar2;
  int local_4;
  
  local_4 = 0;
  iVar1 = EVP_DecryptUpdate(param_1,param_2,&local_4,param_3,0x30);
  iVar2 = local_4;
  if (iVar1 == 1) {
    iVar1 = EVP_CIPHER_CTX_ctrl(param_1,0x11,0x10,param_3 + 0x30);
    if (iVar1 == 1) {
      iVar2 = EVP_DecryptFinal_ex(param_1,param_2 + iVar2,&local_4);
      return iVar2 == 1;
    }
  }
  return false;
}

