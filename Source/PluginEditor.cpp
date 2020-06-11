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
	inputMeter.setLookAndFeel (&lnf);
	inputMeter.setMeterSource (&processor.meterSourceInput);
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
        
        if (foleys::LevelMeter* aLevelMeter = dynamic_cast<foleys::LevelMeter*> (components[i]))
        {
            if (i + 2 != components.size())
                r.removeFromTop(levelMeterHeight);
            components[i]->setBounds(r.removeFromTop(levelMeterHeight));
        }

        r = r.removeFromBottom(r.getHeight() - editorPadding);
    }
}

//=============================== Level 2 ======================================

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

    //======================================

    setSize(editorWidth, 400); // TODO: unnecessary?
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

//============================= Main Editor ====================================

Ckpa_compressorAudioProcessorEditor::Ckpa_compressorAudioProcessorEditor(Ckpa_compressorAudioProcessor& p)
    : AudioProcessorEditor(&p),
    processor(p),
    tabs(p, new Level1Editor(p), new Level2Editor(p), new Level1Editor(p))
{
    addAndMakeVisible(tabs);
    setSize(editorWidth, 382);

    //======================================
    const Array<AudioProcessorParameter*> parameters = processor.getParameters();

    ShapeButton* powerButton;
    buttons.add(powerButton = new ShapeButton("powerButton",
        getLookAndFeel().findColour(Slider::thumbColourId),
        getLookAndFeel().findColour(Slider::thumbColourId).brighter(0.1),
        getLookAndFeel().findColour(Slider::thumbColourId).darker(0.15)));
    // On and off colour sets are switched around, since this used to be the "bypass button"
    powerButton->setOnColours(getLookAndFeel().findColour(ResizableWindow::backgroundColourId),
        getLookAndFeel().findColour(ResizableWindow::backgroundColourId).brighter(0.1),
        getLookAndFeel().findColour(Slider::thumbColourId).darker(0.2));
    powerButton->shouldUseOnColours(true);
    Path path;
    path.restoreFromString(powerButtonPath);
    powerButton->setShape(path, true, true, true);

    const AudioProcessorParameterWithID* buttonParameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[5]);
    ButtonAttachment* powerButtonAttachment;
    buttonAttachments.add(powerButtonAttachment = new ButtonAttachment(processor.parameters.valueTreeState, buttonParameter->paramID, *powerButton));
    addAndMakeVisible(powerButton);
    powerButton->setClickingTogglesState(true);

    Rectangle<int> r = getLocalBounds();
    powerButton->setBounds(r.removeFromBottom(39).removeFromRight(39)); // TODO: Should be done in resized()
}

Ckpa_compressorAudioProcessorEditor::~Ckpa_compressorAudioProcessorEditor()
{
}

void Ckpa_compressorAudioProcessorEditor::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void Ckpa_compressorAudioProcessorEditor::resized()
{
    Rectangle<int> r = getLocalBounds();
    tabs.setBounds(r);
    //buttons.getLast()->setBounds(r.removeFromBottom(40).removeFromRight(40));
}