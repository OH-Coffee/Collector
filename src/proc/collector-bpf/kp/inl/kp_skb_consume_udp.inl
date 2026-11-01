{
    struct sk_buff* skb = (struct sk_buff*)PT_REGS_PARM2(ctx);
    
    __u16 skb_tp_hdr = 0;
    unsigned char *skb_head, *skb_data;
    if (bpf_probe_read_kernel(&skb_tp_hdr, sizeof(skb_tp_hdr), ((char*)skb + SKB_OFFSET_TPH))) {
        return 0;
    }

    if (bpf_probe_read_kernel(&skb_head, sizeof(skb_head), ((char*)skb + SKB_OFFSET_HEAD))) {
        return 0;
    }

    __be16 src_port = 0;
    if (bpf_probe_read_kernel(&src_port, sizeof(src_port), &((struct udphdr*)(skb_head + skb_tp_hdr))->source)) {
        return 0;
    }

    if (53 != __bpf_ntohs(src_port)) {
        return 0;
    }

    struct udp_dev_scratch dev_scratch;
    if (bpf_probe_read_kernel(&dev_scratch, sizeof(dev_scratch), &skb->dev_scratch)) {
        return 0;
    }

    const unsigned int udp_data_len = dev_scratch.len;
    if (udp_data_len <= 12) { // sizeof(dns_header)
        return 0;
    }

    if (bpf_probe_read_kernel(&skb_data, sizeof(skb_data), ((char*)skb + SKB_OFFSET_DATA))) {
        return 0;
    }

    if (udp_data_len > BUF_LEN_RECV_DATA) {
        return 0;
    }

    EventSkbConsumeUdpPrivate *udata = NULL;
    __u32 zero = 0;
    udata = bpf_map_lookup_elem(&buf_for_skb_consume_udp, &zero);
    if (NULL == udata) {
        return 0;
    }

    // map.>~1024=<A call to built-in function 'memset'is not supported.>
    // call to 'memset' is not supported:This occurs when you attempt to zero out a large amount of memory, e.g. 1024 bytes
    // __builtin_memset(&udata->data, 0, sizeof(udata->data));

    unsigned int need_copy_len = udp_data_len;
    if (!dev_scratch.is_linear) {
        // 非线性缓冲区，限制了nr_frags == 1 && frag_list == NULL
        // 并且只在arm64上实现物理内存页的复制
        
        unsigned int skb_end = 0;
        if(bpf_probe_read_kernel(&skb_end, sizeof(skb_end), ((char*)skb + SKB_OFFSET_END))) {
            return 0;
        }

        __u8 nr_frags = 0;
        if(bpf_probe_read_kernel(&nr_frags, sizeof(nr_frags), &((struct skb_shared_info*)(skb_head + skb_end))->nr_frags)) {
            return 0;
        }
        if (1 != nr_frags) {
            //char ____fmt[] = "nr_frags:%d";
            //bpf_trace_printk(____fmt, sizeof(____fmt), (int)nr_frags);
            return 0;
        }
        
        struct sk_buff* frag_list = NULL;
        if(bpf_probe_read_kernel(&frag_list, sizeof(frag_list), &((struct skb_shared_info*)(skb_head + skb_end))->frag_list)) {
            return 0;
        }
        if (NULL != frag_list) {
            //char ____fmt[] = "nr_frags:%p";
            //bpf_trace_printk(____fmt, sizeof(____fmt), frag_list);
            return 0;
        }
        
        unsigned int skb_len = 0, skb_data_len = 0;
        if (bpf_probe_read_kernel(&skb_len, sizeof(skb_len), ((char*)skb + SKB_OFFSET_LEN))) {
            return 0;
        }
        if (bpf_probe_read_kernel(&skb_data_len, sizeof(skb_data_len), ((char*)skb + SKB_OFFSET_DATA_LEN))) {
            return 0;
        }

        int start = skb_len - skb_data_len;
        int copy = start, end, offset = 0;
        // 一阶段，拷贝头
        if (copy > 0) {
            if (copy > need_copy_len) {
                copy = need_copy_len;
            }

            copy &= BUF_LEN_RECV_DATA - 1; // bypass size check
            if (bpf_probe_read_kernel(udata->data.body.data + offset, copy, skb_data + offset)) {
                return 0;
            }
            need_copy_len -= copy;
            if (0 == need_copy_len) {
                goto copy_out;
            }
            offset += copy;
        }

        skb_frag_t frag;
        if (bpf_probe_read_kernel(&frag, sizeof(frag), &((struct skb_shared_info*)(skb_head + skb_end))->frags[0])) {
            return 0;
        }

        end = start + *(__u32*)(((char*)&frag) + SKB_FRAG_OFFSET_LEN);
        copy = end - offset;
        if (copy > 0) {
            if (copy > need_copy_len) {
                copy = need_copy_len;
            }

            char* kaddr = (char*)KMAP((*(void**)&frag));
            unsigned int copy_off = udp_data_len - need_copy_len;

            copy_off &= BUF_LEN_RECV_DATA - 1;
            copy &= BUF_LEN_RECV_DATA - 1;
            if (bpf_probe_read_kernel(udata->data.body.data + copy_off, copy, 
                    kaddr + *(__u32*)(((char*)&frag) + SKB_FRAG_OFFSET_OFFSET) + offset - start)) {
                return 0;
            }

            need_copy_len -= copy;
            if (0 == need_copy_len) {
                goto copy_out;
            }
            offset += copy;
        }
        start = end;
    }
    else {
        int copy = need_copy_len, offset = 0;
        copy &= BUF_LEN_RECV_DATA - 1;
        if (bpf_probe_read_kernel(udata->data.body.data + offset, copy, skb_data + offset)) {
            return 0;
        }
        need_copy_len -= copy;
    }

copy_out:
    if (0 != need_copy_len) {
        return 0;
    }
    
    __u64 pid_tgid = bpf_get_current_pid_tgid();
    __u64 uid_gid  = bpf_get_current_uid_gid();

    udata->data.head.event_type       = EVENT_TYPE_SKB_CONSUME_UDP;
    udata->data.head.event_producer   = EP_BPF_KPROBE;
    udata->data.head.event_body_len   = sizeof(udata->data.body);
    udata->data.head.event_process.pid = pid_tgid >> 32;
    udata->data.head.event_process.uid = uid_gid  & 0xffffffff;

    udata->data.body.reserve1       = 0;
    udata->data.body.reserve2       = dev_scratch.is_linear;
    udata->data.body.data_len       = udp_data_len;
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &udata->data, sizeof(udata->data));
    return 0;
}