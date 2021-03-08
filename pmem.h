int open_pmem_device(const char *daxname);

void *alloc_four_pmem_buffers(void **buf1_, size_t size1,
                              void **buf2_, size_t size2,
                              void **buf3_, size_t size3,
                              void **buf4_, size_t size4, int memfd);
void free_pmem_buffers(void *pmem_buf);

size_t align_up1gb(size_t size);
