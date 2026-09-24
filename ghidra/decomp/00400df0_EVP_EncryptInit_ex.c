
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx,EVP_CIPHER *cipher,ENGINE *impl,uchar *key,uchar *iv)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_EncryptInit_ex_00420058)((int)ctx);
  return iVar1;
}

