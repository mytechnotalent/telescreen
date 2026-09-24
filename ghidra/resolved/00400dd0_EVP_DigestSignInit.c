
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_DigestSignInit(EVP_MD_CTX *ctx,EVP_PKEY_CTX **pctx,EVP_MD *type,ENGINE *e,EVP_PKEY *pkey)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_DigestSignInit_00420048)((int)ctx);
  return iVar1;
}

