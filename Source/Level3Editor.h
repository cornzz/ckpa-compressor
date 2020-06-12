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

class Level3Editor : public Component,
    Slider::Listener
{
public:
    Level3Editor(Ckpa_compressorAudioProcessor&);
    ~Level3Editor();

    void sliderValueChanged(Slider* slider) override;

    void paint(Graphics&) override;
    void resized() override;

private:
    Ckpa_compressorAudioProcessor& processor;

    ThumbOnlySlider tos;
    std::unique_ptr<Slider> compressionSlider;

    float circleDiameter = 0;

    enum {
        editorWidth = 500,
        editorMargin = 20,
        editorPadding = 10,
    };
};