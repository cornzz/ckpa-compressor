/*
  ==============================================================================

    Code by cornzz and Philip Arms.
    Uses code by Juan Gil <https://juangil.com/>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================
*/

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <JuceHeader.h>
#include "PluginParameters.h"

//==============================================================================

class Ckpa_compressorAudioProcessor  : public AudioProcessor,
                                       public ChangeBroadcaster
{
public:
    //==============================================================================

    Ckpa_compressorAudioProcessor();
    ~Ckpa_compressorAudioProcessor();

    //==============================================================================

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    AudioBuffer<float> mixedDownInput;
    AudioBuffer<float> bufferBefore;
    AudioBuffer<float> bufferAfter;
    AudioBuffer<float> bufferGainReduction;

    float xl;
    float yl;
    float xg;
    float yg;
    float control;

    float inputLevel;
    float ylPrev;

    float inverseSampleRate;
    float inverseE;
    float calculateAttackOrRelease(float value);

    //======================================

    PluginParametersManager parameters;

    PluginParameterLinSlider paramThreshold;
    PluginParameterLinSlider paramRatio;
    PluginParameterLinSlider paramAttack;
    PluginParameterLinSlider paramRelease;
    PluginParameterLinSlider paramMakeupGain;
    PluginParameterToggle paramBypass;
    PluginParameterLinSlider paramCompression;

    foleys::LevelMeterSource meterSourceInput;
    foleys::LevelMeterSource meterSourceOutput;
    foleys::LevelMeterSource meterSourceGainReduction;

    bool level1active = false;
    bool level2active = false;

private:

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ckpa_compressorAudioProcessor)
};
