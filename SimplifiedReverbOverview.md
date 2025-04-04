# Simple Reverb Implementation

This is a C++ implementation of a reverb audio effect. Reverb (short for reverberation) recreates the sound characteristics of a physical space by simulating how sound waves reflect off surfaces and decay over time.

## How This Reverb Works

This implementation uses a classic "Schroeder reverb" design with the following components:

1. **Comb Filters**: A series of parallel comb filters that create the initial reflections and echoes. Each comb filter has:
   - A delay line (stores audio samples)
   - A feedback loop with gain (controls decay time)
   - A low-pass filter in the feedback path (controls damping)

2. **Allpass Filters**: A series of allpass filters in series that diffuse the sound, creating a smoother, denser reverb. Each allpass filter has:
   - A delay line
   - A fixed feedback coefficient

## Parameters

This reverb has several user-controllable parameters:

1. **Room Size**: Controls the size of the simulated space by adjusting the feedback of the comb filters. Higher values create a larger space with longer decay.

2. **Damping**: Controls how quickly high frequencies decay in the reverberation. Higher values create a warmer, more absorbent space.

3. **Wet Level**: Controls the amount of processed (reverberant) signal in the output.

4. **Dry Level**: Controls the amount of unprocessed (direct) signal in the output.

5. **Width**: Controls the stereo width of the reverb. At 1.0, the reverb is fully stereo; at 0.0, the reverb is mono.

6. **Freeze Mode**: When active, it creates an infinite reverb by maximizing the feedback in the comb filters.

## Example Use Cases

- **Small Room**: Low room size, medium damping, low wet level
- **Large Hall**: High room size, low damping, high wet level
- **Cathedral**: Very high room size, low damping, high wet level
- **Special Effects**: High room size, high freeze mode, high wet level

## How to Use This Code

1. Create an instance of `SimpleAudioProcessor`
2. Set the sample rate and parameters
3. Process your audio data through the processor
4. The processor will add reverb to your audio

## Demo

This implementation includes a demo that:
1. Generates a simple sine wave
2. Applies reverb to it
3. Saves both the original and processed audio as WAV files
