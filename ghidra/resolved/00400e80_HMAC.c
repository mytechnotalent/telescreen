
/* WARNING: Unknown calling convention -- yet parameter storage is locked */

uchar * HMAC(EVP_MD *evp_md,void *key,int key_len,uchar *d,size_t n,uchar *md,uint *md_len)

{
  uchar *puVar1;
  
  puVar1 = (uchar *)(*(code *)PTR_HMAC_004200a0)(evp_md,key,key_len);
  return puVar1;
}

