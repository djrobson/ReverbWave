/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

// This is your project's AppConfig.h file for the VST3 plugin
// It ensures that the correct plugin definitions are available

#pragma once

// [Plugin Settings]
#ifndef  JucePlugin_Build_VST
 #define JucePlugin_Build_VST              0
#endif
#ifndef  JucePlugin_Build_VST3
 #define JucePlugin_Build_VST3             1
#endif
#ifndef  JucePlugin_Build_AU
 #define JucePlugin_Build_AU               0
#endif
#ifndef  JucePlugin_Build_AUv3
 #define JucePlugin_Build_AUv3             0
#endif
#ifndef  JucePlugin_Build_AAX
 #define JucePlugin_Build_AAX              0
#endif
#ifndef  JucePlugin_Build_Standalone
 #define JucePlugin_Build_Standalone       1
#endif

// Plugin identification
#ifndef  JucePlugin_Name
 #define JucePlugin_Name                   "SimpleReverb VST"
#endif
#ifndef  JucePlugin_Desc
 #define JucePlugin_Desc                   "Professional reverb effect"
#endif
#ifndef  JucePlugin_Manufacturer
 #define JucePlugin_Manufacturer           "SimpleReverb"
#endif
#ifndef  JucePlugin_ManufacturerWebsite
 #define JucePlugin_ManufacturerWebsite    "www.example.com"
#endif
#ifndef  JucePlugin_ManufacturerEmail
 #define JucePlugin_ManufacturerEmail      "info@example.com"
#endif
#ifndef  JucePlugin_ManufacturerCode
 #define JucePlugin_ManufacturerCode       0x5265566b // 'ReVb'
#endif
#ifndef  JucePlugin_PluginCode
 #define JucePlugin_PluginCode             0x52657650 // 'RevP'
#endif

// Plugin version
#ifndef  JucePlugin_VersionCode
 #define JucePlugin_VersionCode            0x10000
#endif
#ifndef  JucePlugin_VersionString
 #define JucePlugin_VersionString          "1.0.0"
#endif

// Plugin type
#ifndef  JucePlugin_IsSynth
 #define JucePlugin_IsSynth                0
#endif
#ifndef  JucePlugin_WantsMidiInput
 #define JucePlugin_WantsMidiInput         0
#endif
#ifndef  JucePlugin_ProducesMidiOutput
 #define JucePlugin_ProducesMidiOutput     0
#endif
#ifndef  JucePlugin_IsMidiEffect
 #define JucePlugin_IsMidiEffect           0
#endif
#ifndef  JucePlugin_EditorRequiresKeyboardFocus
 #define JucePlugin_EditorRequiresKeyboardFocus  0
#endif

// Plugin categories
#ifndef  JucePlugin_VSTCategory
 #define JucePlugin_VSTCategory            kPlugCategEffect
#endif
#ifndef  JucePlugin_VST3Category
 #define JucePlugin_VST3Category           "Fx|Reverb"
#endif
