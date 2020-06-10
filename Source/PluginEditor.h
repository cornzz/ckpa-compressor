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

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "MainTabbedComponent.h"
#include "Visualiser.h"

//==============================================================================

class Level1Editor : public Component
{
public:
    Level1Editor(Ckpa_compressorAudioProcessor&);
    ~Level1Editor();

    void paint(Graphics&) override;
    void resized() override;

private:
    Ckpa_compressorAudioProcessor& processor;
	foleys::LevelMeterLookAndFeel lnf;
    foleys::LevelMeter inputMeter { foleys::LevelMeter::SingleChannel | foleys::LevelMeter::Horizontal };
    foleys::LevelMeter outputMeter { foleys::LevelMeter::SingleChannel | foleys::LevelMeter::Horizontal };
    foleys::LevelMeter gainReductionMeter { foleys::LevelMeter::SingleChannel | foleys::LevelMeter::Horizontal };

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,

        sliderTextEntryBoxWidth = 100,
        sliderTextEntryBoxHeight = 25,
        sliderHeight = 25,
        buttonHeight = 25,
        comboBoxHeight = 25,
        levelMeterHeight = 20,
        labelWidth = 100,
    };

    //======================================

    OwnedArray<Slider> sliders;
    OwnedArray<ToggleButton> toggles;
    OwnedArray<ComboBox> comboBoxes;

    OwnedArray<Label> labels;
    Array<Component*> components;

    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    OwnedArray<SliderAttachment> sliderAttachments;
    OwnedArray<ButtonAttachment> buttonAttachments;
    OwnedArray<ComboBoxAttachment> comboBoxAttachments;
};

//==============================================================================

class Level2Editor : public Component,
    public ChangeListener
{
public:
    Level2Editor(Ckpa_compressorAudioProcessor&);
    ~Level2Editor();

    void changeListenerCallback(ChangeBroadcaster* source) override;

    void paint(Graphics&) override;
    void resized() override;

private:
    Ckpa_compressorAudioProcessor& processor;

    Visualiser visualiser;

    DraggableHorizontalLine dhl;
    OwnedArray<Slider> controlLines;
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    OwnedArray<SliderAttachment> sliderAttachments;

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,
    };
};

//==============================================================================

class Ckpa_compressorAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Ckpa_compressorAudioProcessorEditor (Ckpa_compressorAudioProcessor&);
    ~Ckpa_compressorAudioProcessorEditor();

    //======================================

    void paint (Graphics&) override;
    void resized() override;
    

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Ckpa_compressorAudioProcessor& processor;

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,
    };

    OwnedArray<DrawableButton> buttons;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    OwnedArray<ButtonAttachment> buttonAttachments;
    String powerButtonSVG = "<svg xmlns='http://www.w3.org/2000/svg' height='24' viewBox='0 0 24 24' width='24'><path d='M 0 0 h 24 v 24 H 0 z' fill='none'/><path d='M 13 3 h -2 v 10 h 2 z m 4.83 2.17 l -1.42 1.42 C 17.99 7.86 19 9.81 19 12 c 0 3.87 -3.13 7 -7 7 s -7 -3.13 -7 -7 c 0 -2.19 1.01 -4.14 2.58 -5.42 L 6.17 5.17 C 4.23 6.82 3 9.26 3 12 c 0 4.97 4.03 9 9 9 s 9 -4.03 9 -9 c 0 -2.74 -1.23 -5.18 -3.17 -6.83 z'/></svg>";
    String powerButtonPath = "M 0 0 h 24 v 24 H 0 z M 13 3 h -2 v 10 h 2 z m 4.83 2.17 l -1.42 1.42 C 17.99 7.86 19 9.81 19 12 c 0 3.87 -3.13 7 -7 7 s -7 -3.13 -7 -7 c 0 -2.19 1.01 -4.14 2.58 -5.42 L 6.17 5.17 C 4.23 6.82 3 9.26 3 12 c 0 4.97 4.03 9 9 9 s 9 -4.03 9 -9 c 0 -2.74 -1.23 -5.18 -3.17 -6.83 z";

    MainTabbedComponent tabs;

    //======================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ckpa_compressorAudioProcessorEditor)
};
