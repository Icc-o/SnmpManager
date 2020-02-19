#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <string>

struct cc_oid{
    oid     o[MAX_OID_LEN];
    size_t  length; 
};

//封装查询主机信息
class Host{
public:
    std::string             hostName;       //NICKNAME
    std::string             ip;             //IP
    struct snmp_session     *pSession;
    std::vector<cc_oid>     listOid;
    Host(){}
    ~Host(){}
};