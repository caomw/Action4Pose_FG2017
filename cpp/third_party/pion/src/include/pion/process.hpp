// ---------------------------------------------------------------------
// pion:  a Boost C++ framework for building lightweight HTTP interfaces
// ---------------------------------------------------------------------
// Copyright (C) 2007-2012 Cloudmeter, Inc.  (http://www.cloudmeter.com)
//
// Distributed under the Boost Software License, Version 1.0.
// See http://www.boost.org/LICENSE_1_0.txt
//

#ifndef __PION_PROCESS_HEADER__
#define __PION_PROCESS_HEADER__

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <pion/config.hpp>


namespace pion {    // begin namespace pion

///
/// process: class for managing process/service related functions
///
class PION_API process :
    private boost::noncopyable
{
public:

    // default destructor
    ~process() {}
    
    /// default constructor
    process(void) {}
    
    /// signals the shutdown condition
    static void shutdown(void);
    
    /// blocks until the shutdown condition has been signaled
    static void wait_for_shutdown(void);

    /// sets up basic signal handling for the process
    static void initialize(void);
    
    /// fork process and run as a background daemon
    static void daemonize(void);


protected:

    /// data type for static/global process configuration information
    struct config_type {
        /// constructor just initializes native types
        config_type() : shutdown_now(false) {}
    
        /// true if we should shutdown now
        bool                    shutdown_now;
        
        /// triggered when it is time to shutdown
        boost::condition        shutdown_cond;

        /// used to protect the shutdown condition
        boost::mutex            shutdown_mutex;
    };

    
    /// returns a singleton instance of config_type
    static inline config_type& get_config(void) {
        boost::call_once(process::create_config, m_instance_flag);
        return *m_config_ptr;
    }


private:

    /// creates the config_type singleton
    static void create_config(void);

    
    /// used to ensure thread safety of the config_type singleton
    static boost::once_flag             m_instance_flag;

    /// pointer to the config_type singleton
    static config_type *          m_config_ptr;
};


}   // end namespace pion

#endif
