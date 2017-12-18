# Multi-Channel Granulator

This project is a granulator which allocates each grain a channel in a buffer. Thus limiting the number of grains of the granulator to the number of audio channels.

In it’s current state the granulator is a Juce Audio App.

The granulator has several different methods of enveloping each grain to enable different expressive qualities.

I performed with this system at [this](http://contingentevents.tumblr.com/post/155343699355/ems-concert-with-thomas-ankersmit) event, granulating ambient piano samples.

The number of channels is still hard coded at the beginning of the code under MaxNumberOfGrains.