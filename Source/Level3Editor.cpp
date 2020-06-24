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
    ScopedValueSetter<bool> svs(init, false);

    rand = std::make_unique<Random>();
    anim = std::make_unique<ComponentAnimator>();
    Colour c = findColour(Slider::thumbColourId);
    for (int i = 0; i < 74; ++i) {
        Atom* a;
        atoms.add(a = new Atom(rand.get(), anim.get(), c));
        addAndMakeVisible(a, 0);
    }

    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int ind[] = { 0, 1, 4 };
    for (int i : ind) {
        if (const AudioProcessorParameterWithID* parameter = dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {
            Slider* aSlider;
            sliders.add(aSlider = new Slider());

            aSlider->addListener(this);
            sliderAttachments.add(new SliderAttachment(processor.parameters.valueTreeState, parameter->paramID, *aSlider));
        }
    }

    sliders.add(compressionSlider = new Slider(Slider::LinearHorizontal, Slider::NoTextBox));
    compressionSlider->setLookAndFeel(&tos);
    //compressionSlider->setPopupDisplayEnabled(true, false, this);
    
    compressionSlider->addListener(this);
    sliderAttachments.add(new SliderAttachment(processor.parameters.valueTreeState, "compression", *compressionSlider));
    addAndMakeVisible(*compressionSlider);
}

Level3Editor::~Level3Editor()
{
    compressionSlider->setLookAndFeel(nullptr);
}

void Level3Editor::sliderValueChanged(Slider* slider)
{
    if (!init)
        return;

    double compressionValue;

    if (slider == compressionSlider) { // Compression circle changed
        DBG("Slider Changed: Compression");
        compressionValue = slider->getValue();

        sliders.getFirst()->setValue(compressionValue * 1.5 - 30);                   // Threshold
        sliders.getUnchecked(1)->setValue(pow(compressionValue - 20, 2) / 25 + 1.0); // Ratio
        sliders.getUnchecked(2)->setValue(-0.2 * compressionValue + 4);              // Makeup gain
    }
    else { // Threshold / ratio / makeup gain changed
        if (slider == sliders.getFirst())
            DBG("Slider Changed: Threshold");
        else if (slider == sliders.getUnchecked(1))
            DBG("Slider Changed: Ratio");
        else if (slider == sliders.getUnchecked(2))
            DBG("Slider Changed: Makeup gain");
        double tempThresh = sliders.getFirst()->getValue();
        double tempRatio = sliders.getUnchecked(1)->getValue();
        double tempMakeup = sliders.getUnchecked(2)->getValue();

        tempThresh = jlimit(0.0, 20.0, (tempThresh + 30) / 1.5);
        tempRatio = jlimit(0.0, 20.0, 20 - sqrt((tempRatio - 1) * 25));
        tempMakeup = jlimit(0.0, 20.0, (tempMakeup - 4) / -0.2);
        compressionValue = (tempThresh + tempRatio + tempMakeup) / 3;

        sliders.getLast()->setValue(compressionValue, dontSendNotification);
    }

    circleDiameter = (compressionSlider->getPositionOfValue(compressionValue) + 30) * 2.0f;
    repaint();
    resizeAtoms();
}

void Level3Editor::paint(Graphics& g)
{
    Colour bkg = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    g.fillAll(bkg);

    Rectangle<float> r = getLocalBounds().toFloat().reduced(editorMargin);
    //g.setColour(Colours::black);
    //g.drawRect(r, 1.0f);

    auto rect = Rectangle<float>(circleDiameter, circleDiameter).withCentre(r.getCentre());
    g.setColour(findColour(Slider::thumbColourId));
    g.drawEllipse(rect, 2.0f);
    //g.setColour(Colours::black);
    //g.drawRect(rect, 1.0f);
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
    sliderValueChanged(compressionSlider);
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
    //g.setColour(Colours::black);
    //g.drawRect(getLocalBounds());
}

void Atom::resized()
{
    if (!init) { // Set bounds of AtomEllipse only on init
        ae->setBounds(getLocalBounds().withSizeKeepingCentre(6, 6));
        anim->sendChangeMessage(); // Start animation loop
        init = true;
    }
}

