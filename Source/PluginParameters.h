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

class PluginParameterComboBox : public PluginParameter
{
public:
    PluginParameterComboBox(PluginParametersManager& parametersManager,
        const String& paramName,
        const StringArray items,
        const int defaultChoice = 0,
        const std::function<float(const float)> callback = nullptr)
        : PluginParameter(parametersManager, callback)
        , paramName(paramName)
        , items(items)
        , defaultChoice(defaultChoice)
    {
        paramID = paramName.removeCharacters(" ").toLowerCase();
        parametersManager.parameterTypes.add("ComboBox");

        parametersManager.comboBoxItemLists.add(items);
        NormalisableRange<float> range(0.0f, (float)items.size() - 1.0f, 1.0f);

        parametersManager.valueTreeState.createAndAddParameter(std::make_unique<Parameter>
            (paramID, paramName, "", range, (float)defaultChoice,
                [items](float value) { return items[(int)value]; },
                [items](const String& text) { return items.indexOf(text); })
        );

        parametersManager.valueTreeState.addParameterListener(paramID, this);
        updateValue((float)defaultChoice);
    }

    const String& paramName;
    const StringArray items;
    const int defaultChoice;
};

//==============================================================================

class DraggableLine : public LookAndFeel_V4
{
public:
    void drawLinearSlider(Graphics& g, int x, int y, int width, int height,
        float sliderPos,
        float minSliderPos,
        float maxSliderPos,
        const Slider::SliderStyle style, Slider& slider) override
    {
        if (slider.isBar())
        {
            g.setColour(slider.findColour(Slider::trackColourId));
            g.fillRect(slider.isHorizontal() ? Rectangle<float>(static_cast<float> (x), y + 0.5f, sliderPos - x, height - 1.0f)
                : Rectangle<float>(x + 0.5f, sliderPos, width - 1.0f, y + (height - sliderPos)));
        }
        else
        {
            auto isTwoVal = (style == Slider::SliderStyle::TwoValueVertical || style == Slider::SliderStyle::TwoValueHorizontal);
            auto isThreeVal = (style == Slider::SliderStyle::ThreeValueVertical || style == Slider::SliderStyle::ThreeValueHorizontal);

            auto trackWidth = jmin(6.0f, slider.isHorizontal() ? height * 0.25f : width * 0.25f);

            Point<float> startPoint(slider.isHorizontal() ? x : x + width * 0.5f,
                slider.isHorizontal() ? y + height * 0.5f : height + y);

            Point<float> endPoint(slider.isHorizontal() ? width + x : startPoint.x,
                slider.isHorizontal() ? startPoint.y : y);

            Path backgroundTrack;
            backgroundTrack.startNewSubPath(startPoint);
            backgroundTrack.lineTo(endPoint);
            g.setColour(slider.findColour(Slider::backgroundColourId));
            g.strokePath(backgroundTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

            Path valueTrack;
            Point<float> minPoint, maxPoint, thumbPoint;

            if (isTwoVal || isThreeVal)
            {
                minPoint = { slider.isHorizontal() ? minSliderPos : width * 0.5f,
                             slider.isHorizontal() ? height * 0.5f : minSliderPos };

                if (isThreeVal)
                    thumbPoint = { slider.isHorizontal() ? sliderPos : width * 0.5f,
                                   slider.isHorizontal() ? height * 0.5f : sliderPos };

                maxPoint = { slider.isHorizontal() ? maxSliderPos : width * 0.5f,
                             slider.isHorizontal() ? height * 0.5f : maxSliderPos };
            }
            else
            {
                auto kx = slider.isHorizontal() ? sliderPos : (x + width * 0.5f);
                auto ky = slider.isHorizontal() ? (y + height * 0.5f) : sliderPos;

                minPoint = startPoint;
                maxPoint = { kx, ky };
            }

            auto thumbWidth = getSliderThumbRadius(slider);

            valueTrack.startNewSubPath(minPoint);
            valueTrack.lineTo(isThreeVal ? thumbPoint : maxPoint);
            g.setColour(slider.findColour(Slider::trackColourId));
            g.strokePath(valueTrack, { trackWidth, PathStrokeType::curved, PathStrokeType::rounded });

            if (!isTwoVal)
            {
                g.setColour(slider.findColour(Slider::thumbColourId));
                g.fillEllipse(Rectangle<float>(static_cast<float> (thumbWidth), static_cast<float> (thumbWidth)).withCentre(isThreeVal ? thumbPoint : maxPoint));
            }

            if (isTwoVal || isThreeVal)
            {
                auto sr = jmin(trackWidth, (slider.isHorizontal() ? height : width) * 0.4f);
                auto pointerColour = slider.findColour(Slider::thumbColourId);

                if (slider.isHorizontal())
                {
                    drawPointer(g, minSliderPos - sr,
                        jmax(0.0f, y + height * 0.5f - trackWidth * 2.0f),
                        trackWidth * 2.0f, pointerColour, 2);

                    drawPointer(g, maxSliderPos - trackWidth,
                        jmin(y + height - trackWidth * 2.0f, y + height * 0.5f),
                        trackWidth * 2.0f, pointerColour, 4);
                }
                else
                {
                    drawPointer(g, jmax(0.0f, x + width * 0.5f - trackWidth * 2.0f),
                        minSliderPos - trackWidth,
                        trackWidth * 2.0f, pointerColour, 1);

                    drawPointer(g, jmin(x + width - trackWidth * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                        trackWidth * 2.0f, pointerColour, 3);
                }
            }
        }
    }

private:
};