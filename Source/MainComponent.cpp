/*
 ==============================================================================
 
 This file was auto-generated!
 
 ==============================================================================
 */

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <array>
#include <vector>
#include <algorithm>

const int maxNoGrains = 2;
const int channelShift = 0;

//==========================================================================
class ReferenceCountedBuffer  : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<ReferenceCountedBuffer> Ptr;
    
    ReferenceCountedBuffer(const String& nameToUse,
                           int numChannels,
                           int numSamples)
    
    :   position    (0),
    name        (nameToUse),
    buffer      (numChannels, numSamples)
    {
        DBG (String ("Buffer named '") + name + "' constructed. numChannels = " + String (numChannels) + ", numSamples = " + String (numSamples));
    }
    
    ~ReferenceCountedBuffer()
    {
        DBG (String ("Buffer named '") + name + "' destroyed");
    }
    
    AudioSampleBuffer* getAudioSampleBuffer()
    {
        return &buffer;
    }
    
    //Variables
    int position;
    
private:
    String name;
    AudioSampleBuffer buffer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedBuffer)
};

//==============================================================================
class Grain
{
public:
    
    //Sample location, envelope, granulatorPosition, grainSpeed, grainLengthSamples, id number, No Grains
    Grain(  ReferenceCountedBuffer::Ptr&                _s,
          std::array < std::array<double, 44100>, 3 >&  _e,
          std::array < bool, maxNoGrains >&             _cS,
          float&  _pos,
          float&  _grainSp,
          float&  _granSp,
          int&    _l,
          int&    _id,
          int&    _eI)
    
    :
    grainLength         (_l),
    grainSpeed          (_grainSp),
    granulatorSpeed     (_granSp),
    envelopesIt         (_eI),
    granulatorPosition  (_pos),
    envelopes           (_e),
    grainSample         (_s),
    sample_count        (0),
    channel             (0),
    channelStatus       (_cS),
    grainStartPosition  (0),
    ID                  (_id)
    
    
    {
        
        noGrains++;
        
        std::cout << "Grain " <<ID<< ": Constructor Started" << std::endl;
        std::cout << "Grain: Sample Location is " << grainSample << std::endl;
        interpolation = (float(ID)/float(noGrains)) * grainLength;
        std::cout << "Grain " <<ID<< ": Starting Interpolation is " << interpolation << std::endl;
        std::cout << "Grain " <<ID<< ": Constructor Finished" << std::endl;
    }
    
    ~Grain()
    {
        noGrains--;
        
        channelStatus[channel] = false;
    }
    
    void update(const int& numInputChannels, const int& numOutputChannels, AudioSampleBuffer& tempBuffer)
    {
        //Taking a local copy of the location of the current buffer in the function for when we change "current buffer" but still need audio
        ReferenceCountedBuffer::Ptr retainedCurrentBuffer (grainSample);
        
        //Output silence if there is no file.
        if (retainedCurrentBuffer == nullptr)
        {
            return;
        }
        
        //Take a local copy of a pointer to the actual sample buffer.
        AudioSampleBuffer* currentAudioSampleBuffer (retainedCurrentBuffer->getAudioSampleBuffer());
        
        //Make a writable variable to store the samples amplitude value;
        double wave;
        
        channel %= numOutputChannels;
        
        for(int i = 0; i < tempBuffer.getNumSamples(); i++)
        {
            
            //If we have interpolated through the grain once
            if(round(interpolation) >= grainLength)
            {
                //we are ready to interpolate through it for the next channel.
                interpolation = fmod(interpolation, grainLength);
                grainStartPosition = granulatorPosition + round(float (sample_count) * fabs(granulatorSpeed));
                
                //The current channel we are on is made free
                channelStatus[channel] = false;
                //For each channel status
                
                int randomChannel = random.nextInt(channelStatus.size());
                
                if (channelStatus[randomChannel] == true)
                {
                    while (channelStatus[randomChannel] == true)
                    {
                        randomChannel = random.nextInt(channelStatus.size());
                    }
                }
                
                
                channel = randomChannel;
                channelStatus[channel] = true;
                std::cout << channel << std::endl;
            }
            
            //Set wave to the sample value of the sound file at the granulators position plus our grain's interpolation for this channel.
            int bufferIt = fmod((grainStartPosition + interpolation), currentAudioSampleBuffer->getNumSamples());
            
            //Multiply the wave with parralell element of the envelope.
            int envelopeIt = (interpolation/float(grainLength)) * envelopes[0].size();
            
            wave = currentAudioSampleBuffer->getSample(0, bufferIt);
            
            wave *= envelopes[envelopesIt][envelopeIt];
            
            tempBuffer.setSample(channel, i, wave);
            
            for(int otherChannel = 0; otherChannel < numOutputChannels; otherChannel++)
                if(otherChannel != channel)
                    tempBuffer.setSample(otherChannel, i, 0.0);
            
            //if(channel == 0){}
            //std::cout << channel << " " << grainStartPosition << " " << granulatorPosition + round(float (sample_count) * granulatorSpeed) << " " << interpolation << " " << bufferIt << " " << grainLength << " " << granulatorSpeed << std::endl;
            
            //Increment the interpolation through the grain.
            interpolation += grainSpeed;
            
            //Counting the number of samples we are through the grain.
            sample_count ++;
            sample_count%=tempBuffer.getNumSamples();
        }
        
    }
    
