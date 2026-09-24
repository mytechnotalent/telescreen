
/* WARNING: Control flow encountered bad instruction data */
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx,EVP_CIPHER *cipher,ENGINE *impl,uchar *key,uchar *iv)

{
                    /* WARNING: Bad instruction - Truncating control flow here */
  halt_baddata();
}

