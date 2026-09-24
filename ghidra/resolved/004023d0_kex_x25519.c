
bool FUN_004023d0(undefined8 param_1,undefined8 param_2,uchar *param_3)

{
  int iVar1;
  EVP_PKEY *pkey;
  EVP_PKEY *peer;
  EVP_PKEY_CTX *ctx;
  size_t local_8;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_private_key(0x40a,0,param_1,0x20);
  peer = (EVP_PKEY *)EVP_PKEY_new_raw_public_key(0x40a,0,param_2,0x20);
  if (pkey == (EVP_PKEY *)0x0 || peer == (EVP_PKEY *)0x0) {
    ctx = (EVP_PKEY_CTX *)0x0;
    local_8 = 0x20;
  }
  else {
    ctx = EVP_PKEY_CTX_new(pkey,(ENGINE *)0x0);
    local_8 = 0x20;
    if (((ctx != (EVP_PKEY_CTX *)0x0) && (iVar1 = EVP_PKEY_derive_init(ctx), iVar1 == 1)) &&
       (iVar1 = EVP_PKEY_derive_set_peer(ctx,peer), iVar1 == 1)) {
      iVar1 = EVP_PKEY_derive(ctx,param_3,&local_8);
      EVP_PKEY_CTX_free(ctx);
      EVP_PKEY_free(pkey);
      EVP_PKEY_free(peer);
      if (iVar1 != 1) {
        return false;
      }
      return local_8 == 0x20;
    }
  }
  EVP_PKEY_CTX_free(ctx);
  EVP_PKEY_free(pkey);
  EVP_PKEY_free(peer);
  return false;
}