    void setGrainSpeed  (const float& _s){  grainSpeed  = _s;}
    
    void setGrainLength (const int& _i)
    {
        grainLength = _i;
        interpolation = (float(ID)/float(noGrains)) * grainLength;
    }
    
    void noGrainsChanged(const int & _id)
    {
        grainStartPosition = granulatorPosition + round(float (sample_count) * fabs(granulatorSpeed));
        ID = _id;
        interpolation = round(float(ID)/float(noGrains) * grainLength);
        
        std::cout << "Grain: " << ID << " Interpolation: " << interpolation << std::endl;
    }
    
    //Getters
    static int getNoGrains();
    
private:
    int&    grainLength;
    float&  grainSpeed;
    float&  granulatorSpeed;
    float&  granulatorPosition;
    int&    envelopesIt;
    
    static int noGrains;
    
    std::array < std::array<double, 44100>, 3 >&    envelopes;
    ReferenceCountedBuffer::Ptr&                    grainSample;
    std::array < bool, maxNoGrains >&               channelStatus;
    
    Random random;
    
    //This stored the distance through the grain in samples.
    float   interpolation;
    //This counts the number of samples through this buffer.
    int     sample_count;
    //This will hold the begining position in the sample of this grain
    int     grainStartPosition;
    //This will be the output channel of the grain.
    int     channel;
    //This is the iteration of the grain in the granultors array of grains.
    int     ID;
    
    //JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReferenceCountedBuffer)
};

int Grain::noGrains = 0;
int Grain::getNoGrains()
{
    return noGrains;
}

//==============================================================================
class Granulator
{
public:
    
    //Constructor
    Granulator(ReferenceCountedBuffer::Ptr& _s)
    
    //Init Variables
    :   granulatorSample    (_s),
    granulatorPosition  (0),
    bufferPosition      (0),
    granulatorSpeed     (1),
    grainLength         (5512),
    gain                (0.5),
    grainSpeed          (1)
    {
        
        std::cout << "Granulator: Constructor Started" << std::endl;
        
        std::cout << "Granulator: Sample Location is " << granulatorSample << std::endl;
        
        //Initially fill the envelope will a hanning window.
        double pi = 3.1415926535897;
        for (int i = 0; i<envelopes[0].size(); ++i)
        {
            envelopes[0][i] = (std::sin( float(i)/44100 * (pi) ) );
            envelopes[1][i] = ( (1.0 - (float(i)/envelopes[0].size()) * 1.0f) );
            envelopes[2][i] = 1.0f - (round( (float(i)/envelopes[0].size()) * 1.0f));
        }
        
        std::cout << "Granulator: Hanning Envelope Completed " << envelopes[0][1] << ", " << envelopes[0][22051] << std::endl;
        std::cout << "Granulator: Phasor Envelope Completed " << envelopes[1][1] << ", " << envelopes[1][22051] << std::endl;
        std::cout << "Granulator: Pulse Envelope Completed " << envelopes[2][1] << ", " << envelopes[2][22051] << std::endl;
        
        //Taken = true; untaken = false;
        for(auto channel : channelStatus)
        {
            channel = false;
        }
        
        for(int i = 0; i < round(maxNoGrains/2); ++i)
            grains[i] = ( new Grain(    granulatorSample,
                                    envelopes,
                                    channelStatus,
                                    granulatorPosition,
                                    grainSpeed,
                                    granulatorSpeed,
                                    grainLength,
                                    i,
                                    envelopesIt));
        
        std::cout << "Granulator: No Grains is " << noGrains << std::endl;
        std::cout << "Granulator: Grain Length is " << grainLength << std::endl;
        std::cout << "Granulator: Constructor Finished" << std::endl;
        
    }
    
