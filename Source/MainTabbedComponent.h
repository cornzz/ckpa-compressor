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
    int getTabButtonOverlap(int tabDepth)
    {
        return -7; // Margin between buttons
    }

    int getTabButtonSpaceAroundImage()
    {
        return 7;
    }

    int getTabButtonBestWidth(TabBarButton& button, int tabDepth)
    {
        return 80;
    }

    void drawTabButton(TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
    {
        const Rectangle<int> activeArea(button.getActiveArea());

        DropShadow(Colours::black.withAlpha(0.6f), 4, Point<int>(0, 0)).drawForRectangle(g, activeArea);

        const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

        const Colour bkg(button.getTabBackgroundColour());

        if (button.getToggleState())
        {
            g.setColour(bkg);
        }
        else
        {
            g.setColour(findColour(TabbedComponent::backgroundColourId));
        }

        g.fillRect(activeArea);

        g.setColour(button.findColour(Slider::thumbColourId));

        Rectangle<int> r(activeArea);

        g.fillRect(r.removeFromTop(1));
        g.fillRect(r.removeFromBottom(1));
        g.fillRect(r.removeFromLeft(1));
        g.fillRect(r.removeFromRight(1));

        const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;

        Colour col(bkg.contrasting().withMultipliedAlpha(alpha));

        if (TabbedButtonBar* bar = button.findParentComponentOfClass<TabbedButtonBar>())
        {
            TabbedButtonBar::ColourIds colID = button.isFrontTab() ? TabbedButtonBar::frontTextColourId
                : TabbedButtonBar::tabTextColourId;

            if (bar->isColourSpecified(colID))
                col = bar->findColour(colID);
            else if (isColourSpecified(colID))
                col = findColour(colID);
        }

        const Rectangle<float> area(button.getTextArea().toFloat());

        float length = area.getWidth();
        float depth = area.getHeight();

        TextLayout textLayout;
        createTabTextLayout(button, length, depth, col, textLayout);

        AffineTransform t;
        t = t.translated(area.getX(), area.getY());
        g.addTransform(t);
        textLayout.draw(g, Rectangle<float>(length, depth));
    }

    void drawTabAreaBehindFrontButton(TabbedButtonBar& bar, Graphics& g, const int w, const int h)
    {
        // Necessary to prevent separation line and shadow from drawing
    }
};

struct MainTabbedComponent : public TabbedComponent
{
public:
    MainTabbedComponent(Ckpa_compressorAudioProcessor& p, Component* c1, Component* c2, Component* c3) : 
        TabbedComponent(TabbedButtonBar::TabsAtBottom),
        processor(p)
    {
        auto colour = findColour(ResizableWindow::backgroundColourId).darker(0.1);

        addTab("Level 1", colour, c1, true);
        addTab("Level 2", colour, c2, true);
        addTab("Level 3", colour, c3, true);

        setOutline(0.0f);
        tlaf.setColour(TabbedComponent::backgroundColourId, Colour(0xFF222F36));
        setLookAndFeel(&tlaf);
        setTabBarDepth(tabBarDepth);
    }

    ~MainTabbedComponent()
    {
        setLookAndFeel(nullptr);
    }

    void paint(Graphics& g)
    {
        g.fillAll(findColour(TabbedComponent::backgroundColourId));
    }

    void resized()
    {
        auto content = getLocalBounds();

        tabs->setBounds(content.removeFromBottom(tabBarDepth));
        content = content.withTrimmedBottom(tabBarDepth2);

        for (int i = 0; i < getNumTabs(); ++i) {
            auto* c = getTabContentComponent(i);
            if (auto comp = c)
                comp->setBounds(content);
        }
    }

    void currentTabChanged(int newCurrentTabIndex, const String& newCurrentTabName) override
    {
        processor.level1active = (newCurrentTabName == "Level 1") ? true : false;
    }

private:
    Ckpa_compressorAudioProcessor& processor;

    // Actual depth is tabBarDepth (bottom part) + tabBarDepth2 (top part)
    int tabBarDepth = 33;
    int tabBarDepth2 = 7;
    
    TabsLookAndFeel tlaf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainTabbedComponent)
};
