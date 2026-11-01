#ifndef KMAP_H
#define KMAP_H

#define VMEMMAP_START      0xffffffbf00000000ULL
#define PAGE_OFFSET        0xffffffc000000000ULL
#define SIZEOF_STRUCT_PAGE 64
#define PAGE_SIZE          0x1000

#define KMAP(_x) ((void *)((((u64)(_x) & ~VMEMMAP_START) * PAGE_SIZE / SIZEOF_STRUCT_PAGE) | PAGE_OFFSET))

#else
#undef KMAP_H

#undef VMEMMAP_START
#undef PAGE_OFFSET
#undef SIZEOF_STRUCT_PAGE
#undef PAGE_SIZE

#undef KMAP

#endif