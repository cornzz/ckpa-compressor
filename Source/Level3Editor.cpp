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

//==============================================================================

Level3Editor::Level3Editor(Ckpa_compressorAudioProcessor& p) : processor(p)
{
    compressionSlider = std::make_unique<Slider>(Slider::LinearHorizontal, Slider::NoTextBox);
    compressionSlider->setLookAndFeel(&tos);
    compressionSlider->setRange(1, 25, 0.1);
    compressionSlider->setValue(25);
    
    compressionSlider->addListener(this);

    addAndMakeVisible(*compressionSlider);
}

Level3Editor::~Level3Editor()
{
}

void Level3Editor::sliderValueChanged(Slider* slider)
{
    circleDiameter = (slider->getPositionOfValue(slider->getValue()) + 10) * 2.0f;
    repaint();
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
        .withTrimmedLeft(10)
        .removeFromLeft(r.getHeight() / 2);
    compressionSlider->setBounds(r);
}