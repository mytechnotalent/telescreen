
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_PKEY_derive(EVP_PKEY_CTX *ctx,uchar *key,size_t *keylen)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_PKEY_derive_00420098)((int)ctx);
  return iVar1;
}

