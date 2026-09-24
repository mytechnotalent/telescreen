
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_CIPHER_CTX_ctrl(EVP_CIPHER_CTX *ctx,int type,int arg,void *ptr)

{
  int iVar1;
  
  iVar1 = (*(code *)PTR_EVP_CIPHER_CTX_ctrl_004200c8)((int)ctx,type,arg);
  return iVar1;
}

