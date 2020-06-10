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

//==============================================================================

class TabsLookAndFeel : public LookAndFeel_V4
{

};

class CustomTabBarButton : public TabBarButton
{
public:
    CustomTabBarButton(const String& name, TabbedButtonBar& bar) : TabBarButton(name, bar) 
    {
    }

    Rectangle<int> getActiveArea() const
    {
        auto r = getLocalBounds();
        auto leftRightMargin = 50;

        r.removeFromRight(leftRightMargin);
        r.removeFromLeft(leftRightMargin);

        return r;
    }
};

struct MainTabbedComponent : public TabbedComponent
{
public:
    MainTabbedComponent(Ckpa_compressorAudioProcessor& p, Component* c1, Component* c2, Component* c3) : 
        TabbedComponent(TabbedButtonBar::TabsAtBottom),
        processor(p)
    {
        auto colour = findColour(ResizableWindow::backgroundColourId);

        addTab("Level 1", colour, c1, true);
        addTab("Level 2", colour, c2, true);
        addTab("Level 3", colour, c3, true);
        setOutline(0.0f);
        setLookAndFeel(&tlaf);
    }

    ~MainTabbedComponent()
    {
        setLookAndFeel(nullptr);
    }

    void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override
    {
        processor.level1active = (newCurrentTabName == "Level 1") ? true : false;
    }

protected:
    CustomTabBarButton* createTabButton(const String& tabName, int /*tabIndex*/) override
    {
        return new CustomTabBarButton(tabName, *tabs);
    }

private:
    Ckpa_compressorAudioProcessor& processor;
    
    TabsLookAndFeel tlaf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainTabbedComponent)
};