void Atom::resize(Rectangle<float> newBounds)
{
    if (!init) {
        setBounds(newBounds.toNearestInt());
        return;
    }

    // Set position of ae in new bounds relative to position in old bounds
    //Rectangle<int> destination = anim->getComponentDestination(ae.get());
    anim->cancelAnimation(ae.get(), false);
    Rectangle<int> oldBounds = getBounds();
    float relativeX = ae->getX() / (float) oldBounds.getWidth();
    float relativeY = ae->getY() / (float) oldBounds.getHeight();
    ae->setTopLeftPosition(newBounds.getWidth() * relativeX, newBounds.getHeight() * relativeY);
    setBounds(newBounds.toNearestInt());

    //destination = destination.constrainedWithin(newBounds.toNearestInt());
    //int dist = hypot(destination.getX() - ae->getX(), destination.getY() - ae->getY());
    //anim->animateComponent(ae.get(), destination, 1.0f, dist * 30, false, 1, 1);
}

void Atom::changeListenerCallback(ChangeBroadcaster* source)
{
    ComponentAnimator* anim = dynamic_cast<ComponentAnimator*>(source);

    if (!anim->isAnimating(ae.get())) { // AtomEllipse reached destination, find new dest. coordinates
        int newX, newY, dist;
        while (true) {
            newX = rand->nextInt(getWidth() - ae->getWidth());  // TODO: this is bad
            newY = rand->nextInt(getHeight() - ae->getHeight());
            dist = hypot(newX - ae->getX(), newY - ae->getY());
            if (dist >= (getWidth() - ae->getWidth()) / 2 || ae->getWidth() >= getWidth() / 2)
                break;
        }
        Rectangle<int> newBounds = ae->getBounds().withPosition(newX, newY);
        anim->animateComponent(ae.get(), newBounds, 1.0f, dist * 30, false, 1, 1);
    }
}


void Level3Editor::resizeAtoms()
{
    auto r = Rectangle<float>(circleDiameter, circleDiameter).withCentre(getLocalBounds().toFloat().getCentre());;
    float atomSize = circleDiameter * 0.1;

    atoms.getUnchecked(0)->resize(Rectangle<float>(r.getCentreX() - 4.8 * atomSize, r.getY() + r.getHeight() / 2 - atomSize - 0.8 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(1)->resize(Rectangle<float>(r.getCentreX() - 5 * atomSize,   r.getY() + r.getHeight() / 2 - atomSize,                  atomSize, atomSize));
    atoms.getUnchecked(2)->resize(Rectangle<float>(r.getCentreX() - 5 * atomSize,   r.getY() + r.getHeight() / 2,                             atomSize, atomSize));
    atoms.getUnchecked(3)->resize(Rectangle<float>(r.getCentreX() - 4.8 * atomSize, r.getY() + r.getHeight() / 2 + atomSize - 0.2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(4)->resize(Rectangle<float>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(5)->resize(Rectangle<float>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(6)->resize(Rectangle<float>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(7)->resize(Rectangle<float>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(8)->resize(Rectangle<float>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(9)->resize(Rectangle<float>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(10)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(11)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(12)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(13)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(14)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(15)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(16)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(17)->resize(Rectangle<float>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(18)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 4.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(19)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 3.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(20)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(21)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(22)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(23)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(24)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(25)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(26)->resize(Rectangle<float>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 3.5 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(27)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(28)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(29)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(30)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(31)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(32)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(33)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(34)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(35)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(36)->resize(Rectangle<float>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 4 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(37)->resize(Rectangle<float>(r.getCentreX() + 3.8 * atomSize, r.getY() + r.getHeight() / 2 - atomSize - 0.8 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(38)->resize(Rectangle<float>(r.getCentreX() + 4 * atomSize,   r.getY() + r.getHeight() / 2 - atomSize,                  atomSize, atomSize));
    atoms.getUnchecked(39)->resize(Rectangle<float>(r.getCentreX() + 4 * atomSize,   r.getY() + r.getHeight() / 2,                             atomSize, atomSize));
    atoms.getUnchecked(40)->resize(Rectangle<float>(r.getCentreX() + 3.8 * atomSize, r.getY() + r.getHeight() / 2 + atomSize - 0.2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(41)->resize(Rectangle<float>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(42)->resize(Rectangle<float>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(43)->resize(Rectangle<float>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(44)->resize(Rectangle<float>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(45)->resize(Rectangle<float>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(46)->resize(Rectangle<float>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(47)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(48)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(49)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(50)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(51)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(52)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(53)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(54)->resize(Rectangle<float>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(55)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 4.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(56)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 3.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(57)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(58)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(59)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(60)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(61)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(62)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(63)->resize(Rectangle<float>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 3.5 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(64)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(65)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(66)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(67)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(68)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(69)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(70)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(71)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(72)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(73)->resize(Rectangle<float>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 4 * atomSize, atomSize, atomSize));
}