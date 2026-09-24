
bool FUN_0040205c(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  int iVar1;
  EVP_PKEY *pkey;
  EVP_MD_CTX *ctx;
  long lStack_8;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_private_key(0x43f,0,param_1,0x20);
  ctx = (EVP_MD_CTX *)EVP_MD_CTX_new();
  lStack_8 = 0x40;
  if (pkey != (EVP_PKEY *)0x0 && ctx != (EVP_MD_CTX *)0x0) {
    iVar1 = EVP_DigestSignInit(ctx,(EVP_PKEY_CTX **)0x0,(EVP_MD *)0x0,(ENGINE *)0x0,pkey);
    if (iVar1 != 1) {
      EVP_MD_CTX_free(ctx);
      EVP_PKEY_free(pkey);
      return false;
    }
    iVar1 = EVP_DigestSign(ctx,param_4,&lStack_8,param_2,param_3);
    if (iVar1 == 1) {
      EVP_MD_CTX_free(ctx);
      EVP_PKEY_free(pkey);
      return lStack_8 == 0x40;
    }
  }
  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  return false;
}

