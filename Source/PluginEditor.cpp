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

Ckpa_compressorAudioProcessorEditor::Ckpa_compressorAudioProcessorEditor (Ckpa_compressorAudioProcessor& p)
    : AudioProcessorEditor (&p), 
    processor (p), 
    tabs (p),
    powerButton("powerButton", DrawableButton::ImageFitted)
{
    addAndMakeVisible(tabs);
    setSize(editorWidth, 400);

    //======================================

    std::unique_ptr<Drawable> d = Drawable::createFromSVG(*XmlDocument::parse(powerButtonSVG));
    std::unique_ptr<Drawable> normal = d->createCopy();
    normal->replaceColour(Colours::black, getLookAndFeel().findColour(Slider::thumbColourId));
    std::unique_ptr<Drawable> over = d->createCopy();
    over->replaceColour(Colours::black, getLookAndFeel().findColour(Slider::thumbColourId).brighter(0.15));
    std::unique_ptr<Drawable> down = d->createCopy();
    down->replaceColour(Colours::black, getLookAndFeel().findColour(Slider::thumbColourId).darker(0.15));
    powerButton.setImages(&(*normal), &(*over), &(*down), &(*d));
    addAndMakeVisible(powerButton);
}

Ckpa_compressorAudioProcessorEditor::~Ckpa_compressorAudioProcessorEditor()
{
}

void Ckpa_compressorAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void Ckpa_compressorAudioProcessorEditor::resized()
{
    tabs.setBounds(getLocalBounds());
    powerButton.setBounds(getLocalBounds().removeFromBottom(30).removeFromRight(30));
}

//=============================== Level 1 ======================================

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

            //======================================

            else if (processor.parameters.parameterTypes[i] == "ToggleButton") {
                ToggleButton* aButton;
                toggles.add(aButton = new ToggleButton());
                aButton->setToggleState(parameter->getDefaultValue(), dontSendNotification);

                ButtonAttachment* aButtonAttachment;
                buttonAttachments.add(aButtonAttachment =
                    new ButtonAttachment(processor.parameters.valueTreeState, parameter->paramID, *aButton));

                components.add(aButton);
                editorHeight += buttonHeight;
            }

            //======================================

            else if (processor.parameters.parameterTypes[i] == "ComboBox") {
                ComboBox* aComboBox;
                comboBoxes.add(aComboBox = new ComboBox());
                aComboBox->setEditableText(false);
                aComboBox->setJustificationType(Justification::left);
                aComboBox->addItemList(processor.parameters.comboBoxItemLists[comboBoxCounter++], 1);

                ComboBoxAttachment* aComboBoxAttachment;
                comboBoxAttachments.add(aComboBoxAttachment =
                    new ComboBoxAttachment(processor.parameters.valueTreeState, parameter->paramID, *aComboBox));

                components.add(aComboBox);
                editorHeight += comboBoxHeight;
            }

            //======================================

            Label* aLabel;
            labels.add(aLabel = new Label(parameter->name, parameter->name));
            aLabel->attachToComponent(components.getLast(), true);
            addAndMakeVisible(aLabel);

            components.getLast()->setName(parameter->name);
            components.getLast()->setComponentID(parameter->paramID);
            addAndMakeVisible(components.getLast());
        }
    }
		// adjust the colours to how you like them, e.g.
    lnf.setColour(foleys::LevelMeter::lmBackgroundColour,getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    lnf.setColour(foleys::LevelMeter::lmMeterGradientLowColour, getLookAndFeel().findColour(Slider::ColourIds::trackColourId));
    lnf.setColour(foleys::LevelMeter::lmTicksColour, Colours::black);

    //======================================
    //Levelmeter for Input
	inputMeter.setLookAndFeel (&lnf);
	inputMeter.setMeterSource (&processor.getMeterSource());
    inputMeter.setSelectedChannel(0);
    components.add(&inputMeter);
    addAndMakeVisible(inputMeter);

    Label* inputLabel;
    labels.add(inputLabel = new Label("InputGain", "InputGain"));
    inputLabel->attachToComponent(components.getLast(), true);
    addAndMakeVisible(inputLabel);

    //Levelmeter for Output
    outputMeter.setLookAndFeel(&lnf);
    outputMeter.setMeterSource(&processor.getMeterSource());
    outputMeter.setSelectedChannel(0);
    components.add(&outputMeter);
    addAndMakeVisible(outputMeter);
    Label* outputLabel;
    labels.add(outputLabel = new Label("OutputGain", "OutputGain"));
    outputLabel->attachToComponent(components.getLast(), true);
    addAndMakeVisible(outputLabel);

    //Levelmeter for Differenz between Input and Output
    gainMeter.setLookAndFeel(&lnf);
    gainMeter.setMeterSource(&processor.getMeterSource());
    gainMeter.setSelectedChannel(0);
    components.add(&gainMeter);
    addAndMakeVisible(gainMeter);
    Label* gainLabel;
    labels.add(gainLabel = new Label("DifferenzGain", "DifferenzGain"));
    gainLabel->attachToComponent(components.getLast(), true);
    addAndMakeVisible(gainLabel);
    //======================================

    editorHeight += components.size() * editorPadding;
    setSize(editorWidth, editorHeight);
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

        if (ToggleButton* aButton = dynamic_cast<ToggleButton*> (components[i]))
            components[i]->setBounds(r.removeFromTop(buttonHeight));

        if (ComboBox* aComboBox = dynamic_cast<ComboBox*> (components[i]))
            components[i]->setBounds(r.removeFromTop(comboBoxHeight));
        
        if (foleys::LevelMeter* aLevelMeter = dynamic_cast<foleys::LevelMeter*> (components[i]))
            components[i]->setBounds(r.removeFromTop(levelMeterHeight));

        r = r.removeFromBottom(r.getHeight() - editorPadding);
    }
}

//=============================== Level 2 ======================================

Level2Editor::Level2Editor(Ckpa_compressorAudioProcessor& p) : processor(p)
{
    addAndMakeVisible(processor.visualiser);
    addAndMakeVisible(processor.thresholdLine);

    setSize(editorWidth, 500);
}

Level2Editor::~Level2Editor()
{
}

void Level2Editor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void Level2Editor::resized()
{
    Rectangle<int> r = getLocalBounds().reduced(editorMargin);

    Rectangle<int> rVis = r.removeFromTop(200);
    processor.visualiser.setBounds(rVis);
    processor.thresholdLine.setBounds(rVis);
}
