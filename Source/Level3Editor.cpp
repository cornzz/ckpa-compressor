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

#include "Level3Editor.h"

//==============================================================================

Level3Editor::Level3Editor(Ckpa_compressorAudioProcessor& p, Component* parentForPopup) : processor(p),
    popupParent(parentForPopup)
{
    ScopedValueSetter<bool> svs(init, false);
    
    showDragMe = true;

    rand = std::make_unique<Random>();
    Colour c = findColour(Slider::thumbColourId);
    for (int i = 0; i < numAtoms; ++i) {
        Atom* a;
        atoms.add(a = new Atom(rand.get(), c));
        addChildComponent(a, 0);
    }
    invisibleAtoms.resize(numAtoms);
    std::iota(invisibleAtoms.begin(), invisibleAtoms.end(), 0);

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

    compressionSlider->addListener(this);
    sliderAttachments.add(new SliderAttachment(processor.parameters.valueTreeState, "compression", *compressionSlider));
    addAndMakeVisible(*compressionSlider);

    startTimerHz(30);
}

Level3Editor::~Level3Editor()
{
    compressionSlider->setLookAndFeel(nullptr);
    stopTimer();
}

void Level3Editor::sliderValueChanged(Slider* slider)
{
    if (!init)
        return;

    double compressionValue;
    bool circleChanged = false;

    if (slider == compressionSlider) { // Compression circle changed
        compressionValue = slider->getValue();

        sliders.getUnchecked(0)->setValue(compressionValue * 1.5 - 30);              // Threshold
        sliders.getUnchecked(1)->setValue(pow(compressionValue - 20, 2) / 25 + 1.0); // Ratio
        sliders.getUnchecked(2)->setValue(-0.2 * compressionValue + 4);              // Makeup gain
        circleChanged = true;
    }
    else if (!dragging) { // Threshold / ratio / makeup gain changed
        double tempThresh = sliders.getUnchecked(0)->getValue();
        double tempRatio = sliders.getUnchecked(1)->getValue();
        double tempMakeup = sliders.getUnchecked(2)->getValue();

        tempThresh = jlimit(0.0, 20.0, (tempThresh + 30) / 1.5);
        tempRatio = jlimit(0.0, 20.0, 20 - sqrt((tempRatio - 1) * 25));
        tempMakeup = jlimit(0.0, 20.0, (tempMakeup - 4) / -0.2);
        compressionValue = (tempThresh == 20.0 || tempRatio == 20.0) ? 20.0 : (tempThresh + tempRatio + tempMakeup) / 3;

        sliders.getLast()->setValue(compressionValue, dontSendNotification);
        circleChanged = true;
    }

    if (circleChanged) {
        circleDiameter = (compressionSlider->getPositionOfValue(compressionValue) + 30) * 2.0f;
        repaint();
        resizeAtoms();
    }
}

void Level3Editor::sliderDragStarted(Slider* slider)
{
    if (slider == compressionSlider)
        dragging = true;
}

void Level3Editor::sliderDragEnded(Slider* slider)
{
    if (slider == compressionSlider)
        dragging = false;
}

void Level3Editor::timerCallback()
{
    // Change amount of visible atoms according to current input level
    float rms = processor.meterSourceInput.getRMSLevel(0);
    float rmsDb = juce::Decibels::gainToDecibels(rms, -60.0f) + 3;
    int visibleTarget = ceilf(jmin(1.0f, (1 - rmsDb / -70.0f)) * numAtoms);
    std::random_device rand;
    std::mt19937 g(rand());
    std::shuffle(invisibleAtoms.begin(), invisibleAtoms.end(), g);
    std::shuffle(visibleAtoms.begin(), visibleAtoms.end(), g);

    // fadeIn() / fadeOut() while changing circle size causes atoms to go and stay out of bounds.
    // To prevent that, fadeIn/out is only used when circle is not being resized. 
    // TODO: Still happens on very rapid circle size change.
    while (visibleAtoms.size() < visibleTarget) { // Make more Atoms visible to reach target
        int a = invisibleAtoms.back();
        invisibleAtoms.pop_back();
        Atom* atom = atoms.getUnchecked(a);
        if (!dragging)
            atom->anim->fadeIn(atom, 75);
        else
            atom->setVisible(true);
        visibleAtoms.push_back(a);
    }
    while (visibleAtoms.size() > visibleTarget) { // Make less Atoms visible to reach target
        int a = visibleAtoms.back();
        visibleAtoms.pop_back();
        Atom* atom = atoms.getUnchecked(a);
        if (!dragging)
            atom->anim->fadeOut(atom, 75);
        else
            atom->setVisible(false);
        invisibleAtoms.push_back(a);
    }
}

