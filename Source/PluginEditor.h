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

//==============================================================================
/**
*/
class Level1Editor : public Component
{
public:
    Level1Editor(Ckpa_compressorAudioProcessor&);
    ~Level1Editor();

    void paint(Graphics&) override;
    void resized() override;

private:
    Ckpa_compressorAudioProcessor& processor;

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,

        sliderTextEntryBoxWidth = 100,
        sliderTextEntryBoxHeight = 25,
        sliderHeight = 25,
        buttonHeight = 25,
        comboBoxHeight = 25,
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

class Level2Editor : public Component
{
public:
    Level2Editor(Ckpa_compressorAudioProcessor&);
    ~Level2Editor();

    void paint(Graphics&) override;
    void resized() override;

private:
    Ckpa_compressorAudioProcessor& processor;

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,
    };
};

//==============================================================================

struct MainTabbedComponent : public TabbedComponent
{
    MainTabbedComponent(Ckpa_compressorAudioProcessor& p) : TabbedComponent(TabbedButtonBar::TabsAtBottom)
    {
        auto colour = findColour(ResizableWindow::backgroundColourId);

        addTab("Level 1", colour, new Level1Editor(p), true);
        addTab("Level 2", colour, new Level2Editor(p), true);
        addTab("Level 3", colour, new Level1Editor(p), true);
        setOutline(0.0f);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainTabbedComponent)
};

//==============================================================================

class Ckpa_compressorAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Ckpa_compressorAudioProcessorEditor (Ckpa_compressorAudioProcessor&);
    ~Ckpa_compressorAudioProcessorEditor();

    //==============================================================================
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

    //======================================

    MainTabbedComponent tabs;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ckpa_compressorAudioProcessorEditor)
};
