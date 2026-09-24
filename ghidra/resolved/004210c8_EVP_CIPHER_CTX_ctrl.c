
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_CIPHER_CTX_ctrl(EVP_CIPHER_CTX *ctx,int type,int arg,void *ptr)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

