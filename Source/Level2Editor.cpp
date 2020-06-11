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
        Slider* cls;
        controlLineSliders.add(cls = new Slider(Slider::LinearVertical, Slider::NoTextBox));

        auto colour = (i == 0) ? findColour(Slider::thumbColourId).darker(0.1) : ((i == 1) ? Colour(0xFFCB8035) : Colour(0xFF2E8B00));
        cls->setColour(Slider::thumbColourId, colour);
        cls->setLookAndFeel(&tos);

        const AudioProcessorParameterWithID* controlLineParamter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i]);
        SliderAttachment* controlLineSliderAttachment;
        sliderAttachments.add(controlLineSliderAttachment = new SliderAttachment(processor.parameters.valueTreeState, controlLineParamter->paramID, *cls));
        cls->addListener(this);
        
        addAndMakeVisible(cls);
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

    for (int i : {0, 1, 2})
    {
        Slider* cls = controlLineSliders.getUnchecked(i);

        float sliderPos = cls->getPositionOfValue(cls->getValue());

        g.setColour(cls->findColour(Slider::thumbColourId));
        if (i == 0 || i == 2) { // threshold or makeup gain line
            int sliderTop = cls->getY();
            g.drawHorizontalLine(sliderTop + sliderPos, r.getX(), r.getRight());
        }
        else { // ratio line
            int sliderCentreX = cls->getX() + cls->getWidth() / 2;
            Line<float> line(Point<float>(r.getX(), r.getY() + r.getHeight() / 2), 
                Point<float>(sliderCentreX, sliderPos).transformedBy(cls->getTransform()));
            g.drawLine(line, 1.0f);
        }
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
    controlLineSliders[0]->setBounds(rVis); // Threshold

    rVis = getLocalBounds().reduced(editorMargin);
    rVis = rVis.removeFromLeft(rVis.getHeight() / 2 + 10)
        .removeFromRight(20)
        .removeFromTop(rVis.getHeight() / 2)
        .expanded(0, 10);
    controlLineSliders[1]->setBounds(rVis); // Ratio
    controlLineSliders[1]->setTransform(AffineTransform::verticalFlip(181));

    rVis = getLocalBounds().reduced(editorMargin);
    rVis = rVis.removeFromRight(60)
        .removeFromLeft(20)
        .removeFromTop(rVis.getHeight() * 0.75)
        .withTrimmedTop(rVis.getHeight() * 0.25)
        .expanded(0, 10);
    controlLineSliders[2]->setBounds(rVis); // Makeup Gain
}

//==============================================================================

void Level2Editor::sliderValueChanged(Slider* slider)
{
    repaint(); // Necessary to prevent the control lines from lagging behind
}
