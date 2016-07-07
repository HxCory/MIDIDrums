#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent   : public Component,
                               private Button::Listener,
                               private Slider::Listener,
                               private Timer
{
public:
    //==============================================================================
    MainContentComponent()
       : midiChannel (10),
         startTime (Time::getMillisecondCounterHiRes() * 0.001),
         sampleRate(44100.0),
         previousSampleNumber(0)
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

        addAndMakeVisible (crashCymbalButton);
        crashCymbalButton.setButtonText ("Crash (49)");
        crashCymbalButton.addListener (this);

        addAndMakeVisible (rideCymbalButton);
        rideCymbalButton.setButtonText ("Ride (51)");
        rideCymbalButton.addListener (this);

        addAndMakeVisible (volumeLabel);
        volumeLabel.setText ("Volume (CC7)", dontSendNotification);

        addAndMakeVisible (volumeSlider);
        volumeSlider.setRange (0, 127, 1);
        volumeSlider.addListener (this);

        addAndMakeVisible (panLabel);
        panLabel.setText ("Pan (CC10)", dontSendNotification);

        addAndMakeVisible (panSlider);
        panSlider.setRange (-10, 10 , 1);
        panSlider.addListener (this);

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

        setSize (800, 320);
        startTimer(1);
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
        crashCymbalButton.setBounds (buttonsBounds.getX(), 130, buttonsBounds.getWidth(), 20);
        rideCymbalButton.setBounds (buttonsBounds.getX(), 160, buttonsBounds.getWidth(), 20);
        volumeLabel.setBounds (buttonsBounds.getX(), 190, buttonsBounds.getWidth(), 20);
        volumeSlider.setBounds (buttonsBounds.getX(), 220, buttonsBounds.getWidth(), 20);
        panLabel.setBounds (buttonsBounds.getX(), 250, buttonsBounds.getWidth(), 20);
        panSlider.setBounds (buttonsBounds.getX(), 280, buttonsBounds.getWidth(), 20);

        midiMessagesBox.setBounds (getLocalBounds().withWidth (halfWidth).withX (halfWidth).reduced (10));
    }

private:
    static String getMidiMessageDescription (const MidiMessage& m)
    {
        if (m.isNoteOn())           return "Note on "  + MidiMessage::getMidiNoteName (m.getNoteNumber(), true, true, 3) 
                                                        + "" + String(m.getVelocity());
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

    void timerCallback() override
    {
        const double currentTime = Time::getMillisecondCounterHiRes() * 0.001 - startTime;
        const int currentSampleNumber = (int) (currentTime * sampleRate);

        MidiBuffer::Iterator iterator(midiBuffer);
        MidiMessage message;
        int sampleNumber;

        while(iterator.getNextEvent(message, sampleNumber))
        {
            if(sampleNumber > currentSampleNumber)
                break;

            message.setTimeStamp(sampleNumber/sampleRate);
            addMessageToList(message);
        }

        midiBuffer.clear(previousSampleNumber, currentSampleNumber - previousSampleNumber);
        previousSampleNumber = currentSampleNumber;
    }

    void buttonClicked (Button* button) override
    {
        int noteNumber = -1; // just used as a check that this as been set before we create a MidiMessage object

        if (button == &bassDrumButton)      noteNumber = 36;
        if (button == &snareDrumButton)     noteNumber = 38;
        if (button == &closedHiHatButton)   noteNumber = 42;
        if (button == &openHiHatButton)     noteNumber = 46;
        if (button == &crashCymbalButton)   noteNumber = 49;
        if (button == &rideCymbalButton)    noteNumber = 51;

        if (noteNumber >= 0)
        {
            MidiMessage message = MidiMessage::noteOn (midiChannel, noteNumber, (uint8) 100);
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
            addMessageToList (message);

            MidiMessage messageOff(MidiMessage::noteOff(message.getChannel(),
             message.getNoteNumber()));
            messageOff.setTimeStamp(message.getTimeStamp() + 0.1);
            addMessageToBuffer(messageOff);
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

        if (slider == &panSlider)
        {
            MidiMessage message = MidiMessage::controllerEvent(midiChannel, 10, (int) panSlider.getValue());
            message.setTimeStamp (Time::getMillisecondCounterHiRes() * 0.001 - startTime);
            addMessageToList (message);
        }
    }


    void addMessageToBuffer(const MidiMessage& message)
    {
        const double timestamp = message.getTimeStamp();
        const int sampleNumber = (int) (timestamp * sampleRate);
        midiBuffer.addEvent(message, sampleNumber);
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
    TextButton crashCymbalButton;
    TextButton rideCymbalButton;

    Label volumeLabel;
    Slider volumeSlider;
    Label panLabel;
    Slider panSlider;

    TextEditor midiMessagesBox;

    int midiChannel;
    double startTime;

    MidiBuffer midiBuffer;
    double sampleRate;
    int previousSampleNumber;

    LookAndFeel_V3 lookAndFeel;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
