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

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameters.h"

//==============================================================================

Ckpa_compressorAudioProcessor::Ckpa_compressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this)
    , paramThreshold(parameters, "Threshold", "dB", -60.0f, 0.0f, -24.0f)
    , paramRatio(parameters, "Ratio", ":1", 1.0f, 100.0f, 1.0f)
    , paramAttack(parameters, "Attack", "ms", 0.1f, 100.0f, 2.0f, [](float value) { return value * 0.001f; })
    , paramRelease(parameters, "Release", "ms", 10.0f, 1000.0f, 300.0f, [](float value) { return value * 0.001f; })
    , paramMakeupGain(parameters, "Makeup gain", "dB", -12.0f, 12.0f, 0.0f)
    , paramBypass(parameters, "")
{
    parameters.valueTreeState.state = ValueTree(Identifier(getName().removeCharacters("- ")));
}

Ckpa_compressorAudioProcessor::~Ckpa_compressorAudioProcessor()
{
}

//==============================================================================

void Ckpa_compressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const double smoothTime = 1e-3;
    paramThreshold.reset(sampleRate, smoothTime);
    paramRatio.reset(sampleRate, smoothTime);
    paramAttack.reset(sampleRate, smoothTime);
    paramRelease.reset(sampleRate, smoothTime);
    paramMakeupGain.reset(sampleRate, smoothTime);
    paramBypass.reset(sampleRate, smoothTime);

    //======================================

    mixedDownInput.setSize(1, samplesPerBlock);

    inputLevel = 0.0f;
    ylPrev = 0.0f;

    inverseSampleRate = 1.0f / (float) getSampleRate();
    inverseE = 1.0f / M_E;
    visualiser.clear();
}

void Ckpa_compressorAudioProcessor::releaseResources()
{
}

void Ckpa_compressorAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Create copy of buffer before compression
    AudioBuffer<float> bufferBefore;
    bufferBefore.makeCopyOf(buffer);
    
    AudioBuffer<float> bufferGainReduction;
    if (level1active)
        bufferGainReduction.makeCopyOf(buffer);

    // Don't compress if bypass activated
    if (!(bool) paramBypass.getTargetValue()) {
        mixedDownInput.clear();
        for (int channel = 0; channel < numInputChannels; ++channel)
            mixedDownInput.addFrom(0, 0, buffer, channel, 0, numSamples, 1.0f / numInputChannels);

        for (int sample = 0; sample < numSamples; ++sample) {
            float T = paramThreshold.getNextValue();                                // Threshold
            float R = paramRatio.getNextValue();                                    // Ratio
            float alphaA = calculateAttackOrRelease(paramAttack.getNextValue());    // Attack
            float alphaR = calculateAttackOrRelease(paramRelease.getNextValue());   // Release
            float makeupGain = paramMakeupGain.getNextValue();                      // Makeup Gain

            // Square input
            inputLevel = powf(mixedDownInput.getSample(0, sample), 2.0f);
            // Convert gain to dB
            xg = (inputLevel <= 1e-6f) ? -60.0f : 10.0f * log10f(inputLevel);

            // Compressor
            if (xg < T) {   // Do nothing
                yg = xg;
            } else {        // Compress
                yg = T + (xg - T) / R;
            }

            // Difference of input and output of compression
            xl = xg - yg;

            if (xl > ylPrev) {  // Signal rising -> Attack
                yl = alphaA * ylPrev + (1.0f - alphaA) * xl;
            } else {            // Signal falling -> Release
                yl = alphaR * ylPrev + (1.0f - alphaR) * xl;
            }

            // Calculate control and convert dB to gain
            control = powf(10.0f, (makeupGain - yl) * 0.05f);
            ylPrev = yl;

            for (int channel = 0; channel < numInputChannels; ++channel) {
                float oldValue = buffer.getSample(channel, sample);
                float newValue = oldValue * control;
                buffer.setSample(channel, sample, newValue);
                if (level1active)
                    bufferGainReduction.setSample(channel, sample, oldValue - newValue);
            }
        }
    }
    else {
        if (level1active) {
            for (int sample = 0; sample < numSamples; ++sample) {
                for (int channel = 0; channel < numInputChannels; ++channel) {
                    bufferGainReduction.setSample(channel, sample, 0);
                }
            }
        }
    }

    if (level1active) // Push signal to level metersources
    {
        meterSourceInput.measureBlock(bufferBefore);
        meterSourceOutput.measureBlock(buffer);
        meterSourceGainReduction.measureBlock(bufferGainReduction);
    }

    // Push signal to visualiser buffer
    visualiser.pushBuffer(bufferBefore, buffer);

    //======================================

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);
}

float Ckpa_compressorAudioProcessor::calculateAttackOrRelease(float value)
{
    if (value == 0.0f)
        return 0.0f;
    else
        return pow(inverseE, inverseSampleRate / value);
}


//==============================================================================

void Ckpa_compressorAudioProcessor::getStateInformation(MemoryBlock& destData)
{
    auto state = parameters.valueTreeState.copyState();
    std::unique_ptr<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void Ckpa_compressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.valueTreeState.state.getType()))
            parameters.valueTreeState.replaceState(ValueTree::fromXml(*xmlState));
}

//==============================================================================
bool Ckpa_compressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Ckpa_compressorAudioProcessor::createEditor()
{
    return new Ckpa_compressorAudioProcessorEditor (*this);
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool Ckpa_compressorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

//==============================================================================

const String Ckpa_compressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Ckpa_compressorAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool Ckpa_compressorAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool Ckpa_compressorAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double Ckpa_compressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Ckpa_compressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Ckpa_compressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Ckpa_compressorAudioProcessor::setCurrentProgram(int index)
{
}

const String Ckpa_compressorAudioProcessor::getProgramName(int index)
{
    return {};
}

void Ckpa_compressorAudioProcessor::changeProgramName(int index, const String& newName)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ckpa_compressorAudioProcessor();
}
