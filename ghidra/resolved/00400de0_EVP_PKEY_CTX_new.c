
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

EVP_PKEY_CTX * EVP_PKEY_CTX_new(EVP_PKEY *pkey,ENGINE *e)

{
  EVP_PKEY_CTX *pEVar1;
  
  pEVar1 = (EVP_PKEY_CTX *)(*(code *)PTR_EVP_PKEY_CTX_new_00420050)();
  return pEVar1;
}

