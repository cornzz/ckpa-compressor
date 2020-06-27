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
#include "Visualiser.h"

//==============================================================================

class Level2Editor : public Component,
                     public ChangeListener,
                     public Slider::Listener
{
public:
    Level2Editor(Ckpa_compressorAudioProcessor&);
    ~Level2Editor();

    void changeListenerCallback(ChangeBroadcaster* source) override;
    void sliderValueChanged(Slider* slider) override;

    void paint(Graphics&) override;
    void paintOverChildren(Graphics& g) override;
    void resized() override;


private:
    Ckpa_compressorAudioProcessor& processor;

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10
    };

    Visualiser visualiser;

    ThumbOnlySlider tos;
    OwnedArray<Slider> controlLineSliders;
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    OwnedArray<SliderAttachment> sliderAttachments;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Level2Editor)
};