void Level3Editor::paint(Graphics& g)
{
    Colour bkg = getLookAndFeel().findColour(ResizableWindow::backgroundColourId);
    g.fillAll(bkg);

    Rectangle<float> r = getLocalBounds().toFloat().reduced(editorMargin);

    auto rect = Rectangle<float>(circleDiameter, circleDiameter).withCentre(r.getCentre());
    g.setColour(findColour(Slider::thumbColourId));
    g.drawEllipse(rect, 2.0f);

    if (showDragMe) {
        processor.showBubbleMessage(compressionSlider, popupParent, true, 1200);
        showDragMe = false;
    }
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
    // Set circle size, atom bounds, trigger repaint and start atom animation
    sliderValueChanged(compressionSlider);
}

//==============================================================================

Atom::Atom(Random* r, Colour c) : rand(r)
{
    ae = std::make_unique<AtomEllipse>(c);
    addAndMakeVisible(*ae);
    
    anim = std::make_unique<ComponentAnimator>();
    // As soon as animator is done animating atom, an animation to new coordinates should be started
    // See changeListenerCallback()
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
        ae->setBounds(getLocalBounds().withSizeKeepingCentre(4, 4));
        anim->sendChangeMessage(); // Start animation loop
        init = true;
    }
}

void Atom::resize(Rectangle<int> newBounds)
{
    if (!init) {
        setBounds(newBounds);
        return;
    }

    Rectangle<int> oldBounds = getBounds();
    float oldWidth = static_cast<float>(oldBounds.getWidth());
    float oldHeight = static_cast<float>(oldBounds.getHeight());

    // Get relative position of current destination in old bounds and cancel animation
    Rectangle<int> destination = anim->getComponentDestination(ae.get());
    float destRelativeX = destination.getX() / oldWidth;
    float destRelativeY = destination.getY() / oldHeight;
    anim->cancelAnimation(ae.get(), false);

    // Set position of ae in new bounds relative to position in old bounds
    float relativeX = ae->getX() / oldWidth;
    float relativeY = ae->getY() / oldHeight;
    setBounds(newBounds);
    float newWidth = static_cast<float>(newBounds.getWidth());
    float newHeight = static_cast<float>(newBounds.getHeight());
    ae->setTopLeftPosition(newWidth * relativeX, newHeight * relativeY);

    // Set relative position of current destination in new bounds
    destination.setPosition(newWidth * destRelativeX, newHeight * destRelativeY);
    int dist = hypot(destination.getX() - ae->getX(), destination.getY() - ae->getY());
    anim->animateComponent(ae.get(), destination, 1.0f, dist * 30, false, 1, 1);
}

void Atom::changeListenerCallback(ChangeBroadcaster* source)
{
    ComponentAnimator* anim = dynamic_cast<ComponentAnimator*>(source);

    if (!anim->isAnimating(ae.get())) { // AtomEllipse reached destination, find new dest. coordinates
        int newX, newY, dist;
        while (true) {
            newX = rand->nextInt(getWidth() - ae->getWidth());
            newY = rand->nextInt(getHeight() - ae->getHeight());
            dist = hypot(newX - ae->getX(), newY - ae->getY());
            if (dist >= (getWidth() - ae->getWidth()) / 2 || ae->getWidth() >= getWidth() / 2)
                break; // Distance to new dest. is satisfactory
        }
        Rectangle<int> newBounds = ae->getBounds().withPosition(newX, newY);
        anim->animateComponent(ae.get(), newBounds, 1.0f, dist * 30, false, 1, 1);
    }
}

