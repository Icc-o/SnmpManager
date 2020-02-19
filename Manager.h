#include <vector>
#include "Host.h"
#include <functional>
//封装一下snmp操作的流程，对外提供一些接口

class Manager
{
public:
    Manager();
    ~Manager();

    void set_func(std::function<bool(Host, snmp_pdu*)> f);
    
    void run();
    void stop();
    void add_host(Host* h);
    void handle_data(Host* h, snmp_pdu* p);
    void set_interval(uint32_t i);

    //再添加一个添加主机和oid的便捷接口
    //bool add_host_info(std::vector<char*> hosts, std::vector<char*> oids);

private:
    void init_sessions();
    void asyn_send();
    void wait_request();

private:
    std::vector<Host*>          m_hosts;
    std::function<bool(Host,snmp_pdu*)>   m_handleFunc;   //当接收到返回数据时的处理函数
    bool                        m_running;
    uint32_t                    m_sendCount;
    uint32_t                    m_loopInterval;
};