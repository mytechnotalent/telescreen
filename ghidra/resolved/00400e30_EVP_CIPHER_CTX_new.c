
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

EVP_CIPHER_CTX * EVP_CIPHER_CTX_new(void)

{
  EVP_CIPHER_CTX *pEVar1;
  
  pEVar1 = (EVP_CIPHER_CTX *)(*(code *)PTR_EVP_CIPHER_CTX_new_00420078)();
  return pEVar1;
}

