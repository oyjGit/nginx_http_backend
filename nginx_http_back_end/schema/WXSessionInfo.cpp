////////////////////////////////////////////////////////////////////////////////
// NOTE : Generated by slothjson. It is NOT supposed to modify this file.
////////////////////////////////////////////////////////////////////////////////
#include "WXSessionInfo.h"


WXSessionInfo::WXSessionInfo()
{
    __skip_session_key = false;
    __json_has_session_key = false;

    expires_in = 0;
    __skip_expires_in = false;
    __json_has_expires_in = false;

    __skip_openid = false;
    __json_has_openid = false;
}

WXSessionInfo& WXSessionInfo::operator=(const WXSessionInfo& obj_val)
{
    this->session_key = obj_val.session_key;
    this->expires_in = obj_val.expires_in;
    this->openid = obj_val.openid;
    return *this;
}

bool WXSessionInfo::operator==(const WXSessionInfo& obj_val) const
{
    if (!(this->session_key == obj_val.session_key)) return false;
    if (!(this->expires_in == obj_val.expires_in)) return false;
    if (!(this->openid == obj_val.openid)) return false;
    return true;
}

bool WXSessionInfo::encode(allocator_t& alloc, rapidjson::Value& json_val) const
{
    do
    {
        json_val.SetObject();
        if (!__skip_session_key && !encode_field(session_key, "session_key", alloc, json_val)) break;
        if (!__skip_expires_in && !encode_field(expires_in, "expires_in", alloc, json_val)) break;
        if (!__skip_openid && !encode_field(openid, "openid", alloc, json_val)) break;

        return true;
    } while (0);

    return false;
}

bool WXSessionInfo::decode(const rapidjson::Value& json_val)
{
    do
    {
        if (!decode_field(json_val, "session_key", session_key, __json_has_session_key)) break;
        if (!decode_field(json_val, "expires_in", expires_in, __json_has_expires_in)) break;
        if (!decode_field(json_val, "openid", openid, __json_has_openid)) break;

        return true;
    } while (0);

    return false;
}

bool encode(const WXSessionInfo& obj_val, allocator_t& alloc, rapidjson::Value& json_val)
{
    return obj_val.encode(alloc, json_val);
}

bool decode(const rapidjson::Value& json_val, WXSessionInfo& obj_val)
{
    return obj_val.decode(json_val);
}
