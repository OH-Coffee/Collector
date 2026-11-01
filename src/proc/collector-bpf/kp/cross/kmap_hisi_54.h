#ifndef KMAP_H
#define KMAP_H

#define VMEMMAP_START      0xfffffffeffe00000ULL
#define PAGE_OFFSET        0xffffff8000000000ULL
#define SIZEOF_STRUCT_PAGE 64
#define PAGE_SIZE          0x1000

#define KMAP(_x) ({						\
	u64 __idx = ((u64)_x - VMEMMAP_START) / SIZEOF_STRUCT_PAGE;\
	u64 __addr = PAGE_OFFSET + (__idx * PAGE_SIZE);			\
    (void*)__addr; \
})

#else
#undef KMAP_H

#undef VMEMMAP_START
#undef PAGE_OFFSET
#undef SIZEOF_STRUCT_PAGE
#undef PAGE_SIZE

#undef KMAP

#endif