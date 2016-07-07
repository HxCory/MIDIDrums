#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent   : public Component,
                               private Button::Listener,
                               private Slider::Listener
{
public:
    //==============================================================================
    MainContentComponent()
       : midiChannel (10),
         startTime (Time::getMillisecondCounterHiRes() * 0.001)
    {
        setLookAndFeel (&lookAndFeel);
        
        addAndMakeVisible (bassDrumButton);
        bassDrumButton.setButtonText ("Bass Drum (36)");
        bassDrumButton.addListener (this);

        addAndMakeVisible (snareDrumButton);
        snareDrumButton.setButtonText ("Snare Drum (38)");
        snareDrumButton.addListener (this);

        addAndMakeVisible (closedHiHatButton);
        closedHiHatButton.setButtonText ("Closed HH (42)");
        closedHiHatButton.addListener (this);

        addAndMakeVisible (openHiHatButton);
        openHiHatButton.setButtonText ("Open HH (46)");
        openHiHatButton.addListener (this);

        addAndMakeVisible (volumeLabel);
        volumeLabel.setText ("Volume (CC7)", dontSendNotification);

        addAndMakeVisible (volumeSlider);
        volumeSlider.setRange (0, 127, 1);
        volumeSlider.addListener (this);

        addAndMakeVisible (midiMessagesBox);
        midiMessagesBox.setMultiLine (true);
        midiMessagesBox.setReturnKeyStartsNewLine (true);
        midiMessagesBox.setReadOnly (true);
        midiMessagesBox.setScrollbarsShown (true);
        midiMessagesBox.setCaretVisible (false);
        midiMessagesBox.setPopupMenuEnabled (true);
        midiMessagesBox.setColour (TextEditor::backgroundColourId, Colour (0x32ffffff));
        midiMessagesBox.setColour (TextEditor::outlineColourId, Colour (0x1c000000));
        midiMessagesBox.setColour (TextEditor::shadowColourId, Colour (0x16000000));

        setSize (800, 300);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::grey);
    }

    void resized() override
    {
        const int halfWidth = getWidth() / 2;

        Rectangle<int> buttonsBounds (getLocalBounds().withWidth (halfWidth).reduced (10));

        bassDrumButton.setBounds (buttonsBounds.getX(), 10, buttonsBounds.getWidth(), 20);
        snareDrumButton.setBounds (buttonsBounds.getX(), 40, buttonsBounds.getWidth(), 20);
        closedHiHatButton.setBounds (buttonsBounds.getX(), 70, buttonsBounds.getWidth(), 20);
        openHiHatButton.setBounds (buttonsBounds.getX(), 100, buttonsBounds.getWidth(), 20);
        volumeLabel.setBounds (buttonsBounds.getX(), 190, buttonsBounds.getWidth(), 20);
        volumeSlider.setBounds (buttonsBounds.getX(), 220, buttonsBounds.getWidth(), 20);

        midiMessagesBox.setBounds (getLocalBounds().withWidth (halfWidth).withX (halfWidth).reduced (10));
    }

private:
    static String getMidiMessageDescription (const MidiMessage& m)
    {
        if (m.isNoteOn())           return "Note on "  + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
        if (m.isNoteOff())          return "Note off " + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3);
        if (m.isProgramChange())    return "Program change " + String (m.getProgramChangeNumber());
        if (m.isPitchWheel())       return "Pitch wheel " + String (m.getPitchWheelValue());
        if (m.isAftertouch())       return "After touch " + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) +  ": " + String (m.getAfterTouchValue());
        if (m.isChannelPressure())  return "Channel pressure " + String (m.getChannelPressureValue());
        if (m.isAllNotesOff())      return "All notes off";
        if (m.isAllSoundOff())      return "All sound off";
        if (m.isMetaEvent())        return "Meta event";

        if (m.isController())
        {
            String name (MidiMessage::getControllerName (m.getControllerNumber()));

            if (name.isEmpty())
                name = "[" + String (m.getControllerNumber()) + "]";

            return "Controller " + name + ": " + String (m.getControllerValue());
        }

        return String::toHexString (m.getRawData(), m.getRawDataSize());
    }

    void buttonClicked (Button* button) override
    {
        int noteNumber = -1; // just used as a check that this as been set before we create a MidiMessage object

        if (button == &bassDrumButton)      noteNumber = 36;
        if (button == &snareDrumButton)     noteNumber = 38;
        if (button == &closedHiHatButton)   noteNumber = 42;
        if (button == &openHiHatButton)     noteNumber = 46;

        if (noteNumber >= 0)
        {
            MidiMessage message = MidiMessage::noteOn (midiChannel, noteNumber, (uint8) 100);
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
            addMessageToList (message);
        }
    }

    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &volumeSlider)
        {
            MidiMessage message = MidiMessage::controllerEvent (midiChannel, 7, (int) volumeSlider.getValue());
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
            addMessageToList (message);
        }
    }

    void logMessage (const String& m)
    {
        midiMessagesBox.moveCaretToEnd();
        midiMessagesBox.insertTextAtCaret (m + newLine);
    }

    void addMessageToList (const MidiMessage& message)
    {
        const double time = message.getTimeStamp();

        const int hours   = ((int) (time / 3600.0)) % 24;
        const int minutes = ((int) (time / 60.0)) % 60;
        const int seconds = ((int) time) % 60;
        const int millis  = ((int) (time * 1000.0)) % 1000;

        const String timecode (String::formatted ("%02d:%02d:%02d.%03d",
                                                  hours,
                                                  minutes,
                                                  seconds,
                                                  millis));

        logMessage (timecode + "  -  " + getMidiMessageDescription (message));
    }

    //==============================================================================
    TextButton bassDrumButton;
    TextButton snareDrumButton;
    TextButton closedHiHatButton;
    TextButton openHiHatButton;

    Label volumeLabel;
    Slider volumeSlider;

    TextEditor midiMessagesBox;

    int midiChannel;
    double startTime;

    LookAndFeel_V3 lookAndFeel;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED