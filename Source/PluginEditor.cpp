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

//==============================================================================

Ckpa_compressorAudioProcessorEditor::Ckpa_compressorAudioProcessorEditor(Ckpa_compressorAudioProcessor& p)
    : AudioProcessorEditor(&p),
    processor(p),
    tabs(p, new Level1Editor(p), new Level2Editor(p), new Level3Editor(p))
{
    setSize(editorWidth, 383);
    addAndMakeVisible(tabs);

    //======================================

    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    //Power Button
    ShapeButton* powerButton;
    buttons.add(powerButton = new ShapeButton("powerButton",
        findColour(Slider::thumbColourId),
        findColour(Slider::thumbColourId).brighter(0.1),
        findColour(Slider::thumbColourId).darker(0.15)));
    // On and off colour sets are switched around, since this used to be the "bypass button"
    powerButton->setOnColours(findColour(ResizableWindow::backgroundColourId),
        findColour(ResizableWindow::backgroundColourId).brighter(0.1),
        findColour(Slider::thumbColourId).darker(0.2));
    powerButton->shouldUseOnColours(true);
    powerButton->setTooltip("Bypass");
    Path pbPath;
    pbPath.restoreFromString(powerButtonPath);
    powerButton->setShape(pbPath, true, true, true);

    const AudioProcessorParameterWithID* buttonParameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[5]);
    ButtonAttachment* powerButtonAttachment;
    buttonAttachments.add(powerButtonAttachment = new ButtonAttachment(processor.parameters.valueTreeState, buttonParameter->paramID, *powerButton));
    addAndMakeVisible(powerButton);
    powerButton->setClickingTogglesState(true);
    Rectangle<int> rPower = getLocalBounds();
    powerButton->setBounds(rPower.removeFromBottom(39).removeFromRight(39)); // TODO: Should be done in resized()

    //Reset Button
    ShapeButton* resetButton;
    buttons.add(resetButton = new ShapeButton("resetButton",
        getLookAndFeel().findColour(Slider::thumbColourId),
        getLookAndFeel().findColour(Slider::thumbColourId).brighter(0.1),
        getLookAndFeel().findColour(Slider::thumbColourId).darker(0.15)));
    resetButton->setTooltip("Reset");
    Path rbPath;
    rbPath.restoreFromString(resetButtonPath);
    resetButton->setShape(rbPath, true, true, true);

    resetButton->onClick = [this] { resetParameters(); };
    addAndMakeVisible(resetButton);
    Rectangle<int> rReset = getLocalBounds();
    resetButton->setBounds(rReset.removeFromBottom(39).withTrimmedRight(39).removeFromRight(39)); // TODO: Should be done in resized()
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
}

void Ckpa_compressorAudioProcessorEditor::resetParameters()
{
    processor.paramThreshold.resetParameter();
    processor.paramRatio.resetParameter();
    processor.paramAttack.resetParameter();
    processor.paramRelease.resetParameter();
    processor.paramMakeupGain.resetParameter();
    processor.paramCompression.resetParameter();
}
