// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2023 Intel Corporation. All Rights Reserved.

#include "rs-dds-depth-sensor-proxy.h"
#include "rs-dds-option.h"

#include <src/librealsense-exception.h>
#include <rsutils/json.h>


namespace librealsense {


static const std::string metadata_header_key( "header", 6 );
static const std::string depth_units_key( "depth-units", 11 );


float dds_depth_sensor_proxy::get_depth_scale() const
{
    if( auto opt = get_option_handler( RS2_OPTION_DEPTH_UNITS ) )
    {
        // We don't want to do a long control-reply cycle on a value that gets updated automatically
        if( auto dds_option = std::dynamic_pointer_cast< rs_dds_option >( opt ) )
            return dds_option->get_last_known_value();
        else
            return opt->query();
    }
    // Rather than throwing an exception, we try to be a little more helpful: without depth units, the depth image will
    // show black, prompting bug reports. The D400 units min/max are taken from the HW, but the default is set to:
    return 0.001f;
}


void dds_depth_sensor_proxy::add_no_metadata( frame * const f, streaming_impl & streaming )
{
    f->additional_data.depth_units = get_depth_scale();
    super::add_no_metadata( f, streaming );
}


void dds_depth_sensor_proxy::add_frame_metadata( frame * const f, nlohmann::json && dds_md, streaming_impl & streaming )
{
    if( auto du = rsutils::json::nested( dds_md, metadata_header_key, depth_units_key ) )
    {
        try
        {
            f->additional_data.depth_units = rsutils::json::value< float >( du );
        }
        catch( nlohmann::json::exception const & )
        {
            f->additional_data.depth_units = get_depth_scale();
        }
    }
    else
    {
        f->additional_data.depth_units = get_depth_scale();
    }

    super::add_frame_metadata( f, std::move( dds_md ), streaming );
}


}  // namespace librealsense