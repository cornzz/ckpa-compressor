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

class Atom : public Component,
    public ChangeListener
{
public:
    Atom(Random*, ComponentAnimator*, Colour);
    ~Atom();

    void paint(Graphics&) override;
    void resized() override;
    void resize(Rectangle<float>);

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

private:
    bool init = false;

    std::unique_ptr<AtomEllipse> ae;

    Random* rand;
    ComponentAnimator* anim;

    int width, height;
};

class Level3Editor : public Component,
    Slider::Listener
{
public:
    Level3Editor(Ckpa_compressorAudioProcessor&);
    ~Level3Editor();

    void sliderValueChanged(Slider*) override;

    void paint(Graphics&) override;
    void resized() override;

    void resizeAtoms();

private:
    Ckpa_compressorAudioProcessor& processor;

    ThumbOnlySlider tos;
    std::unique_ptr<Slider> compressionSlider;

    float circleDiameter = 0;

    std::unique_ptr<Random> rand;
    std::unique_ptr<ComponentAnimator> anim;
    OwnedArray<Atom> atoms;

    enum {
        editorWidth = 500,
        editorMargin = 20,
        editorPadding = 10,
    };
};