//==============================================================================

void Level3Editor::resizeAtoms()
{
    auto r = Rectangle<float>(circleDiameter, circleDiameter).withCentre(getLocalBounds().toFloat().getCentre());;
    float atomSize = circleDiameter * 0.1;

    atoms.getUnchecked(0)->resize(Rectangle<int>(r.getCentreX() - 4.8 * atomSize, r.getY() + r.getHeight() / 2 - atomSize - 0.8 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(1)->resize(Rectangle<int>(r.getCentreX() - 5 * atomSize,   r.getY() + r.getHeight() / 2 - atomSize,                  atomSize, atomSize));
    atoms.getUnchecked(2)->resize(Rectangle<int>(r.getCentreX() - 5 * atomSize,   r.getY() + r.getHeight() / 2,                             atomSize, atomSize));
    atoms.getUnchecked(3)->resize(Rectangle<int>(r.getCentreX() - 4.8 * atomSize, r.getY() + r.getHeight() / 2 + atomSize - 0.2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(4)->resize(Rectangle<int>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(5)->resize(Rectangle<int>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(6)->resize(Rectangle<int>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(7)->resize(Rectangle<int>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(8)->resize(Rectangle<int>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(9)->resize(Rectangle<int>(r.getCentreX() - 4 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(10)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(11)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(12)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(13)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(14)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(15)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(16)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(17)->resize(Rectangle<int>(r.getCentreX() - 3 * atomSize, r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(18)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 4.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(19)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 3.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(20)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(21)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(22)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 - 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(23)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(24)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(25)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(26)->resize(Rectangle<int>(r.getCentreX() - 2 * atomSize, r.getY() + r.getHeight() / 2 + 3.5 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(27)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(28)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(29)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(30)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(31)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(32)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(33)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(34)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(35)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(36)->resize(Rectangle<int>(r.getCentreX() - 1 * atomSize, r.getY() + r.getHeight() / 2 + 4 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(37)->resize(Rectangle<int>(r.getCentreX() + 3.8 * atomSize, r.getY() + r.getHeight() / 2 - atomSize - 0.8 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(38)->resize(Rectangle<int>(r.getCentreX() + 4 * atomSize,   r.getY() + r.getHeight() / 2 - atomSize,                  atomSize, atomSize));
    atoms.getUnchecked(39)->resize(Rectangle<int>(r.getCentreX() + 4 * atomSize,   r.getY() + r.getHeight() / 2,                             atomSize, atomSize));
    atoms.getUnchecked(40)->resize(Rectangle<int>(r.getCentreX() + 3.8 * atomSize, r.getY() + r.getHeight() / 2 + atomSize - 0.2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(41)->resize(Rectangle<int>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(42)->resize(Rectangle<int>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(43)->resize(Rectangle<int>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(44)->resize(Rectangle<int>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(45)->resize(Rectangle<int>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(46)->resize(Rectangle<int>(r.getCentreX() + 3 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(47)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(48)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(49)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(50)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(51)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(52)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(53)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(54)->resize(Rectangle<int>(r.getCentreX() + 2 * atomSize, r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(55)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 4.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(56)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 3.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(57)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(58)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(59)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 - 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(60)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 0.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(61)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 1.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(62)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 2.5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(63)->resize(Rectangle<int>(r.getCentreX() + 1 * atomSize, r.getY() + r.getHeight() / 2 + 3.5 * atomSize, atomSize, atomSize));

    atoms.getUnchecked(64)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 5 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(65)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 4 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(66)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(67)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(68)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 - 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(69)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 0 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(70)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 1 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(71)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 2 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(72)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 3 * atomSize, atomSize, atomSize));
    atoms.getUnchecked(73)->resize(Rectangle<int>(r.getCentreX(), r.getY() + r.getHeight() / 2 + 4 * atomSize, atomSize, atomSize));
}