#pragma once

#include <memory>
#include <sigc++/sigc++.h>

#include "VideoSignals.hpp"
#include "AudioSignals.hpp"
#include "GpsSignals.hpp"

class Signals{
public:
    typedef std::shared_ptr<Signals> Pointer;


    VideoSignals::Pointer videoSignals;
    AudioSignals::Pointer audioSignals;
    GpsSignals::Pointer gpsSignals;


    Signals(): videoSignals(std::make_shared<VideoSignals>()), audioSignals(std::make_shared<AudioSignals>()), gpsSignals(std::make_shared<GpsSignals>()){};
};