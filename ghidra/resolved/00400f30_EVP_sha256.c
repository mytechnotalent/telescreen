
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

EVP_MD * EVP_sha256(void)

{
  EVP_MD *pEVar1;
  
  pEVar1 = (EVP_MD *)(*(code *)PTR_EVP_sha256_004200f8)();
  return pEVar1;
}

