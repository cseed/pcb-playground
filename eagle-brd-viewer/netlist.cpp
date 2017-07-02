
#include "netlist.hpp"

Node::Node() {}

Node::Node(const std::vector<std::string> &port_names)
{
    for (auto port_name : port_names)
	append_port(port_name);
}

void
Node::append_port(const std::string &port_name)
{
    // FIXME no instances
    ports.push_back(std::make_unique<Port>(port_name, this));
    port_index.insert(make_pair(port_name, ports.back().get()));
}

Port *
Node::find_port(const std::string &port_name)
{
    auto i = port_index.find(port_name);
    assert (i != port_index.end());
    return i->second;
}


Module::Module() {}
Module::Module(std::string name1)
    : pname(name1)
{}
Module::Module(std::string name1,
	       const std::vector<std::string> &port_names)
    : Node(port_names),
      pname(name1)
{}

Net *
Module::make_net(const std::string &net_name)
{
    nets.push_back(std::make_unique<Net>(this, net_name));
    net_index.insert(make_pair(net_name, nets.back().get()));
    return nets.back().get();
}

Net *
Module::find_net(const std::string &net_name)
{
    auto i = net_index.find(net_name);
    assert (i != net_index.end());
    return i->second;
}

Instance *
Module::make_instance(std::shared_ptr<Module> module, const std::string &instance_name)
{
    instances.push_back(std::make_unique<Instance>(this, module, instance_name));
    instance_index.insert(make_pair(instance_name, instances.back().get()));
    return instances.back().get();
}

Instance *
Module::find_instance(const std::string &instance_name)
{
    auto i = instance_index.find(instance_name);
    assert (i != instance_index.end());
    return i->second;
}

void
Module::as_verilog(std::ostream &s)
{
    s << "\nmodule " << pname << "(\n";
    for (const std::unique_ptr<Port> &p : ports)
	{
	    s << "  inout " << p->name() << ";\n";
	}
    s << ");\n\n";
    
    for (const std::unique_ptr<Net> &n : nets)
	s << "  wire " << n->name() << ";\n";
    
    for (const std::unique_ptr<Instance> &i : instances)
	i->as_verilog(s);
    
    s << "endmodule;\n";
}

Net::Net(Module *scope1, const std::string &name1)
    : pscope(scope1),
      pname(name1)
{}

void
Net::connect(Port *port)
{
    port->disconnect();
    
    assert (port->pconnection == nullptr);
    port->pconnection = this;
    pconnections.insert(port);
}

void
Net::disconnect(Port *port)
{
    assert (port->pconnection == this);
    pconnections.erase(port);
    port->pconnection = nullptr;
}

Instance::Instance(Module *scope1, std::shared_ptr<Module> module1, const std::string &name1)
    : pscope(scope1),
      pmodule(module1),
      pname(name1)
{
    for (size_t i = 0; i < pmodule->num_ports(); i ++)
	append_port(pmodule->port_name(i));
}

void
Instance::as_verilog(std::ostream &s)
{
    s << "\n  " << pmodule->name() << " " << pname << "(\n";
    for (const std::unique_ptr<Port> &p : ports)
	{
	    s << "    ." << p->name() << "(";
	    if (p->connection())
		s << p->connection()->name();
	    s << "),\n";
	}
    s << "  );\n";
}

Port::Port(const std::string &name1, Node *node1)
    : pname(name1),
      pnode(node1),
      pconnection(0)
{}

void
Port::connect(Net *net)
{
    if (pconnection)
	disconnect();
    
    assert (pconnection == nullptr);
    net->pconnections.insert(this);
    pconnection = net;
}

void
Port::disconnect()
{
    if (pconnection)
	{
	    pconnection->pconnections.erase(this);
	    pconnection = nullptr;
	}
}
