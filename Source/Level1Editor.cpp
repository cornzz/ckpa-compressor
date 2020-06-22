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

    int editorHeight = 2 * editorMargin;
    for (int i = 0; i < parameters.size() - 1; ++i) {
        if (const AudioProcessorParameterWithID* parameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {

            if (processor.parameters.parameterTypes[i] == "Slider") {
                Slider* aSlider;
                sliders.add(aSlider = new Slider());
                aSlider->setTextValueSuffix(parameter->label);
                aSlider->setTextBoxStyle(Slider::TextBoxLeft, false, sliderTextEntryBoxWidth, sliderTextEntryBoxHeight);

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

    //============ Level Meters ============

    lnf.setColour(foleys::LevelMeter::lmMeterBackgroundColour, getLookAndFeel().findColour(Slider::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterOutlineColour, Colours::transparentWhite);
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, getLookAndFeel().findColour(Slider::thumbColourId));
    String labelStrings[] = { "Input", "Output", "Gain Reduction" };

    for (int i = 0; i < 3; ++i) {
        foleys::LevelMeter* levelMeter;
        foleys::LevelMeter::MeterFlags flags = foleys::LevelMeter::SingleChannel | foleys::LevelMeter::Horizontal;
        if (i == 2)
            flags = flags | foleys::LevelMeter::HorizontalRight;
        levelMeters.add(levelMeter = new foleys::LevelMeter(flags));
        levelMeter->setLookAndFeel(&lnf);
        levelMeter->setSelectedChannel(0);
        components.add(levelMeter);
        addAndMakeVisible(levelMeter);

        Label* meterLabel;
        labels.add(meterLabel = new Label(labelStrings[i], labelStrings[i]));
        meterLabel->attachToComponent(components.getLast(), true);
        addAndMakeVisible(meterLabel);
    }

    levelMeters.getUnchecked(0)->setMeterSource(&processor.meterSourceInput);
    levelMeters.getUnchecked(1)->setMeterSource(&processor.meterSourceOutput);
    levelMeters.getUnchecked(2)->setMeterSource(&processor.meterSourceGainReduction);

    //======================================

    editorHeight += components.size() * editorPadding;
}

Level1Editor::~Level1Editor()
{
    for (auto* m : levelMeters) {
        m->setLookAndFeel(nullptr);
    }
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
            if (i + 2 != components.size()) // Leave space before last level meter
                r.removeFromTop(levelMeterHeight);
            components[i]->setBounds(r.removeFromTop(levelMeterHeight));
        }

        r = r.removeFromBottom(r.getHeight() - editorPadding);
    }
}