
bool FUN_00402160(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  bool bVar1;
  int iVar2;
  EVP_PKEY *pkey;
  EVP_MD_CTX *ctx;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_public_key(0x43f,0,param_1,0x20);
  ctx = (EVP_MD_CTX *)EVP_MD_CTX_new();
  if ((pkey == (EVP_PKEY *)0x0 || ctx == (EVP_MD_CTX *)0x0) ||
     (iVar2 = EVP_DigestVerifyInit(ctx,(EVP_PKEY_CTX **)0x0,(EVP_MD *)0x0,(ENGINE *)0x0,pkey),
     iVar2 != 1)) {
    bVar1 = false;
  }
  else {
    iVar2 = EVP_DigestVerify(ctx,param_4,0x40,param_2,param_3);
    bVar1 = iVar2 == 1;
  }
  EVP_MD_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  return bVar1;
}

