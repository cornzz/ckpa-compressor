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

    //============ Control Lines ===========

    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int ind[] = { 0, 1, 4 }; // Indices in processor parameter array (0 = thresh, 1 = ratio, 4 = makeupg)
    for (int i : ind)
    {
        Slider* controlLine;
        controlLines.add(controlLine = new Slider(Slider::LinearVertical, Slider::NoTextBox));

        auto colour = (i == 0) ? findColour(Slider::thumbColourId).darker(0.1) : ((i == 1) ? Colour(0xFFCB8035) : Colour(0xFF2E8B00));
        controlLine->setColour(Slider::thumbColourId, colour);
        controlLine->setLookAndFeel(&tos);

        const AudioProcessorParameterWithID* controlLineParamter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i]);
        SliderAttachment* controlLineSliderAttachment;
        sliderAttachments.add(controlLineSliderAttachment = new SliderAttachment(processor.parameters.valueTreeState, controlLineParamter->paramID, *controlLine));
        controlLine->addListener(this);
        
        addAndMakeVisible(controlLine);
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

void Level2Editor::paintOverChildren(Graphics& g)
{
    auto r = getLocalBounds().reduced(editorMargin).toFloat();

    for (int i : {0, 2})
    {
        Slider* controlLine = controlLines.getUnchecked(i);

        float sliderPos = controlLine->getPositionOfValue(controlLine->getValue());
        int y = controlLine->getY();

        g.setColour(controlLine->findColour(Slider::thumbColourId));
        g.drawHorizontalLine(y + sliderPos, r.getX(), r.getRight());
    }
}

void Level2Editor::resized()
{
    Rectangle<int> rVis = getLocalBounds().reduced(editorMargin);
    visualiser.setBounds(rVis);

    //======================================

    rVis = getLocalBounds().reduced(editorMargin);
    rVis = rVis.removeFromLeft(20)
        .removeFromTop(rVis.getHeight() / 2)
        .expanded(0, 10);
    controlLines[0]->setBounds(rVis); // Threshold

    rVis = getLocalBounds().reduced(editorMargin);
    rVis = rVis.removeFromLeft(rVis.getHeight() / 2 + 10)
        .removeFromRight(20)
        .removeFromTop(rVis.getHeight() / 2)
        .expanded(0, 10);
    controlLines[1]->setBounds(rVis); // Ratio
    controlLines[1]->setTransform(AffineTransform::verticalFlip(181));

    rVis = getLocalBounds().reduced(editorMargin);
    rVis = rVis.removeFromRight(60)
        .removeFromLeft(20)
        .removeFromTop(rVis.getHeight() * 0.75)
        .withTrimmedTop(rVis.getHeight() * 0.25)
        .expanded(0, 10);
    controlLines[2]->setBounds(rVis); // Makeup Gain
}

void Level2Editor::sliderValueChanged(Slider* slider)
{
    repaint(); // Necessary to prevent the control lines from lagging behind
}