    ~Granulator()
    {
        for(auto grain : grains)
            delete grain;
    }
    
    //Member Functions:
    void update(const AudioSourceChannelInfo& bufferToFill)
    {
        
        //Taking a local copy of the location of the current buffer incase "current buffer" changes.
        ReferenceCountedBuffer::Ptr retainedCurrentBuffer (granulatorSample);
        
        //Output silence if there is no file.
        if (retainedCurrentBuffer == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        
        //Take a local copy of a pointer to the actual sample buffer.
        AudioSampleBuffer* currentAudioSampleBuffer (retainedCurrentBuffer->getAudioSampleBuffer());
        //granulatorPosition = retainedCurrentBuffer->position;
        
        //Establish channels
        const int numInputChannels = currentAudioSampleBuffer->getNumChannels();
        const int numOutputChannels = bufferToFill.buffer->getNumChannels() - channelShift;
        
        //We will use this variable to deduce how much more data before the block loops. At the begining of the Block it will be equal to the size of the block.
        int outputSamplesRemaining = bufferToFill.numSamples;
        
        //This variable will be used to calculate how much we will need to offset the start of the sample to ensure a smooth loop.
        int outputSamplesOffset = 0;
        
        while (outputSamplesRemaining > 0)
        {
            //Calculating how many more samples in the file we have to go before completion.
            int bufferSamplesRemaining = currentAudioSampleBuffer->getNumSamples() - round(granulatorPosition);
            
            //Finding out if the number of bufferSamples remaining is smaller than the number of block samples remaining and assigning the difference to a variable
            int samplesThisTime = jmin (outputSamplesRemaining, bufferSamplesRemaining);
            
            AudioSampleBuffer grainsBuffer(numOutputChannels, samplesThisTime);
            grainsBuffer.clear();
            
            for(int i = 0; i < Grain::getNoGrains(); ++i)
            {
                //Create a temporary audio buffer to be filled by the grains.
                AudioSampleBuffer temp(maxNoGrains, samplesThisTime);
                //Send the number of input channels, number of output channels and the location of the temporary audio buffer to the grains
                //One at a time the grains fill the buffer
                grains[i]->update(numInputChannels, numOutputChannels, temp);
                
                //Then we add buffer that the grain has filled to our GrainBuffer
                for(int channel = 0; channel<numOutputChannels; ++channel)
                {
                    grainsBuffer.addFrom(   channel,
                                         0,
                                         temp,
                                         channel,
                                         0,
                                         samplesThisTime);
                }
                temp.clear();
            }
            
            
            for(int channel = 0; channel < numOutputChannels; ++channel)
            {
                
//                bufferToFill.buffer->copyFrom(<#int destChannel#>,
//                                              <#int destStartSample#>,
//                                              <#const juce::AudioBuffer<float> &source#>,
//                                              <#int sourceChannel#>,
//                                              <#int sourceStartSample#>,
//                                              <#int numSamples#>)
                
                bufferToFill.buffer->copyFrom(  channel + channelShift,
                                              bufferToFill.startSample + outputSamplesOffset,
                                              grainsBuffer,
                                              channel,
                                              bufferPosition,
                                              samplesThisTime);
                
                //Apply the gain variable to the buffer.
                bufferToFill.buffer->applyGain(channel + channelShift, bufferToFill.startSample + outputSamplesOffset, samplesThisTime, gain);
                
                std::cout << channel << ": " << bufferToFill.buffer->getRMSLevel(channel + channelShift, bufferToFill.startSample, bufferToFill.numSamples) << std::endl;
            }
            
            //Minus the number of samples copied from the number of samples remaining for the block
            outputSamplesRemaining -= samplesThisTime;
            //Plus the number of samples copied so far to the offset value.
            outputSamplesOffset += samplesThisTime;
            //Add on the number of samples we have traveled into the sample.
            granulatorPosition += float(samplesThisTime) * granulatorSpeed;
            //Add of the number of samples we have traveled into the output buffer
            bufferPosition += samplesThisTime;
            //When the sample position is equal to the last sample in the audiofile, we loop the audio.
            if(granulatorPosition > currentAudioSampleBuffer->getNumSamples()-1)
            {
                granulatorPosition=0;
            }
            if(granulatorPosition < 0)
            {
                granulatorPosition=currentAudioSampleBuffer->getNumSamples()-1;
            }
            //When the buffer position goes out side the buffer have it start again.
            bufferPosition%= grainsBuffer.getNumSamples();
        }
        //Set the buffers position to the changed position
        retainedCurrentBuffer->position = granulatorPosition;
        
    }
    
    void setSpeed       (const float& _s)           { granulatorSpeed = _s; }
    void setEnvelopesIt (const int& _i)             { envelopesIt = _i; }
    void setGranulatorPosition(const float& _grP)   { granulatorPosition = _grP; }
    void setGain (const float& _dB)   
    {
        //10^(x/20)
        gain = pow(10, (_dB/20)); 
    }
    void setGrainSpeed(const float& _s)
    {
        for(int i = 0; i < Grain::getNoGrains(); ++i)
            grains[i]->setGrainSpeed(_s);
    }
    void setGrainLength(const int& _i)
    {
        for(int i = 0; i < Grain::getNoGrains(); ++i)
            grains[i]->setGrainLength(_i);
    }
    void setNoGrains(const int& newNoGrains)
    {
        int mNoGrains = Grain::getNoGrains();
        
        int noGrainsAction = (newNoGrains - mNoGrains);
        if(noGrainsAction > 0)
        {
            for(int i = mNoGrains; i < mNoGrains + noGrainsAction; ++i)
            {
                std::cout << "I: " << i << std::endl;
                grains[i] = ( new Grain(    granulatorSample,
                                        envelopes,
                                        channelStatus,
                                        granulatorPosition,
                                        grainSpeed,
                                        granulatorSpeed,
                                        grainLength,
                                        i,
                                        envelopesIt));
            }
        }
        else if(noGrainsAction < 0)
        {
            for(int i = mNoGrains; --i >= (mNoGrains + noGrainsAction);)
            {
                delete grains[i];
                std::cout << "Grain " << i << " deleted" << std::endl;
            }
        }
        
        std::cout << "New Number Of Grains: " << Grain::getNoGrains() << std::endl;
        
        for(int i = 0; i < Grain::getNoGrains(); ++i)
        {
            grains[i]->noGrainsChanged(i);
        }
    }
    void setPulseWidth(const float& _s)
    {
        for(int i = 0; i<44100; ++i)
        {
            if(i < 44100*_s)
            {
                envelopes[2][i] = 1;
            }
            else
            {
                envelopes[2][i] = 0;
            }
        }
    }
    
    
private:
    
    //Member Variables:
    
    //Vector of Grains
    std::array<Grain*, maxNoGrains>                 grains;
    //A Refference to the current sample we are reading.
    ReferenceCountedBuffer::Ptr&                    granulatorSample;
    //The grains envelope that will be used to let the grains over lap.
    std::array < std::array<double, 44100>, 3 >     envelopes;
    //Array of used channels
    std::array < bool, maxNoGrains>                 channelStatus;
    
    //The holistic position of the granulator though the sample.
    float   granulatorPosition;
    //The position in the output buffer.
    int     bufferPosition;
    //Speed that we iterate through sample.
    float   granulatorSpeed;
    //The length in samples of each grain.
    int     grainLength;
    //The final output volume max.
    float   gain;
    //Number of grains used by the granulator.
    int     noGrains;
    //Speed of the grains
    float   grainSpeed;
    //Envelopes Iterator will determin which envelope is used.
    int     envelopesIt;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Granulator)
};


