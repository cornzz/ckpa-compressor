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
#include "Level1Editor.h"
#include "Level2Editor.h"
#include "Level3Editor.h"

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

    SharedResourcePointer<TooltipWindow> tooltipWindow;

    void resetParameters();

    OwnedArray<ShapeButton> buttons;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    OwnedArray<ButtonAttachment> buttonAttachments;
    // Refactored by hand, since the juce Path class doesnt support neither relative coordinate commands, nor h / v / s commands -- SVG from: https://material.io/resources/icons/
    String powerButtonPath = "m 0 0 l 25 0 l 25 25 l 0 25 l 25 25 l 25 0 z m 13 3 l 11 3 l 11 13 l 13 13 z m 17.83 5.17 l 16.41 6.59 c 17.99 7.86 19 9.81 19 12 c 19 15.87 15.87 19 12 19 c 8.13 19 5 15.87 5 12 c 5 9.81 6.01 7.86 7.58 6.58 l 6.17 5.17 c 4.23 6.82 3 9.26 3 12 c 3 16.97 7.03 21 12 21 c 16.97 21 21 16.97 21 12 c 21 9.26 19.77 6.82 17.83 5.17 z";
    String resetButtonPath = "m 0 0 l 25 0 l 25 25 l 0 25 l 25 25 l 25 0 z m 14 12 c 14 10.9 13.1 10 12 10 c 10.9 10 10 10.9 10 12 c 10 13.1 10.9 14 12 14 c 13.1 14 14 13.1 14 12 z m 12 3 c 7.03 3 3 7.03 3 12 l 0 12 l 4 16 l 8 12 l 5 12 c 5 8.13 8.13 5 12 5 c 15.87 5 19 8.13 19 12 c 19 15.87 15.87 19 12 19 c 10.49 19 9.09 18.51 7.94 17.7 l 6.52 19.14 c 8.04 20.3 9.94 21 12 21 c 16.97 21 21 16.97 21 12 c 21 7.03 16.97 3 12 3 z";    
    
    MainTabbedComponent tabs;

    //======================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ckpa_compressorAudioProcessorEditor)
};
