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

Level2Editor::Level2Editor(Ckpa_compressorAudioProcessor& p, Component* parentForPopup) : processor(p),
    popupParent(parentForPopup)
{
    processor.addChangeListener(this);
    visualiser.clear();
    addAndMakeVisible(visualiser);

    showDragMe = true;

    //============ Control Lines ===========

    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int ind[] = { 0, 1, 4 }; // Indices in processor parameter array (0 = thresh, 1 = ratio, 4 = makeupg)
    for (int i : ind)
    {
        if (const AudioProcessorParameterWithID* controlLineParamter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {
            Slider* cls;
            controlLineSliders.add(cls = new Slider(Slider::LinearVertical, Slider::NoTextBox));

            auto colour = (i == 0) ? Colour(0xFFb52f2f) : ((i == 1) ? Colour(0xFFCB8035) : Colour(0xFF2E8B00));
            cls->setColour(Slider::thumbColourId, colour);
            cls->setLookAndFeel(&tos);

            switch (i)
            {
                case 0: cls->setTextValueSuffix(" dB Threshold"); break;
                case 1: cls->setTextValueSuffix(" Ratio"); break;
                case 4: cls->setTextValueSuffix(" dB Makeup Gain"); break;
                default: break;
            }

            SliderAttachment* controlLineSliderAttachment;
            sliderAttachments.add(controlLineSliderAttachment = 
                new SliderAttachment(processor.parameters.valueTreeState, controlLineParamter->paramID, *cls));
            cls->addListener(this);

            std::function<float(float, float, float)> convertFrom0To1Func, convertTo0To1Func;
            if (i == 0) { // Set conversion function for threshold line
                convertFrom0To1Func = [](float start, float end, float x) { return Decibels::gainToDecibels(x, start); };
                convertTo0To1Func = [](float start, float end, float x) { return Decibels::decibelsToGain(x, start); };
            }
            else if (i == 1) { // Set conversion function for ratio line
                convertFrom0To1Func = [](float start, float end, float x) { return (x <= 0) ? start : (x >= 1) ? end : 1 / (1 - x); };
                convertTo0To1Func = [](float start, float end, float x) { return (x >= end) ? 1 : (x <= start) ? 0 : 1 - 1 / x; };
            }
            NormalisableRange<double> range(cls->getMinimum(), cls->getMaximum(), convertFrom0To1Func, convertTo0To1Func);
            cls->setNormalisableRange(range);
        
            addAndMakeVisible(cls);
        }
    }
}

Level2Editor::~Level2Editor()
{
    processor.removeChangeListener(this);
    popupParent = nullptr;
}

void Level2Editor::changeListenerCallback(ChangeBroadcaster* source)
{
    Ckpa_compressorAudioProcessor* p = dynamic_cast<Ckpa_compressorAudioProcessor*> (source);
    visualiser.pushBuffer(p->bufferBefore, p->bufferAfter);
}

void Level2Editor::sliderValueChanged(Slider* slider)
{
    if (dragging)
        processor.showBubbleMessage(slider, popupParent);
    repaint(); // Necessary to prevent the control lines from lagging behind
}

void Level2Editor::sliderDragStarted(Slider* slider)
{
    dragging = true;
}

void Level2Editor::sliderDragEnded(Slider* slider)
{
    dragging = false;
}

//==============================================================================

void Level2Editor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    if (showDragMe) {
        processor.showBubbleMessage(controlLineSliders[1], popupParent, true, 1200);
        showDragMe = false;
    }
}

void Level2Editor::paintOverChildren(Graphics& g)
{
    auto r = getLocalBounds().reduced(editorMargin).toFloat();

    for (int i : {1, 0, 2})
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
