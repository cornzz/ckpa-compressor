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

#include "Level3Editor.h"
#include <math.h>

//==============================================================================

Level3Editor::Level3Editor(Ckpa_compressorAudioProcessor& p) : processor(p)
{
    compressionSlider = std::make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
    compressionSlider->setLookAndFeel(&tos);
    compressionSlider->setRange(1, 25, 0.1);
    compressionSlider->setValue(25);
    
    compressionSlider->addListener(this);
    addAndMakeVisible(*compressionSlider);

    rand = std::make_unique<Random>();
    anim = std::make_unique<ComponentAnimator>();
    Colour c = findColour(Slider::thumbColourId);
    for (int i = 0; i < 40; ++i) {
        Atom* a;
        atoms.add(a = new Atom(rand.get(), anim.get(), c));
        addAndMakeVisible(a);
    }
}

Level3Editor::~Level3Editor()
{
}

void Level3Editor::sliderValueChanged(Slider* slider)
{
    circleDiameter = (slider->getPositionOfValue(slider->getValue()) + 30) * 2.0f;
    repaint();

    int atomSize = circleDiameter * 0.1;
    for (int j = 0; j < 4; ++j) {
        for (int i = 0; i < 10; ++i) {
            atoms.getUnchecked(i + (j * 10))->setBounds(Rectangle<int>(i * atomSize, j * atomSize, atomSize, atomSize));
        }
    }
    anim->cancelAllAnimations(false);

    //processor.paramThreshold.updateValue(-30);
}

void Level3Editor::paint(Graphics& g)
{
    Colour bkg = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    g.fillAll(bkg);

    Rectangle<float> r = getLocalBounds().toFloat().reduced(editorMargin);
    //g.setColour(bkg.darker(0.15));
    //g.fillRect(r);

    circleDiameter = (circleDiameter == 0) ? r.getHeight() : circleDiameter;
    auto rect = Rectangle<float>(circleDiameter, circleDiameter).withCentre(r.getCentre());
    g.setColour(findColour(Slider::thumbColourId));
    g.drawEllipse(rect, 2.0f);
}

void Level3Editor::resized()
{
    Rectangle<int> r = getLocalBounds().reduced(editorMargin);

    r = r.removeFromRight(r.getWidth() / 2)
        .removeFromBottom(r.getHeight() / 2 + 10)
        .removeFromTop(20)
        .withTrimmedLeft(30)
        .removeFromLeft(r.getHeight() / 2 - 20);
    compressionSlider->setBounds(r);
}

//==============================================================================

Atom::Atom(Random* r, ComponentAnimator* a, Colour c) : rand(r),
    anim(a)
{
    ae = std::make_unique<AtomEllipse>(c);
    addAndMakeVisible(*ae);
    
    anim->addChangeListener(this);
}

Atom::~Atom()
{
}

void Atom::paint(Graphics& g)
{
    g.setColour(Colours::black);
    g.drawRect(getLocalBounds());
}

void Atom::resized()
{
    if (!init) {
        ae->setBounds(getLocalBounds().withSizeKeepingCentre(6, 6));
        anim->sendChangeMessage();
        init = true;
    }
}

void Atom::changeListenerCallback(ChangeBroadcaster* source)
{
    ComponentAnimator* anim = dynamic_cast<ComponentAnimator*>(source);

    if (!anim->isAnimating(ae.get())) {
        int newX, newY, dist;
        while (true) {
            newX = rand->nextInt(getWidth() - ae->getWidth());
            newY = rand->nextInt(getHeight() - ae->getHeight());
            dist = hypot(newX - ae->getX(), newY - ae->getY());
            if (dist >= (getWidth() - ae->getWidth()) / 2 || ae->getWidth() >= getWidth() / 2)
                break;
        }
        Rectangle<int> newBounds = ae->getBounds().withPosition(newX, newY);
        anim->animateComponent(ae.get(), newBounds, 1.0f, dist * 30, false, 1, 1);
    }
}

