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

#include "Visualiser.h"

//==============================================================================

Visualiser::Visualiser() : AudioVisualiserComponent(1)
{
    setNumChannels(1);
    setBufferSize(512);
    setSamplesPerBlock(1024);
    setColours(getLookAndFeel().findColour(ResizableWindow::backgroundColourId).darker(0.15), getLookAndFeel().findColour(Slider::thumbColourId));
}

void Visualiser::setNumChannels(int numChannels)
{
    channelsTop.clear();

    for (int i = 0; i < numChannels; ++i)
        channelsTop.add(new ChannelInfo2(*this, numSamplesTop));
}

void Visualiser::setBufferSize(int newNumSamples)
{
    numSamplesTop = newNumSamples;

    for (auto* c : channelsTop)
        c->setBufferSize(newNumSamples);

    AudioVisualiserComponent::setBufferSize(newNumSamples);
}

void Visualiser::clear()
{
    for (auto* c : channelsTop)
        c->clear();

    AudioVisualiserComponent::clear();
}

void Visualiser::pushBufferTop(const float** d, int numChannels, int num)
{
    numChannels = jmin(numChannels, channelsTop.size());

    for (int i = 0; i < numChannels; ++i)
        channelsTop.getUnchecked(i)->pushSamples(d[i], num);
}

void Visualiser::pushBuffer(const AudioBuffer<float>& bufferBottom, const AudioBuffer<float>& bufferTop)
{
    pushBufferTop(bufferTop.getArrayOfReadPointers(),
        bufferTop.getNumChannels(),
        bufferTop.getNumSamples());

    AudioVisualiserComponent::pushBuffer(bufferBottom);
}

void Visualiser::setSamplesPerBlock(int newSamplesPerPixel) noexcept
{
    inputSamplesPerBlockTop = newSamplesPerPixel;

    AudioVisualiserComponent::setSamplesPerBlock(newSamplesPerPixel);
}

//==============================================================================

void Visualiser::setColours(Colour bk, Colour fg) noexcept
{
    waveformColour2 = fg.darker(0.4);

    AudioVisualiserComponent::setColours(bk, fg);
}

void Visualiser::paint(Graphics& g) 
{
    // Call base class paint method first, since it fills the component with a background colour.
    AudioVisualiserComponent::paint(g);

    auto r = getLocalBounds().toFloat();
    auto channelHeight = r.getHeight();

    g.setColour(waveformColour2);

    for (auto* c : channelsTop)
        paintChannel(g, r.removeFromTop(channelHeight), c->levels.begin(), c->levels.size(), c->nextSample);
}