
bool FUN_00401fe0(undefined8 param_1,undefined8 param_2)

{
  int iVar1;
  EVP_PKEY *pkey;
  long local_8;
  
  pkey = (EVP_PKEY *)EVP_PKEY_new_raw_private_key(0x43f,0,param_1,0x20);
  local_8 = 0x20;
  if ((pkey != (EVP_PKEY *)0x0) &&
     (iVar1 = EVP_PKEY_get_raw_public_key(pkey,param_2,&local_8), iVar1 == 1)) {
    EVP_PKEY_free(pkey);
    return local_8 == 0x20;
  }
  EVP_PKEY_free(pkey);
  return false;
}

