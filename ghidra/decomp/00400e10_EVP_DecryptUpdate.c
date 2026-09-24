
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_DecryptUpdate(EVP_CIPHER_CTX *ctx,uchar *out,int *outl,uchar *in,int inl)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_DecryptUpdate_00420068)((int)ctx);
  return iVar1;
}

