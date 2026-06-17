#include <map>
#include <courierr/courierr.h>
#include "rs_instance_factory.h"
#include "rs_instance_base.h"

namespace
{
    using RS_factory_map = std::map<std::string, std::shared_ptr<tk205::RSInstanceFactory> >;

    RS_factory_map& get_RS_factory_map()
    {
    static RS_factory_map factory_map;
    return factory_map;
    }
}

namespace tk205 {
    //static
    bool RSInstanceFactory::register_factory(std::string const& RS_ID,
                                               std::shared_ptr<RSInstanceFactory> factory)
    {
    get_RS_factory_map()[RS_ID] = factory;
    return true;
    }

    //static
    std::shared_ptr<RSInstanceBase> RSInstanceFactory::create(std::string const& RS_ID, 
                                                              const char* RS_instance_file,
                                                              std::shared_ptr<Courierr::Courierr> logger)
    {
    const auto factory = get_RS_factory_map()[RS_ID];
    
    return (factory == nullptr) ? nullptr : factory->create_instance(RS_instance_file, logger);
    }
}