class MainContentComponent :    public AudioAppComponent,
                                public Button::Listener,
                                public Slider::Listener,
                                private Thread
{
    
public:
    
    //==============================================================================
    MainContentComponent()
    :   Thread("Background Thread"),
    granulator(currentBuffer)
    {
        
        setSize (460, 460);
        
        granulatorSpeedSlider.setSliderStyle(Slider::LinearBarVertical);
        granulatorSpeedSlider.setRange (-10.0, 10.0, 0.01);
        granulatorSpeedSlider.setValue (1.0, dontSendNotification);  // [8
        granulatorSpeedSlider.addListener (this);
        addAndMakeVisible (granulatorSpeedSlider);
        
        granulatorSpeedLabel.setText ("Playback Speed", dontSendNotification);
        addAndMakeVisible (granulatorSpeedLabel);
        
        grainSpeedSlider.setSliderStyle(Slider::LinearBarVertical);
        grainSpeedSlider.setRange (0.0, 10.0, 0.01);
        grainSpeedSlider.setValue (1.0, dontSendNotification);  // [8
        grainSpeedSlider.addListener (this);
        addAndMakeVisible (grainSpeedSlider);
        
        grainSpeedLabel.setText ("Grain Speed", dontSendNotification);
        addAndMakeVisible (grainSpeedLabel);
        
        grainLengthSlider.setSliderStyle(Slider::LinearBarVertical);
        grainLengthSlider.setRange (1, 176400, 1);
        grainLengthSlider.setValue (44100, dontSendNotification);  // [8
        grainLengthSlider.addListener (this);
        addAndMakeVisible (grainLengthSlider);
        
        grainLengthLabel.setText ("Grain Length", dontSendNotification);
        addAndMakeVisible (grainLengthLabel);
        
        noGrainsSlider.setSliderStyle(Slider::LinearBarVertical);
        noGrainsSlider.setRange (1, maxNoGrains, 1);
        noGrainsSlider.setValue (round(maxNoGrains/2), dontSendNotification);  // [8
        noGrainsSlider.addListener (this);
        addAndMakeVisible (noGrainsSlider);
        
        noGrainsLabel.setText ("No. Grains", dontSendNotification);
        addAndMakeVisible (noGrainsLabel);
        
        gainSlider.setSliderStyle(Slider::LinearBarVertical);
        gainSlider.setRange (-64.0, 6, 0.01);
        gainSlider.setValue (0.0, dontSendNotification);  // [8
        gainSlider.addListener (this);
        addAndMakeVisible (gainSlider);
        
        gainLabel.setText ("Gain/dB", dontSendNotification);
        addAndMakeVisible (gainLabel);
        
        pulseWidthSlider.setSliderStyle(Slider::LinearHorizontal);
        pulseWidthSlider.setRange (0, 1, 0.01);
        pulseWidthSlider.setValue (0.0, dontSendNotification);  // [8
        pulseWidthSlider.addListener (this);
        addAndMakeVisible (pulseWidthSlider);
   
        pulseWidthLabel.setText ("Pulse Width", dontSendNotification);
        addAndMakeVisible (pulseWidthLabel);
        
        
        addAndMakeVisible(openButton);
        openButton.setButtonText("Open");
        openButton.addListener(this);
        
        addAndMakeVisible(clearButton);
        clearButton.setButtonText("Clear");
        clearButton.addListener(this);
        
        addAndMakeVisible(envelopeSawButton);
        envelopeSawButton.setButtonText("Saw");
        envelopeSawButton.addListener(this);
        
        addAndMakeVisible(envelopeSinButton);
        envelopeSinButton.setButtonText("Sin");
        envelopeSinButton.addListener(this);
        
        addAndMakeVisible(envelopePulseButton);
        envelopePulseButton.setButtonText("Pulse");
        envelopePulseButton.addListener(this);
        
        formatManager.registerBasicFormats();
        setAudioChannels (0, maxNoGrains + channelShift);
        
        //VERY IMPORTANT LINE OF CODE.
        startThread();
        
    }
    
    ~MainContentComponent()
    {
        stopThread(4000);
        shutdownAudio();
    }
    
    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        
        granulator.update(bufferToFill);
        
    }
    
    void releaseResources() override
    {
        currentBuffer = nullptr;
    }
    
    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
        
        g.setColour (Colours::black);
        g.setFont (15.0f);
        g.drawFittedText ("Broken Granulator", 0, 0, getWidth(), 30, Justification::centred, 1);
        
    }
    
    void resized() override
    {
        //Sliders
        int bottomMargin = 140;
        int numberOfVirticleDivitions = 7;
        granulatorSpeedSlider.setBounds ((getWidth()/numberOfVirticleDivitions)*2, 60, 30, getHeight() -bottomMargin);
        grainSpeedSlider.setBounds      ((getWidth()/numberOfVirticleDivitions)*3, 60, 30, getHeight()- bottomMargin);
        grainLengthSlider.setBounds     ((getWidth()/numberOfVirticleDivitions)*4, 60, 30, getHeight()- bottomMargin);
        noGrainsSlider.setBounds        ((getWidth()/numberOfVirticleDivitions)*5, 60, 30, getHeight()- bottomMargin);
        gainSlider.setBounds            ((getWidth()/numberOfVirticleDivitions)*6, 60, 30, getHeight()- bottomMargin);
        
        pulseWidthSlider.setBounds      ((getWidth()/7)*2, 400, getHeight()- bottomMargin, 30);
        
        //Labels
        granulatorSpeedLabel.setBounds  ((getWidth()/numberOfVirticleDivitions)*2 - 30, 30, 100, 20);
        grainSpeedLabel.setBounds       ((getWidth()/numberOfVirticleDivitions)*3 - 30, 30, 100, 20);
        grainLengthLabel.setBounds      ((getWidth()/numberOfVirticleDivitions)*4 - 30, 30, 100, 20);
        noGrainsLabel.setBounds         ((getWidth()/numberOfVirticleDivitions)*5 - 25, 30, 100, 20);
        gainLabel.setBounds             ((getWidth()/numberOfVirticleDivitions)*6, 30, 100, 20);
        
        pulseWidthLabel.setBounds       ((getWidth()/numberOfVirticleDivitions)*4, 390, 100, 20);
        
        //Buttons
        openButton.setBounds            ((getWidth()/numberOfVirticleDivitions)*1 - 25, 70, 50, 20);
        clearButton.setBounds           ((getWidth()/numberOfVirticleDivitions)*1 -25 , 110, 50, 20);
        envelopeSawButton.setBounds     ((getWidth()/numberOfVirticleDivitions)*1 -25 , 150, 50, 20);
        envelopeSinButton.setBounds     ((getWidth()/numberOfVirticleDivitions)*1 -25 , 190, 50, 20);
        envelopePulseButton.setBounds   ((getWidth()/numberOfVirticleDivitions)*1 -25 , 230, 50, 20);
    }
    
    //==============================================================================
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &granulatorSpeedSlider)   { granulator.setSpeed       (slider->getValue()); }
        if (slider == &grainSpeedSlider)        { granulator.setGrainSpeed  (slider->getValue()); }
        if (slider == &grainLengthSlider)       { granulator.setGrainLength (slider->getValue()); }
        if (slider == &noGrainsSlider)          { granulator.setNoGrains    (slider->getValue()); }
        if (slider == &gainSlider)              { granulator.setGain        (slider->getValue()); }
        if (slider == &pulseWidthSlider)        { granulator.setPulseWidth  (slider->getValue()); }
        
    }
    
    
