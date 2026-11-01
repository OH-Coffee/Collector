#ifndef SKB_OFFSET_H
#define SKB_OFFSET_H

#define SKB_OFFSET_TPH          178
#define SKB_OFFSET_HEAD         192
#define SKB_OFFSET_DATA         200
#define SKB_OFFSET_END          188
#define SKB_OFFSET_LEN          112
#define SKB_OFFSET_DATA_LEN     116
#define SKB_FRAG_OFFSET_LEN     8
#define SKB_FRAG_OFFSET_OFFSET  12

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