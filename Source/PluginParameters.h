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

#include <JuceHeader.h>
using Parameter = AudioProcessorValueTreeState::Parameter;

//==============================================================================

class PluginParametersManager
{
public:
    PluginParametersManager(AudioProcessor& p) : valueTreeState(p, nullptr)
    {
    }

    AudioProcessorValueTreeState valueTreeState;
    StringArray parameterTypes;
    Array<StringArray> comboBoxItemLists;
};

//==============================================================================

class PluginParameter
    : public LinearSmoothedValue<float>
    , public AudioProcessorValueTreeState::Listener
{
protected:
    PluginParameter(PluginParametersManager& parametersManager,
        const std::function<float(float)> callback = nullptr)
        : parametersManager(parametersManager)
        , callback(callback)
    {
    }

public:
    void updateValue(float value)
    {
        if (callback != nullptr)
            setCurrentAndTargetValue(callback(value));
        else
            setCurrentAndTargetValue(value);
    }

    void parameterChanged(const String& parameterID, float newValue) override
    {
        updateValue(newValue);
    }

    PluginParametersManager& parametersManager;
    std::function<float(float)> callback;
    String paramID;
};

//==============================================================================

class PluginParameterSlider : public PluginParameter
{
protected:
    PluginParameterSlider(PluginParametersManager& parametersManager,
        const String& paramName,
        const String& labelText,
        const float minValue,
        const float maxValue,
        const float defaultValue,
        const std::function<float(float)> callback,
        const bool logarithmic)
        : PluginParameter(parametersManager, callback)
        , paramName(paramName)
        , labelText(labelText)
        , minValue(minValue)
        , maxValue(maxValue)
        , defaultValue(defaultValue)
    {
        paramID = paramName.removeCharacters(" ").toLowerCase();
        parametersManager.parameterTypes.add("Slider");

        NormalisableRange<float> range(minValue, maxValue);
        if (logarithmic)
            range.setSkewForCentre(sqrt(minValue * maxValue));

        parametersManager.valueTreeState.createAndAddParameter(std::make_unique<Parameter>
            (paramID, paramName, labelText, range, defaultValue,
                [](float value) { return String(value, 2); },
                [](const String& text) { return text.getFloatValue(); })
        );

        parametersManager.valueTreeState.addParameterListener(paramID, this);
        updateValue(defaultValue);
    }

public:
    const String& paramName;
    const String& labelText;
    const float minValue;
    const float maxValue;
    const float defaultValue;
};

//======================================

class PluginParameterLinSlider : public PluginParameterSlider
{
public:
    PluginParameterLinSlider(PluginParametersManager& parametersManager,
        const String& paramName,
        const String& labelText,
        const float minValue,
        const float maxValue,
        const float defaultValue,
        const std::function<float(float)> callback = nullptr)
        : PluginParameterSlider(parametersManager,
            paramName,
            labelText,
            minValue,
            maxValue,
            defaultValue,
            callback,
            false)
    {
    }
};

//======================================

class PluginParameterLogSlider : public PluginParameterSlider
{
public:
    PluginParameterLogSlider(PluginParametersManager& parametersManager,
        const String& paramName,
        const String& labelText,
        const float minValue,
        const float maxValue,
        const float defaultValue,
        const std::function<float(float)> callback = nullptr)
        : PluginParameterSlider(parametersManager,
            paramName,
            labelText,
            minValue,
            maxValue,
            defaultValue,
            callback,
            true)
    {
    }
};

//==============================================================================

class PluginParameterToggle : public PluginParameter
{
public:
    PluginParameterToggle(PluginParametersManager& parametersManager,
        const String& paramName,
        const bool defaultState = false,
        const std::function<float(float)> callback = nullptr)
        : PluginParameter(parametersManager, callback)
        , paramName(paramName)
        , defaultState(defaultState)
    {
        paramID = paramName.removeCharacters(" ").toLowerCase();
        parametersManager.parameterTypes.add("ToggleButton");

        const StringArray toggleStates = { "False", "True" };
        NormalisableRange<float> range(0.0f, 1.0f, 1.0f);

        parametersManager.valueTreeState.createAndAddParameter(std::make_unique<Parameter>
            (paramID, paramName, "", range, (float)defaultState,
                [toggleStates](float value) { return toggleStates[(int)value]; },
                [toggleStates](const String& text) { return toggleStates.indexOf(text); })
        );

        parametersManager.valueTreeState.addParameterListener(paramID, this);
        updateValue((float)defaultState);
    }

    const String& paramName;
    const bool defaultState;
};

//==============================================================================

class ThumbOnlySlider : public LookAndFeel_V4
{
public:
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider) override
    {
        // Visualise slider bounds
        //g.setColour(Colours::black);
        //g.drawRect(Rectangle<int>(x, y, width, height));

        auto thumbWidth = getSliderThumbRadius(slider) * 1.45f;
        g.setColour(slider.findColour(Slider::thumbColourId));
        auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
        auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;
        g.fillEllipse(Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth))
            .withCentre({ kx, ky }));
    }

private:
};