private:
    //==============================================================================
    //Private Methods...
    
    void buttonClicked (Button* button) override
    {
        if (button == &openButton)          openButtonClicked();
        if (button == &clearButton)         clearButtonClicked();
        if (button == &envelopeSawButton)   envelopeSawButtonClicked();
        if (button == &envelopeSinButton)   envelopeSinButtonClicked();
        if (button == &envelopePulseButton) envelopePulseButtonClicked();
    }
    
    //This will be used to implement our background thread that the audio is run though.
    void run() override
    {
        while (! threadShouldExit())
        {
            checkForPathToOpen();
            //Check for buffers to free. This waits for the notify() function in openButtonClicked().
            checkForBuffersToFree();
            //Wait 500ms before interating through the loop again.
            wait(500);
        }
    }
    
    //Checking for buffers to remove when we change over files/open a new file.
    void checkForBuffersToFree()
    {
        //Iterate backwards through the array of buffers as not to screw up the iteration if we do remove something.
        for (int i = buffers.size(); --i >= 0;)
        {
            //Allocate a pointer to the buffer we at this iteration.
            ReferenceCountedBuffer::Ptr buffer (buffers.getUnchecked (i));
            //If the buffer is the second element of this buffer array then we remove
            if (buffer->getReferenceCount() == 2)
                //This action will free the memory if the buffer at this iteration and it's pointer will become a nullprt when function terminates.
                buffers.remove (i);
        }
    }
    
    void checkForPathToOpen()
    {
        //Create string
        String pathToOpen;
        //Swap strings
        swapVariables (pathToOpen, chosenPath);
        //If we have data within the string pathToOpen.
        if (pathToOpen.isNotEmpty())
        {
            //Create a file of type pathToOpen
            const File file (pathToOpen);
            //Create a reader for this file using the format manager.
            ScopedPointer<AudioFormatReader> reader (formatManager.createReaderFor (file));
            //If the reader deduces that the path leads to a good file.
            if (reader != nullptr)
            {
                //Reset the granulator position so that we don't start the new sound file at a location that there is no data.
                granulator.setGranulatorPosition(0);
                //Create a new Reference counted buffer and a pointer to it that is the same size as the choosen file.
                ReferenceCountedBuffer::Ptr newBuffer = new ReferenceCountedBuffer (file.getFileName(),
                                                                                    reader->numChannels,
                                                                                    reader->lengthInSamples);
                //Copy in all of the file data in to the identically sized buffer.
                reader->read (newBuffer->getAudioSampleBuffer(), 0, reader->lengthInSamples, 0, true, true);
                //Reasign our current buffer pointer to point at our new buffer
                currentBuffer = newBuffer;
                //Add new buffer to the array of buffers.
                buffers.add (newBuffer);
            }
        }
    }
    
    void openButtonClicked()
    {
        FileChooser chooser("Only wav files", File::nonexistent, "*.wav");
        
        if(chooser.browseForFileToOpen()){
            //.getResult returns file type and is imediatly copied to file
            const File file(chooser.getResult());
            //We then create a string representing the directory to the file
            String path(file.getFullPathName());
            //Change Choosen path to the string path was representing and vice versa
            swapVariables(chosenPath, path);
            //Call notify.
            notify();
        }
    }
    
    void clearButtonClicked()
    {
        //Render the current buffer null when the clear button is clicked.
        currentBuffer = nullptr;
    }
    
    void envelopePulseButtonClicked()
    {
        granulator.setEnvelopesIt(2);
    }
    
    void envelopeSawButtonClicked()
    {
        granulator.setEnvelopesIt(1);
    }
    
    void envelopeSinButtonClicked()
    {
        granulator.setEnvelopesIt(0);
    }
    
    // Your private member variables go here...
    
    //Granulator Gui Variables
    //Slider to set the speed of the sample playback
    //Speed of grain iteration
    //Length of each grain
    //Number of grains
    Slider  granulatorSpeedSlider, grainSpeedSlider, grainLengthSlider, noGrainsSlider, gainSlider, pulseWidthSlider;
    Label   granulatorSpeedLabel, grainSpeedLabel, grainLengthLabel, noGrainsLabel, gainLabel, pulseWidthLabel;
    
    //File Streaming
    //Buttons for opening a new file or clearing the current file.
    TextButton openButton, clearButton;
    //Envelope Buttons
    TextButton envelopeSawButton, envelopeSinButton, envelopePulseButton;
    //This is extract the format information from the chosen file.
    AudioFormatManager formatManager;
    //This will be used to store multiple soundfiles when we are changing file.
    ReferenceCountedArray<ReferenceCountedBuffer> buffers;
    //This will be a pointer to the current buffer we wish to play.
    ReferenceCountedBuffer::Ptr currentBuffer;
    //This is the file path to the chosen file.
    String chosenPath;
    
    Granulator granulator;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
