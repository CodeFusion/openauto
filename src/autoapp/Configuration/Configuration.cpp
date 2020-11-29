/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

#include <f1x/openauto/autoapp/Configuration/Configuration.hpp>
#include <easylogging++.h>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace configuration
{

const std::string Configuration::cConfigFileName = "openauto.ini";

const std::string Configuration::cGeneralShowClockKey = "General.ShowClock";

const std::string Configuration::cGeneralShowBigClockKey = "General.ShowBigClock";
const std::string Configuration::cGeneralOldGUIKey = "General.OldGUI";
const std::string Configuration::cGeneralAlphaTransKey = "General.AlphaTrans";
const std::string Configuration::cGeneralHideMenuToggleKey = "General.HideMenuToggle";
const std::string Configuration::cGeneralHideAlphaKey = "General.HideAlpha";
const std::string Configuration::cGeneralShowLuxKey = "General.ShowLux";
const std::string Configuration::cGeneralShowCursorKey = "General.ShowCursor";
const std::string Configuration::cGeneralHideBrightnessControlKey = "General.HideBrightnessControl";
const std::string Configuration::cGeneralShowNetworkinfoKey = "General.ShowNetworkinfo";
const std::string Configuration::cGeneralHideWarningKey = "General.HideWarning";

const std::string Configuration::cGeneralHandednessOfTrafficTypeKey = "General.HandednessOfTrafficType";

const std::string Configuration::cGeneralMp3MasterPathKey = "General.Mp3MasterPath";
const std::string Configuration::cGeneralMp3SubFolderKey = "General.Mp3SubFolder";
const std::string Configuration::cGeneralMp3TrackKey = "General.Mp3Track";
const std::string Configuration::cGeneralMp3AutoPlayKey = "General.Mp3AutoPlay";
const std::string Configuration::cGeneralShowAutoPlayKey = "General.ShowAutoPlay";
const std::string Configuration::cGeneralInstantPlayKey = "General.InstantPlay";

const std::string Configuration::cVideoFPSKey = "Video.FPS";
const std::string Configuration::cVideoResolutionKey = "Video.Resolution";
const std::string Configuration::cVideoScreenDPIKey = "Video.ScreenDPI";
const std::string Configuration::cVideoOMXLayerIndexKey = "Video.OMXLayerIndex";
const std::string Configuration::cVideoMarginWidth = "Video.MarginWidth";
const std::string Configuration::cVideoMarginHeight = "Video.MarginHeight";

const std::string Configuration::cAudioMusicAudioChannelEnabled = "Audio.MusicAudioChannelEnabled";
const std::string Configuration::cAudioSpeechAudioChannelEnabled = "Audio.SpeechAudioChannelEnabled";
const std::string Configuration::cAudioOutputBackendType = "Audio.OutputBackendType";

const std::string Configuration::cBluetoothAdapterTypeKey = "Bluetooth.AdapterType";
const std::string Configuration::cBluetoothRemoteAdapterAddressKey = "Bluetooth.RemoteAdapterAddress";

const std::string Configuration::cInputEnableTouchscreenKey = "Input.EnableTouchscreen";
const std::string Configuration::cInputEnablePlayerControlKey = "Input.EnablePlayerControl";
const std::string Configuration::cInputPlayButtonKey = "Input.PlayButton";
const std::string Configuration::cInputPauseButtonKey = "Input.PauseButton";
const std::string Configuration::cInputTogglePlayButtonKey = "Input.TogglePlayButton";
const std::string Configuration::cInputNextTrackButtonKey = "Input.NextTrackButton";
const std::string Configuration::cInputPreviousTrackButtonKey = "Input.PreviousTrackButton";
const std::string Configuration::cInputHomeButtonKey = "Input.HomeButton";
const std::string Configuration::cInputPhoneButtonKey = "Input.PhoneButton";
const std::string Configuration::cInputCallEndButtonKey = "Input.CallEndButton";
const std::string Configuration::cInputVoiceCommandButtonKey = "Input.VoiceCommandButton";
const std::string Configuration::cInputLeftButtonKey = "Input.LeftButton";
const std::string Configuration::cInputRightButtonKey = "Input.RightButton";
const std::string Configuration::cInputUpButtonKey = "Input.UpButton";
const std::string Configuration::cInputDownButtonKey = "Input.DownButton";
const std::string Configuration::cInputScrollWheelButtonKey = "Input.ScrollWheelButton";
const std::string Configuration::cInputBackButtonKey = "Input.BackButton";
const std::string Configuration::cInputEnterButtonKey = "Input.EnterButton";
const std::string Configuration::cInputNavButtonKey = "Input.NavButton";

Configuration::Configuration()
{
    this->load();
}

void Configuration::load()
{
    handednessOfTrafficType_ = HandednessOfTrafficType::LEFT_HAND_DRIVE;
    showClock_ = true;
    showBigClock_ = false;
    oldGUI_ = false;
    alphaTrans_ = 50;
    hideMenuToggle_ = false;
    hideAlpha_ = false;
    showLux_ = false;
    showCursor_ = false;
    hideBrightnessControl_ = false;
    hideWarning_ = false;
    showNetworkinfo_ = false;
    mp3MasterPath_ = "/media/MYMEDIA";
    mp3SubFolder_ = "/";
    mp3Track_ = 0;
    mp3AutoPlay_ = false;
    showAutoPlay_ = false;
    instantPlay_ = false;
    videoFPS_ = aasdk::proto::enums::VideoFPS::_30;
    videoResolution_ = aasdk::proto::enums::VideoResolution::_480p;
    screenDPI_ = 140;
    omxLayerIndex_ = 1;
    enableTouchscreen_ = true;
    enablePlayerControl_ = false;
    buttonCodes_.clear();
    bluetoothAdapterType_ = BluetoothAdapterType::NONE;
    bluetoothRemoteAdapterAddress_ = "";
    musicAudioChannelEnabled_ = true;
    speechAudiochannelEnabled_ = true;
    audioOutputBackendType_ = AudioOutputBackendType::QT;
}

void Configuration::reset()
{
    handednessOfTrafficType_ = HandednessOfTrafficType::LEFT_HAND_DRIVE;
    showClock_ = true;
    showBigClock_ = false;
    oldGUI_ = false;
    alphaTrans_ = 50;
    hideMenuToggle_ = false;
    hideAlpha_ = false;
    showLux_ = false;
    showCursor_ = false;
    hideBrightnessControl_ = false;
    hideWarning_ = false;
    showNetworkinfo_ = false;
    mp3MasterPath_ = "/media/MYMEDIA";
    mp3SubFolder_ = "/";
    mp3Track_ = 0;
    mp3AutoPlay_ = false;
    showAutoPlay_ = false;
    instantPlay_ = false;
    videoFPS_ = aasdk::proto::enums::VideoFPS::_30;
    videoResolution_ = aasdk::proto::enums::VideoResolution::_480p;
    screenDPI_ = 140;
    omxLayerIndex_ = 1;
    enableTouchscreen_ = true;
    enablePlayerControl_ = false;
    buttonCodes_.clear();
    bluetoothAdapterType_ = BluetoothAdapterType::NONE;
    bluetoothRemoteAdapterAddress_ = "";
    musicAudioChannelEnabled_ = true;
    speechAudiochannelEnabled_ = true;
    audioOutputBackendType_ = AudioOutputBackendType::QT;
}

void Configuration::save()
{
}

bool Configuration::hasTouchScreen() const
{
    return true;
}

void Configuration::setHandednessOfTrafficType(HandednessOfTrafficType value)
{
    handednessOfTrafficType_ = value;
}

HandednessOfTrafficType Configuration::getHandednessOfTrafficType() const
{
    return handednessOfTrafficType_;
}

void Configuration::showClock(bool value)
{
    showClock_ = value;
}

bool Configuration::showClock() const
{
    return showClock_;
}

void Configuration::showBigClock(bool value)
{
    showBigClock_ = value;
}

bool Configuration::showBigClock() const
{
    return showBigClock_;
}

void Configuration::oldGUI(bool value)
{
    oldGUI_ = value;
}

bool Configuration::oldGUI() const
{
    return oldGUI_;
}

size_t Configuration::getAlphaTrans() const
{
    return alphaTrans_;
}

void Configuration::setAlphaTrans(size_t value)
{
    alphaTrans_ = value;
}

void Configuration::hideMenuToggle(bool value)
{
    hideMenuToggle_ = value;
}

bool Configuration::hideMenuToggle() const
{
    return hideMenuToggle_;
}

void Configuration::hideAlpha(bool value)
{
    hideAlpha_ = value;
}

bool Configuration::hideAlpha() const
{
    return hideAlpha_;
}

void Configuration::showLux(bool value)
{
    showLux_ = value;
}

bool Configuration::showLux() const
{
    return showLux_;
}

void Configuration::showCursor(bool value)
{
    showCursor_ = value;
}

bool Configuration::showCursor() const
{
    return showCursor_;
}

void Configuration::hideBrightnessControl(bool value)
{
    hideBrightnessControl_ = value;
}

bool Configuration::hideBrightnessControl() const
{
    return hideBrightnessControl_;
}

void Configuration::hideWarning(bool value)
{
    hideWarning_ = value;
}

bool Configuration::hideWarning() const
{
    return hideWarning_;
}

void Configuration::showNetworkinfo(bool value)
{
    showNetworkinfo_ = value;
}

bool Configuration::showNetworkinfo() const
{
    return showNetworkinfo_;
}

std::string Configuration::getMp3MasterPath() const
{
    return mp3MasterPath_;
}

void Configuration::setMp3MasterPath(const std::string& value)
{
    mp3MasterPath_ = value;
}

std::string Configuration::getMp3SubFolder() const
{
    return mp3SubFolder_;
}

void Configuration::setMp3Track(int32_t value)
{
    mp3Track_ = value;
}

void Configuration::setMp3SubFolder(const std::string& value)
{
    mp3SubFolder_ = value;
}

int32_t Configuration::getMp3Track() const
{
    return mp3Track_;
}

void Configuration::mp3AutoPlay(bool value)
{
    mp3AutoPlay_ = value;
}

bool Configuration::mp3AutoPlay() const
{
    return mp3AutoPlay_;
}

void Configuration::showAutoPlay(bool value)
{
    showAutoPlay_ = value;
}

bool Configuration::showAutoPlay() const
{
    return showAutoPlay_;
}

void Configuration::instantPlay(bool value)
{
    instantPlay_ = value;
}

bool Configuration::instantPlay() const
{
    return instantPlay_;
}

aasdk::proto::enums::VideoFPS::Enum Configuration::getVideoFPS() const
{
    return videoFPS_;
}

void Configuration::setVideoFPS(aasdk::proto::enums::VideoFPS::Enum value)
{
    videoFPS_ = value;
}

aasdk::proto::enums::VideoResolution::Enum Configuration::getVideoResolution() const
{
    return videoResolution_;
}

void Configuration::setVideoResolution(aasdk::proto::enums::VideoResolution::Enum value)
{
    videoResolution_ = value;
}

size_t Configuration::getScreenDPI() const
{
    return screenDPI_;
}

void Configuration::setScreenDPI(size_t value)
{
    screenDPI_ = value;
}

void Configuration::setOMXLayerIndex(int32_t value)
{
    omxLayerIndex_ = value;
}

int32_t Configuration::getOMXLayerIndex() const
{
    return omxLayerIndex_;
}


bool Configuration::getTouchscreenEnabled() const
{
    return enableTouchscreen_;
}

void Configuration::setTouchscreenEnabled(bool value)
{
    enableTouchscreen_ = value;
}

bool Configuration::playerButtonControl() const
{
    return enablePlayerControl_;
}

void Configuration::playerButtonControl(bool value)
{
    enablePlayerControl_ = value;
}

Configuration::ButtonCodes Configuration::getButtonCodes() const
{
    return buttonCodes_;
}

void Configuration::setButtonCodes(const ButtonCodes& value)
{
    buttonCodes_ = value;
}

BluetoothAdapterType Configuration::getBluetoothAdapterType() const
{
    return bluetoothAdapterType_;
}

void Configuration::setBluetoothAdapterType(BluetoothAdapterType value)
{
    bluetoothAdapterType_ = value;
}

std::string Configuration::getBluetoothRemoteAdapterAddress() const
{
    return bluetoothRemoteAdapterAddress_;
}

void Configuration::setBluetoothRemoteAdapterAddress(const std::string& value)
{
    bluetoothRemoteAdapterAddress_ = value;
}

bool Configuration::musicAudioChannelEnabled() const
{
    return musicAudioChannelEnabled_;
}

void Configuration::setMusicAudioChannelEnabled(bool value)
{
    musicAudioChannelEnabled_ = value;
}

bool Configuration::speechAudioChannelEnabled() const
{
    return speechAudiochannelEnabled_;
}

void Configuration::setSpeechAudioChannelEnabled(bool value)
{
    speechAudiochannelEnabled_ = value;
}

AudioOutputBackendType Configuration::getAudioOutputBackendType() const
{
    return audioOutputBackendType_;
}

void Configuration::setAudioOutputBackendType(AudioOutputBackendType value)
{
    audioOutputBackendType_ = value;
}

}
}
}
