#pragma once

#include <cassert>

#include <vector>
#include <set>
#include <string>
#include <memory>
#include <iostream>
#include <map>

namespace std
{

// FIXME move
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}

class Node;
class Module;
class Net;
class Instance;
class Port;

class Port
{
private:
    friend class Net;
    
    std::string pname;
    Node *pnode;
    Net *pconnection;
    
public:
    Port(const std::string &name1, Node *node1);
    
    const std::string &name() const { return pname; }
    
    Net *connection() const { return pconnection; }
    void connect(Net *net);
    void disconnect();
};

class Node
{
protected:
    friend class Port;
    
    std::vector<std::unique_ptr<Port>> ports;
    std::map<std::string, Port *> port_index;
    
public:
    Node();
    Node(const std::vector<std::string> &port_names);
    
    size_t num_ports() const { return ports.size(); }
    Port *port (size_t i) const { return ports[i].get(); }
    
    const std::string &port_name(size_t i) const { return ports[i]->name(); }
    
    // FIXME iterate over ports?
    
    // FIXME check instances
    void append_port(const std::string &port_name);
    Port *find_port(const std::string &port_name);
};

// FIXME module?  component?  part?  device?
class Module : public Node
{
private:
    // FIXME add port direction (more generally, type), vector ports
    std::string pname;
    
    std::vector<std::unique_ptr<Net>> nets;
    std::map<std::string, Net*> net_index;
    
    std::vector<std::unique_ptr<Instance>> instances;
    std::map<std::string, Instance*> instance_index;
    
    // FIXME sub-instances, net lists, etc.
    
public:
    Module();
    Module(std::string name1);
    Module(std::string name1,
	   const std::vector<std::string> &port_names1);
    
    const std::string &name() const { return pname; }
    
    Net *make_net(const std::string &net_name);
    Net *find_net(const std::string &net_nanme);
    
    Instance *make_instance(std::shared_ptr<Module> module, const std::string &instance_name);
    Instance *find_instance(const std::string &instance_name);
    
    void as_verilog(std::ostream &s);
};

class Net
{
private:
    friend class Port;
    
    Module *pscope;
    
    std::string pname;
    std::set<Port *> pconnections;
    
public:
    Net(Module *scope1, const std::string &name1);
    
    const std::string &name() const { return pname; }
    const std::set<Port *> &connections() const { return pconnections; }
    void connect(Port *port);
    void disconnect(Port *port);
};

class Instance : public Node
{
private:
    friend class Port;
    
    Module *pscope;
    std::shared_ptr<Module> pmodule;
    std::string pname;
    
public:
    Instance(Module *scope1, std::shared_ptr<Module> module1, const std::string &name1);
    
    const std::string &name() const { return pname; }
    void as_verilog(std::ostream &s);
};
