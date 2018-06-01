////////////////////////////////////////////////////////////////////////////////
// NOTE : Generated by slothjson. It is NOT supposed to modify this file.
////////////////////////////////////////////////////////////////////////////////
#ifndef __userinfo_20180601161551_h__
#define __userinfo_20180601161551_h__

#include "slothjson.h"

using namespace slothjson;

struct CUserInfo
{
    int64_t id;
    std::string name;
    std::string nick_name;
    std::string phone;

    void skip_id() { __skip_id = true; }
    void skip_name() { __skip_name = true; }
    void skip_nick_name() { __skip_nick_name = true; }
    void skip_phone() { __skip_phone = true; }

    bool json_has_id() const { return __json_has_id; }
    bool json_has_name() const { return __json_has_name; }
    bool json_has_nick_name() const { return __json_has_nick_name; }
    bool json_has_phone() const { return __json_has_phone; }

    CUserInfo();
    CUserInfo& operator=(const CUserInfo& obj_val);
    bool operator==(const CUserInfo& obj_val) const;
    bool encode(allocator_t& alloc, rapidjson::Value& json_val) const;
    bool decode(const rapidjson::Value& json_val);

private:
    bool __skip_id;
    bool __skip_name;
    bool __skip_nick_name;
    bool __skip_phone;

    bool __json_has_id;
    bool __json_has_name;
    bool __json_has_nick_name;
    bool __json_has_phone;
};

bool encode(const CUserInfo& obj_val, allocator_t& alloc, rapidjson::Value& json_val);
bool decode(const rapidjson::Value& json_val, CUserInfo& obj_val);


#endif // __userinfo_20180601161551_h__