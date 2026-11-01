#include "bpf_public_header.h"

#define QSTR_CACHE_NUM 10

NEW_PERCPU_ARRAY(EventVfsRename, 1, buf_for_vfs_rename);

static inline bool get_data(EventType i_type, const char *i_oldname, const char *i_newname, EventVfsRename *io_data)
{
    bool r_bool = false;

    do {
        EventHead           *head_ptr = &io_data->head;
        EventBodyVfsRename  *body_ptr = &io_data->body;
        
        uint64_t pid_tgid = bpf_get_current_pid_tgid();
        uint64_t uid_gid  = bpf_get_current_uid_gid();

        head_ptr->event_type       = i_type;
        head_ptr->event_producer   = EP_BPF_KPROBE;
        head_ptr->event_body_len   = sizeof(io_data->body);

        head_ptr->event_process.pid = pid_tgid >> 32;
        head_ptr->event_process.uid = uid_gid  & 0xffffffff;

        if(i_oldname != NULL) { bpf_probe_read(body_ptr->oldname, BUF_LEN_RENAME_PATH, i_oldname); }
        if(i_newname != NULL) { bpf_probe_read(body_ptr->newname, BUF_LEN_RENAME_PATH, i_newname); } 

        r_bool = true;
    } while (false);

    return r_bool;
}

static inline uint64_t get_dev_num(const struct dentry *dentry_ptr)
{
    uint64_t ret_value = 0;

    struct super_block *blk_ptr = NULL;
    if(NULL != dentry_ptr && 0 == bpf_probe_read_kernel(&blk_ptr, sizeof(blk_ptr), &dentry_ptr->d_sb)) {
        if(NULL != blk_ptr) { bpf_probe_read_kernel(&ret_value, sizeof(ret_value), &blk_ptr->s_dev); }
    }

    return ret_value;
}

static inline uint64_t get_ino_num(const struct dentry *dentry_ptr)
{
    uint64_t ret_value = 0;

    struct inode *ino_ptr = NULL;
    if(NULL != dentry_ptr && 0 == bpf_probe_read_kernel(&ino_ptr, sizeof(ino_ptr), &dentry_ptr->d_inode)) {
        if(NULL != ino_ptr) { bpf_probe_read_kernel(&ret_value, sizeof(ret_value), &ino_ptr->i_ino); }
    }

    return ret_value;
}

static inline struct dentry * get_path(struct dentry *dentry_ptr, char *o_buf)
{
    do {
        if(NULL == dentry_ptr || NULL == o_buf) { break; }

        uint64_t max    = BUF_LEN_RENAME_PATH / 2;
        uint64_t offset = 0;

        #pragma unroll(QSTR_CACHE_NUM)
        for (int i = 0; i < QSTR_CACHE_NUM; i++) {
            if (offset >= max) { break; }

#ifdef ARCHITECTURE_ARM
            uint64_t      ino_num = 0;
            struct inode *ino_ptr = NULL;
            if(0 == bpf_probe_read_kernel(&ino_ptr, sizeof(ino_ptr), &dentry_ptr->d_inode)) {
                if(NULL != ino_ptr) { bpf_probe_read_kernel(&ino_num, sizeof(ino_num), &ino_ptr->i_ino); }
            }
            
            uint64_t            dev_num = 0;
            struct super_block *blk_ptr = NULL;
            if(0 == bpf_probe_read_kernel(&blk_ptr, sizeof(blk_ptr), &dentry_ptr->d_sb)) {
                if(NULL != blk_ptr) { bpf_probe_read_kernel(&dev_num, sizeof(dev_num), &blk_ptr->s_dev); }
            }
#endif

            struct qstr quick_str = {};
            if(0 != bpf_probe_read_kernel(&quick_str, sizeof(struct qstr), &dentry_ptr->d_name)) {
                break;
            }

            if(quick_str.len == 1) {
                char buf[2] = { 0 };

                if(bpf_probe_read_str(buf, sizeof(buf), quick_str.name) <= 0) {
                    break;
                }
                if(buf[0] == '/') { break; }
            }

            const uint64_t size = max - offset;

            int read_num = bpf_probe_read_str(&o_buf[offset], size, quick_str.name);
            if(read_num > 0) {
                if((offset += read_num) < max) { o_buf[offset - 1] = '/'; }
            }

            if(0 != bpf_probe_read_kernel(&dentry_ptr, sizeof(dentry_ptr), &dentry_ptr->d_parent)) {
                break;
            }
        }
        if(offset > 0 && offset < max) { o_buf[offset - 1] = '\0'; }
    } while (false);

    return dentry_ptr;
}

SEC("kprobe/vfs_rename")
int callback_for_enter_vfs_rename(struct pt_regs *i_args)
{
    int r_code = 0;
    
    do {
        uint32_t zero = 0;
        EventVfsRename *data = bpf_map_lookup_elem(&buf_for_vfs_rename, &zero);
        if (NULL == data) { break; }

        struct dentry * old_dentry = (struct dentry *)PT_REGS_PARM2(i_args);
        struct dentry * new_dentry = (struct dentry *)PT_REGS_PARM4(i_args);

        old_dentry = get_path(old_dentry, data->body.oldname);
        new_dentry = get_path(new_dentry, data->body.newname);

        uint64_t dev_num = get_dev_num(old_dentry);
        uint64_t ino_num = get_ino_num(old_dentry);
        
        data->body.mount.inode   = ino_num;
        data->body.mount.dev_maj = MAJOR((dev_t)dev_num);
        data->body.mount.dev_min = MINOR((dev_t)dev_num);
        
        if(get_data(EVENT_TYPE_VFS_RENAME, NULL, NULL, data)) {
            bpf_perf_event_output(i_args, &events, BPF_F_CURRENT_CPU, data, sizeof(*data));
        }
    } while (false);

    return r_code;
}