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

#include "Level2Editor.h"

//==============================================================================

Level2Editor::Level2Editor(Ckpa_compressorAudioProcessor& p) : processor(p)
{
    processor.addChangeListener(this);
    visualiser.clear();
    addAndMakeVisible(visualiser);

    //======================================

    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int ind[] = { 0, 4, 1 };
    for (int i : ind)
    {
        Slider* controlLine;
        controlLines.add(controlLine = new Slider(Slider::LinearVertical, Slider::NoTextBox));
        controlLine->setLookAndFeel(&dhl);
        if (i != 0)
            controlLine->setColour(Slider::thumbColourId, (i == 4) ? Colour(0xFF2E8B00) : Colour(0xFFCB8035));
        else
            controlLine->setColour(Slider::thumbColourId, findColour(Slider::thumbColourId).darker(0.1));
        const AudioProcessorParameterWithID* controlLineParamter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i]);
        SliderAttachment* controlLineSliderAttachment;
        sliderAttachments.add(controlLineSliderAttachment = new SliderAttachment(processor.parameters.valueTreeState, controlLineParamter->paramID, *controlLine));
        addAndMakeVisible(controlLine);

        controlLine->addListener(&visualiser);
        visualiser.addControlLine(controlLine);
    }
}

Level2Editor::~Level2Editor()
{
    processor.removeChangeListener(this);
}

void Level2Editor::changeListenerCallback(ChangeBroadcaster* source)
{
    Ckpa_compressorAudioProcessor* p = dynamic_cast<Ckpa_compressorAudioProcessor*> (source);
    visualiser.pushBuffer(p->bufferBefore, p->bufferAfter);
}

void Level2Editor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void Level2Editor::resized()
{
    Rectangle<int> rVis = getLocalBounds().reduced(editorMargin);
    visualiser.setBounds(rVis);

    rVis = getLocalBounds().reduced(editorMargin);
    controlLines[0]->setBounds(rVis.removeFromLeft(20).removeFromTop(rVis.getHeight() / 2).expanded(0, 10)); // Threshold

    rVis = getLocalBounds().reduced(editorMargin);
    controlLines[1]->setBounds(rVis.removeFromRight(60).removeFromLeft(20).removeFromTop(rVis.getHeight() * 0.75).withTrimmedTop(rVis.getHeight() * 0.25).expanded(0, 10)); // Makeup Gain

    rVis = getLocalBounds().reduced(editorMargin);
    controlLines[2]->setBounds(rVis.removeFromLeft(rVis.getHeight() / 2 + 10).removeFromRight(20).removeFromTop(rVis.getHeight() / 2).expanded(0, 10)); // Ratio
    controlLines[2]->setTransform(AffineTransform::verticalFlip(181));
}