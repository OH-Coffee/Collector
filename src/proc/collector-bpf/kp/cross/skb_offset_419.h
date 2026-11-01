#ifndef SKB_OFFSET_H
#define SKB_OFFSET_H

#define SKB_OFFSET_TPH          194
#define SKB_OFFSET_HEAD         208
#define SKB_OFFSET_DATA         216
#define SKB_OFFSET_END          204
#define SKB_OFFSET_LEN          128
#define SKB_OFFSET_DATA_LEN     132
#define SKB_FRAG_OFFSET_LEN     12
#define SKB_FRAG_OFFSET_OFFSET  8

#else
#undef SKB_OFFSET_H

#undef SKB_OFFSET_TPH
#undef SKB_OFFSET_HEAD
#undef SKB_OFFSET_DATA
#undef SKB_OFFSET_END
#undef SKB_OFFSET_LEN
#undef SKB_OFFSET_DATA_LEN
#undef SKB_FRAG_OFFSET_LEN
#undef SKB_FRAG_OFFSET_OFFSET

#endif
