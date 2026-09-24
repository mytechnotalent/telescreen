
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

EVP_CIPHER * EVP_aes_256_gcm(void)

{
  EVP_CIPHER *pEVar1;
  
  pEVar1 = (EVP_CIPHER *)(*(code *)PTR_EVP_aes_256_gcm_004200d0)();
  return pEVar1;
}

