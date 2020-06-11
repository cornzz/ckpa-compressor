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

#include "Level1Editor.h"

//==============================================================================

Level1Editor::Level1Editor(Ckpa_compressorAudioProcessor& p) : processor(p)
{
    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int comboBoxCounter = 0;

    int editorHeight = 2 * editorMargin;
    for (int i = 0; i < parameters.size(); ++i) {
        if (const AudioProcessorParameterWithID* parameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {

            if (processor.parameters.parameterTypes[i] == "Slider") {
                Slider* aSlider;
                sliders.add(aSlider = new Slider());
                aSlider->setTextValueSuffix(parameter->label);
                aSlider->setTextBoxStyle(Slider::TextBoxLeft,
                    false,
                    sliderTextEntryBoxWidth,
                    sliderTextEntryBoxHeight);

                SliderAttachment* aSliderAttachment;
                sliderAttachments.add(aSliderAttachment =
                    new SliderAttachment(processor.parameters.valueTreeState, parameter->paramID, *aSlider));

                components.add(aSlider);
                editorHeight += sliderHeight;
            }

            Label* aLabel;
            labels.add(aLabel = new Label(parameter->name, parameter->name));
            aLabel->attachToComponent(components.getLast(), true);
            addAndMakeVisible(aLabel);

            components.getLast()->setName(parameter->name);
            components.getLast()->setComponentID(parameter->paramID);
            addAndMakeVisible(components.getLast());
        }
    }

    lnf.setColour(foleys::LevelMeter::lmMeterBackgroundColour, getLookAndFeel().findColour(Slider::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterOutlineColour, Colours::transparentWhite);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, getLookAndFeel().findColour(Slider::thumbColourId));

    //======================================
    //Levelmeter for input
    inputMeter.setLookAndFeel(&lnf);
    inputMeter.setMeterSource(&processor.meterSourceInput);
    inputMeter.setSelectedChannel(0);
    components.add(&inputMeter);
    addAndMakeVisible(inputMeter);

    Label* inputLabel;
    labels.add(inputLabel = new Label("Input", "Input"));
    inputLabel->attachToComponent(components.getLast(), true);
    addAndMakeVisible(inputLabel);

    // Levelmeter for output
    outputMeter.setLookAndFeel(&lnf);
    outputMeter.setMeterSource(&processor.meterSourceOutput);
    outputMeter.setSelectedChannel(0);
    components.add(&outputMeter);
    addAndMakeVisible(outputMeter);

    Label* outputLabel;
    labels.add(outputLabel = new Label("Output", "Output"));
    outputLabel->attachToComponent(components.getLast(), true);
    addAndMakeVisible(outputLabel);

    // Levelmeter for gain reduction
    gainReductionMeter.setLookAndFeel(&lnf);
    gainReductionMeter.setMeterSource(&processor.meterSourceGainReduction);
    gainReductionMeter.setSelectedChannel(0);
    components.add(&gainReductionMeter);
    addAndMakeVisible(gainReductionMeter);

    Label* gainLabel;
    labels.add(gainLabel = new Label("GainReduction", "Gain reduction"));
    gainLabel->attachToComponent(components.getLast(), true);
    addAndMakeVisible(gainLabel);

    //======================================

    editorHeight += components.size() * editorPadding;
}

Level1Editor::~Level1Editor()
{
    inputMeter.setLookAndFeel(nullptr);
}

void Level1Editor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void Level1Editor::resized()
{
    Rectangle<int> r = getLocalBounds().reduced(editorMargin);
    r = r.removeFromRight(r.getWidth() - labelWidth);

    for (int i = 0; i < components.size(); ++i) {
        if (Slider* aSlider = dynamic_cast<Slider*> (components[i]))
            components[i]->setBounds(r.removeFromTop(sliderHeight));

        if (foleys::LevelMeter* aLevelMeter = dynamic_cast<foleys::LevelMeter*> (components[i]))
        {
            if (i + 2 != components.size())
                r.removeFromTop(levelMeterHeight);
            components[i]->setBounds(r.removeFromTop(levelMeterHeight));
        }

        r = r.removeFromBottom(r.getHeight() - editorPadding);
    }
}