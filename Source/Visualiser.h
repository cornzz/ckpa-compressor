/*
  ==============================================================================

    Code by cornzz.

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

#include <JuceHeader.h>

//==============================================================================
/**
    An extension of the AudioVisualiserComponent class to support two waveforms
    drawn on top of each other.
    All methods necessary for drawing the waveform were overridden and extended
    to replicate the work done by the base class for a second AudioBuffer.
    The base methods are called by the overriding methods to draw the bottom 
    waveform, while the overriding methods draw the top waveform.
*/
class Visualiser : public AudioVisualiserComponent
{
public:
    Visualiser();

    void setNumChannels(int numChannels);
    void setBufferSize(int newNumSamples);
    void clear();
    void pushBufferTop(const float** d, int numChannels, int num);
    void pushBuffer(const AudioBuffer<float>& buffer, const AudioBuffer<float>& buffer2);
    void setSamplesPerBlock(int newSamplesPerPixel) noexcept;

    void setColours(Colour bk, Colour fg) noexcept;
    void paint(Graphics& g) override;

private:
    struct ChannelInfo2
    {
        ChannelInfo2(AudioVisualiserComponent& o, int bufferSize) : owner(o)
        {
            setBufferSize(bufferSize);
            clear();
        }

        void clear() noexcept
        {
            levels.fill({});
            value = {};
            subSample = 0;
        }

        void pushSamples(const float* inputSamples, int num) noexcept
        {
            for (int i = 0; i < num; ++i)
                pushSample(inputSamples[i]);
        }

        void pushSample(float newSample) noexcept
        {
            if (--subSample <= 0)
            {
                if (++nextSample == levels.size())
                    nextSample = 0;

                levels.getReference(nextSample) = value;
                subSample = owner.getSamplesPerBlock();
                value = Range<float>(newSample, newSample);
            }
            else
            {
                value = value.getUnionWith(newSample);
            }
        }

        void setBufferSize(int newSize)
        {
            levels.removeRange(newSize, levels.size());
            levels.insertMultiple(-1, {}, newSize - levels.size());

            if (nextSample >= newSize)
                nextSample = 0;
        }

        AudioVisualiserComponent& owner;
        Array<Range<float>> levels;
        Range<float> value;
        std::atomic<int> nextSample{ 0 }, subSample{ 0 };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelInfo2)
    };

    OwnedArray<ChannelInfo2> channelsTop;
    int numSamplesTop, inputSamplesPerBlockTop;
    Colour waveformColour2;
};