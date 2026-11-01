
#include <CoreFoundation/CoreFoundation.h>

#include "file/file_api.hpp"

static const std::map<FileSignInfoType, const CFStringRef> g_sign_type_map = {
    { FSI_TYPE_SIGN_ID,     kSecCodeInfoIdentifier      },
    { FSI_TYPE_TEAM_ID,     kSecCodeInfoTeamIdentifier  },
};

FileSignInfo file_sign_info(const std::string &i_path, FileSignInfoType i_types)
{
    FileSignInfo r_info = {};
    
    CFURLRef         cfr_url     = nil;
    CFStringRef      cfr_path    = nil;
    SecStaticCodeRef static_code = nil;

    do { @autoreleasepool {
        OSStatus r_status = errSecUnimplemented;
        
        cfr_path = CFStringCreateWithCStringNoCopy(nil, i_path.c_str(), kCFStringEncodingUTF8, kCFAllocatorNull);
        if(nil == cfr_path) {
            break;
        }
        
        cfr_url = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfr_path, kCFURLPOSIXPathStyle, false);
        if(nil == cfr_url) {
            break;
        }
        
        r_status = SecStaticCodeCreateWithPath(cfr_url, kSecCSDefaultFlags, &static_code);
        if(errSecSuccess != r_status) {
            break;
        }
        
        CFDictionaryRef cfr_info = nil;
        r_status = SecCodeCopySigningInformation(static_code, kSecCSSigningInformation, &cfr_info);
        if(errSecSuccess != r_status) {
            if(nil != cfr_info) { CFRelease(cfr_info); } break;
        }
        
        // __bridge_transfer后，对象由ARC释放
        NSDictionary *sign_info = (__bridge_transfer NSDictionary *)cfr_info;
        for (uint64_t offset = 0; offset < (sizeof(i_types) * 8); ++offset) {
            auto type = (FileSignInfoType)((uint64_t)0x01 << offset);
            
            if(IS_SET(i_types, type)) {
                if(const auto &it = g_sign_type_map.find(type); g_sign_type_map.end() != it) {
                    NSString *val = sign_info[(__bridge NSString *)it->second];
                    if(nil != val) {
                        r_info[type] = [val UTF8String];
                    }
                }
            }
        }
    } } while (false);
    
    if(nil != cfr_url)      { CFRelease(cfr_url);     cfr_url     = nil; }
    if(nil != cfr_path)     { CFRelease(cfr_path);    cfr_path    = nil; }
    if(nil != static_code)  { CFRelease(static_code); static_code = nil; }
    
    return r_info;
}
