#include <iostream>
#include <string>

// This is a simplified text-based UI for controlling our reverb
// In a real app we would use a proper GUI framework like JUCE, Qt, or ImGui

void printParameter(const std::string& name, float value, int width = 20) {
    std::cout << name;
    
    // Pad with spaces to align the visualization
    for (int i = name.length(); i < width; ++i) {
        std::cout << " ";
    }
    
    // Show a simple text-based slider
    std::cout << "[";
    int position = static_cast<int>(value * 10.0f);
    
    for (int i = 0; i < 10; ++i) {
        if (i < position) {
            std::cout << "=";
        } else if (i == position) {
            std::cout << "|";
        } else {
            std::cout << " ";
        }
    }
    
    std::cout << "] " << (value * 100.0f) << "%" << std::endl;
}

class ReverbGUI {
public:
    void showReverbControls(float roomSize, float damping, float wetLevel, 
                           float dryLevel, float width, float freezeMode,
                           float highFreqDelay, float crossover) {
        std::cout << "===== Reverb Controls =====" << std::endl;
        printParameter("Room Size", roomSize);
        printParameter("Damping", damping);
        printParameter("Wet Level", wetLevel);
        printParameter("Dry Level", dryLevel);
        printParameter("Width", width);
        printParameter("Freeze Mode", freezeMode);
        printParameter("High Freq Delay", highFreqDelay);
        printParameter("Crossover", crossover);
        std::cout << "===========================" << std::endl;
    }
    
    void showPresets() {
        std::cout << "Available Presets:" << std::endl;
        std::cout << "1. Small Room" << std::endl;
        std::cout << "2. Medium Room" << std::endl;
        std::cout << "3. Large Hall" << std::endl;
        std::cout << "4. Cathedral" << std::endl;
        std::cout << "5. Special FX" << std::endl;
        std::cout << "6. Bright Chamber" << std::endl;
        std::cout << "7. Dark Space" << std::endl;
    }
    
    int getPresetChoice() {
        std::cout << "Select a preset (1-7) or 0 to keep current settings: ";
        int choice;
        std::cin >> choice;
        return choice;
    }
    
    void applyPreset(int preset, float& roomSize, float& damping, float& wetLevel, 
                     float& dryLevel, float& width, float& freezeMode,
                     float& highFreqDelay, float& crossover) {
        switch(preset) {
            case 1: // Small Room
                roomSize = 0.2f;
                damping = 0.5f;
                wetLevel = 0.2f;
                dryLevel = 0.8f;
                width = 0.8f;
                freezeMode = 0.0f;
                highFreqDelay = 0.2f;
                crossover = 0.4f;
                break;
                
            case 2: // Medium Room
                roomSize = 0.5f;
                damping = 0.5f;
                wetLevel = 0.3f;
                dryLevel = 0.7f;
                width = 1.0f;
                freezeMode = 0.0f;
                highFreqDelay = 0.3f;
                crossover = 0.5f;
                break;
                
            case 3: // Large Hall
                roomSize = 0.8f;
                damping = 0.3f;
                wetLevel = 0.5f;
                dryLevel = 0.5f;
                width = 1.0f;
                freezeMode = 0.0f;
                highFreqDelay = 0.4f;
                crossover = 0.6f;
                break;
                
            case 4: // Cathedral
                roomSize = 0.9f;
                damping = 0.2f;
                wetLevel = 0.6f;
                dryLevel = 0.4f;
                width = 1.0f;
                freezeMode = 0.0f;
                highFreqDelay = 0.5f;
                crossover = 0.7f;
                break;
                
            case 5: // Special FX
                roomSize = 1.0f;
                damping = 0.0f;
                wetLevel = 0.9f;
                dryLevel = 0.1f;
                width = 1.0f;
                freezeMode = 0.5f;
                highFreqDelay = 0.8f;
                crossover = 0.3f;
                break;
                
            case 6: // Bright Chamber
                roomSize = 0.6f;
                damping = 0.2f;
                wetLevel = 0.4f;
                dryLevel = 0.6f;
                width = 0.9f;
                freezeMode = 0.0f;
                highFreqDelay = 0.1f; // Very little high freq delay for brightness
                crossover = 0.7f;     // High crossover point
                break;
                
            case 7: // Dark Space
                roomSize = 0.7f;
                damping = 0.7f;
                wetLevel = 0.5f;
                dryLevel = 0.5f;
                width = 0.8f;
                freezeMode = 0.0f;
                highFreqDelay = 0.7f; // More high freq delay for a darker sound
                crossover = 0.3f;     // Low crossover point
                break;
                
            default:
                // Keep current settings
                break;
        }
    }
};

// Main function to test the GUI
int main() {
    ReverbGUI gui;
    
    // Default parameters
    float roomSize = 0.5f;
    float damping = 0.5f;
    float wetLevel = 0.33f;
    float dryLevel = 0.4f;
    float width = 1.0f;
    float freezeMode = 0.0f;
    float highFreqDelay = 0.3f;
    float crossover = 0.5f;
    
    // Show current settings
    gui.showReverbControls(roomSize, damping, wetLevel, dryLevel, width, freezeMode, highFreqDelay, crossover);
    
    // Show presets
    gui.showPresets();
    
    // Get user choice
    int choice = gui.getPresetChoice();
    
    if (choice >= 1 && choice <= 7) {
        // Apply the selected preset
        gui.applyPreset(choice, roomSize, damping, wetLevel, dryLevel, width, freezeMode, highFreqDelay, crossover);
        
        // Show the updated settings
        std::cout << "\nApplied preset. New settings:" << std::endl;
        gui.showReverbControls(roomSize, damping, wetLevel, dryLevel, width, freezeMode, highFreqDelay, crossover);
    } else {
        std::cout << "Keeping current settings." << std::endl;
    }
    
    return 0;
}
