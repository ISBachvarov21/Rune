//
// Copyright (C) 2013 Vadim Zeitlin
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef SOCI_CONNECTION_PARAMETERS_H_INCLUDED
#define SOCI_CONNECTION_PARAMETERS_H_INCLUDED

#include "soci-platform.h"

#include <map>
#include <string>

namespace soci
{

// Names of some predefined options and their values.
extern SOCI_DECL char const * option_reconnect;
extern SOCI_DECL char const * option_true;

namespace details
{

class dynamic_backend_ref;

} // namespace details

class backend_factory;

// Simple container for the information used when opening a session.
class SOCI_DECL connection_parameters
{
public:
    connection_parameters();
    connection_parameters(backend_factory const & factory, std::string const & connectString);
    connection_parameters(std::string const & backendName, std::string const & connectString);
    explicit connection_parameters(std::string const & fullConnectString);

    connection_parameters(connection_parameters const& other);
    connection_parameters& operator=(connection_parameters const& other);

    ~connection_parameters();


    // Retrieve the backend and the connection strings specified in the ctor.
    backend_factory const * get_factory() const { return factory_; }
    void set_connect_string(const std::string & connectString) { connectString_ = connectString; }
    std::string const & get_connect_string() const { return connectString_; }

    // Set the value of the given option, overwriting any previous value.
    void set_option(const char * name, std::string const & value)
    {
        options_[name] = value;
    }

    // Return true if the option with the given name was found and fill the
    // provided parameter with its value.
    bool get_option(const char * name, std::string & value) const
    {
        Options::const_iterator const it = options_.find(name);
        if (it == options_.end())
            return false;

        value = it->second;

        return true;
    }

    // Return true if the option with the given name was found with option_true
    // value.
    bool is_option_on(const char * name) const
    {
      std::string value;
      return get_option(name, value) && value == option_true;
    }

private:
    // The backend and connection string specified in our ctor.
    backend_factory const * factory_;
    std::string connectString_;

    // References the backend name used for obtaining the factor from
    // dynamic_backends.
    details::dynamic_backend_ref * backendRef_;

    // We store all the values as strings for simplicity.
    typedef std::map<std::string, std::string> Options;
    Options options_;
};

} // namespace soci

#endif // SOCI_CONNECTION_PARAMETERS_H_INCLUDED
