
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx,uchar *out,int *outl)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_EncryptFinal_ex_004200e0)((int)ctx);
  return iVar1;
}

