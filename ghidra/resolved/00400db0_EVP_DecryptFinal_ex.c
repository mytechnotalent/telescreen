
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_DecryptFinal_ex(EVP_CIPHER_CTX *ctx,uchar *outm,int *outl)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_DecryptFinal_ex_00420038)((int)ctx);
  return iVar1;
}

