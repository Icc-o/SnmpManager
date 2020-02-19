#include "Manager.h"
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <sys/select.h>
#include <string>
#include <thread>
#include <iostream>

using namespace std;

//可以考虑用单例模式
Manager* pManager = nullptr;

//用作回调
int async_response(int operation, struct snmp_session *sp, int reqid,struct snmp_pdu *pdu, void *magic)
{
    Host* h = static_cast<Host*>(magic);
    if(pManager)
    {
        pManager->handle_data(h,pdu);
    }
    else
    {
        cout<<"pManager == nullptr"<<endl;
        return 1;
    }
    return 0;
}

Manager::Manager()
{
    m_running = false;
    m_hosts.clear();
    pManager = this;
    m_loopInterval = 1;
}

Manager::~Manager()
{

}

void Manager::add_host(Host* h)
{
    m_hosts.push_back(h);
}

void Manager::handle_data(Host* h, snmp_pdu* p)
{
    --m_sendCount;
    m_handleFunc(*h, p);
}

void Manager::set_interval(uint32_t i)
{
    m_loopInterval = i;
}

void Manager::init_sessions()
{
    
    for(auto &h : m_hosts)
    {
        netsnmp_session     session;
        netsnmp_pdu*        pdu;

        snmp_sess_init(&session);
        session.peername = const_cast<char*>(h->ip.c_str());
        session.retries = 2;
        session.timeout = 10000000;
        session.remote_port = 161;
        session.version = SNMP_VERSION_2c;
        session.community = (u_char*)strdup("public");
        session.community_len = strlen("public");

        session.callback = async_response;
        session.callback_magic = h;

        h->pSession = snmp_open(&session);
    }
}

void Manager::asyn_send()
{
    for(auto &h : m_hosts)
    {
        struct snmp_pdu* pdu = snmp_pdu_create(SNMP_MSG_GET);
        for(auto &oid : h->listOid)
        {
            snmp_add_null_var(pdu, oid.o, oid.length);
        }
        if(snmp_send(h->pSession, pdu))
        {
            ++m_sendCount;
        }
        else
        {
            cout<<"snmp_send error!"<<endl;
            snmp_free_pdu(pdu);
        }
    }
}

void Manager::wait_request()
{
    while(m_sendCount>0)
    {
        int fds=0, block=1;
        fd_set fdset;
        struct timeval timeout;
        FD_ZERO(&fdset);
        snmp_select_info(&fds, &fdset, &timeout, &block);
        fds = select(fds, &fdset, nullptr, nullptr, block?NULL:&timeout);
        if(fds<0)
        {
            cout<<"select failed"<<endl;
            exit(1);
        }
        if(fds)
        {
            snmp_read(&fdset);
        }
        else
        {
            snmp_timeout();
        }
    }
}


void Manager::run()
{
    m_running = true;
    if(m_hosts.empty())
    {
        cout<<"no host!"<<endl;
        return;
    }
    init_sessions();
    //std::thread t([&](){
        while(m_running)
        {
            asyn_send();
            wait_request();
            sleep(m_loopInterval);
        }
    //});
    
    //那要在哪里等待线程退出呢？
}

void Manager::stop()
{
    
}

void Manager::set_func(std::function<bool(Host, snmp_pdu*)> f)
{
    m_handleFunc = f;
}

