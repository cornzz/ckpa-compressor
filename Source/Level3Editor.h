/*
  ==============================================================================

    Code by cornzz.
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

#include <math.h>
#include <vector>
#include <random>

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class Atom : public Component,
             public ChangeListener
{
public:
    Atom(Random*, Colour);
    ~Atom();

    void paint(Graphics&) override;
    void resized() override;
    void resize(Rectangle<int>);

    void changeListenerCallback(ChangeBroadcaster* source) override;

    struct AtomEllipse : public Component
    {
        AtomEllipse(Colour c) : c(c)
        {
        }

        void paint(Graphics& g) override {
            Rectangle<float> r = getLocalBounds().toFloat();
            g.setColour(c);
            g.fillEllipse(r);
        }

        Colour c;
    };

    std::unique_ptr<ComponentAnimator> anim;

private:
    bool init = false;

    std::unique_ptr<AtomEllipse> ae;

    Random* rand;

    int width, height;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Atom)
};

class Level3Editor : public Component,
                     public Slider::Listener,
                     public Timer
{
public:
    Level3Editor(Ckpa_compressorAudioProcessor& p, Component* parentForPopup);
    ~Level3Editor();

    void sliderValueChanged(Slider*) override;
    void sliderDragStarted(Slider*) override;
    void sliderDragEnded(Slider*) override;
    void timerCallback() override;

    void paint(Graphics&) override;
    void resized() override;

    void resizeAtoms();

private:
    Ckpa_compressorAudioProcessor& processor;

    enum {
        editorWidth = 500,
        editorMargin = 20,
        editorPadding = 10
    };
    
    bool init = true;
    bool dragging = false;

    float circleDiameter = 0;
    int numAtoms = 74;

    OwnedArray<Slider> sliders;
    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    OwnedArray<SliderAttachment> sliderAttachments;

    ThumbOnlySlider tos;
    Slider* compressionSlider;

    Component* popupParent;
    bool showDragMe = false;

    std::unique_ptr<Random> rand;
    OwnedArray<Atom> atoms;
    std::vector<int> visibleAtoms, invisibleAtoms;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Level3Editor)
};