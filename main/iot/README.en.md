# IoT Control Module  

This module implements the IoT control functionality for the XiaoZhi AI voice chatbot, enabling users to control various IoT devices connected to the ESP32 development board through voice commands.  

## Working Principle  

The workflow of the IoT control module is as follows:  

1. **Device Registration**: During the development board initialization phase (e.g., in `compact_wifi_board.cc`), various IoT devices are registered into the system via `ThingManager`.  
2. **Device Description**: The system sends device description information (including name, properties, methods, etc.) to the AI server via a communication protocol (e.g., MQTT or WebSocket).  
3. **User Interaction**: The user interacts with the XiaoZhi AI via voice, expressing their intent to control IoT devices.  
4. **Command Execution**: The AI server parses the user's intent, generates control commands, and sends them back to the ESP32 via the protocol. The `ThingManager` then dispatches the commands to the corresponding devices for execution.  
5. **Status Update**: After executing the command, device state changes are collected by `ThingManager` and sent back to the AI server to maintain state synchronization.  

## Core Components  

### ThingManager  

`ThingManager` is the core management class of the IoT control module, implemented as a singleton:  

- `AddThing`: Registers an IoT device.  
- `GetDescriptorsJson`: Retrieves the description information of all devices for reporting device capabilities to the AI server.  
- `GetStatesJson`: Retrieves the current state of all devices, with an option to return only the changed parts.  
- `Invoke`: Invokes the corresponding device method based on commands issued by the AI server.  

### Thing  

`Thing` is the base class for all IoT devices, providing the following core functionalities:  

- **Property Management**: Defines the queryable state of a device via `PropertyList`.  
- **Method Management**: Defines the executable operations of a device via `MethodList`.  
- **JSON Serialization**: Converts device descriptions and states into JSON format for network transmission.  
- **Command Execution**: Parses and executes instructions from the AI server.  

## Device Design Example  

### Lamp  

The lamp is a simple IoT device example that controls the on/off state of an LED via GPIO:  

```cpp
class Lamp : public Thing {
private:
    gpio_num_t gpio_num_ = GPIO_NUM_18;  // GPIO pin  
    bool power_ = false;                 // Lamp on/off state  

public:
    Lamp() : Thing("Lamp", "A test lamp") {
        // Initialize GPIO  
        InitializeGpio();  

        // Define property: power (indicates the lamp's on/off state)  
        properties_.AddBooleanProperty("power", "Whether the lamp is on", [this]() -> bool {
            return power_;  
        });  

        // Define method: TurnOn (turns on the lamp)  
        methods_.AddMethod("TurnOn", "Turn on the lamp", ParameterList(), [this](const ParameterList& parameters) {
            power_ = true;  
            gpio_set_level(gpio_num_, 1);  
        });  

        // Define method: TurnOff (turns off the lamp)  
        methods_.AddMethod("TurnOff", "Turn off the lamp", ParameterList(), [this](const ParameterList& parameters) {
            power_ = false;  
            gpio_set_level(gpio_num_, 0);  
        });  
    }  
};  
```  

Users can control the lamp with voice commands like, "XiaoZhi, please turn on the lamp."  

### Speaker  

The speaker control implements volume adjustment functionality:  

```cpp
class Speaker : public Thing {
public:
    Speaker() : Thing("Speaker", "Speaker") {
        // Define property: volume (current volume level)  
        properties_.AddNumberProperty("volume", "Current volume level", [this]() -> int {
            auto codec = Board::GetInstance().GetAudioCodec();  
            return codec->output_volume();  
        });  

        // Define method: SetVolume (sets the volume)  
        methods_.AddMethod("SetVolume", "Set volume", ParameterList({
            Parameter("volume", "Integer between 0 and 100", kValueTypeNumber, true)  
        }), [this](const ParameterList& parameters) {
            auto codec = Board::GetInstance().GetAudioCodec();  
            codec->SetOutputVolume(static_cast<uint8_t>(parameters["volume"].number()));  
        });  
    }  
};  
```  

Users can control the speaker volume with voice commands like, "XiaoZhi, set the volume to 50."  

## Designing Custom IoT Devices  

To design a new IoT device, follow these steps:  

1. **Create a Device Class**: Inherit from the `Thing` base class.  
2. **Define Properties**: Use `properties_` to add queryable states for the device.  
3. **Define Methods**: Use `methods_` to add executable operations for the device.  
4. **Implement Hardware Control**: Implement hardware control in the method callbacks.  
5. **Register the Device**: Register the device in one of two ways (see below) and add the device instance during board-level initialization.  

### Two Device Registration Methods  

1. **Using the DECLARE_THING Macro**: Suitable for generic device types.  
   ```cpp
   // Add at the end of the device implementation file  
   DECLARE_THING(MyDevice);  
   
   // Then, in board-level initialization  
   thing_manager.AddThing(iot::CreateThing("MyDevice"));  
   ```  

2. **Directly Creating a Device Instance**: Suitable for board-specific devices.  
   ```cpp
   // In board-level initialization  
   auto my_device = new iot::MyDevice();  
   thing_manager.AddThing(my_device);  
   ```  

### Recommended Device Implementation Locations  

Depending on the device's generality, choose one of the following implementation locations:  

1. **Generic Devices**: Place in the `main/iot/things/` directory and register using `DECLARE_THING`.  
2. **Board-Specific Devices**: Implement directly in the board-level directory (e.g., `main/boards/your_board/`) and register by directly creating an instance.  

This flexibility allows you to design device implementations specific to different boards while maintaining reusability for generic devices.  

### Property Types  

IoT devices support the following property types:  

- **Boolean** (`kValueTypeBoolean`): On/off states, etc.  
- **Number** (`kValueTypeNumber`): Temperature, volume, etc.  
- **String** (`kValueTypeString`): Device names, status descriptions, etc.  

### Method Parameters  

Device methods can define parameters, supporting the following parameter types:  

- **Boolean**: Switches, etc.  
- **Number**: Temperature, volume, etc.  
- **String**: Commands, modes, etc.  

void InitializeIot() {
    auto& thing_manager = iot::ThingManager::GetInstance();  
    thing_manager.AddThing(iot::CreateThing("Speaker"));  
    thing_manager.AddThing(iot::CreateThing("Lamp"));  
}  
```  

Afterward, users can control these devices with voice commands, such as:  

- "XiaoZhi, turn on the lamp."  
- "I'm going to sleep, turn off the lamp for me."  
- "The volume is a bit too low."  
- "Set the volume to 80%."  

The AI server will parse these voice commands into corresponding device control commands and send them to the ESP32 for execution via the protocol.  

## Notes  

### Randomness in Large Model Control  

Since voice control is handled by a large language model (LLM), the control process may exhibit some randomness and unpredictability. To enhance safety and reliability, consider the following recommendations:  

1. **Add Warning Messages for Critical Operations**: For potentially dangerous or irreversible operations, include warning messages in the method description.  
   ```cpp
   methods_.AddMethod("PowerOff", "Turn off system power [Warning: This operation will completely shut down the device. Use with caution.]",  
                     ParameterList(), [this](const ParameterList& parameters) {
       // Power-off implementation  
   });  
   ```  

2. **Implement a Confirmation Mechanism**: Important operations should explicitly require user confirmation in the description.  
   ```cpp
   methods_.AddMethod("ResetToFactory", "Restore factory settings [User confirmation required]",  
                     ParameterList(), [this](const ParameterList& parameters) {
       // Factory reset implementation  
   });  
   ```  

### Communication Protocol Limitations  

The current IoT protocol (version 1.0) has the following limitations:  

1. **Unidirectional Control Flow**: The LLM can only issue commands and cannot immediately retrieve execution results.  
2. **Delayed Status Updates**: Device state changes can only be known in the next round of conversation by reading property values.  
3. **Asynchronous Feedback**: If operation result feedback is needed, it must be implemented indirectly via device properties.  

### Best Practices  

1. **Use Meaningful Property Names**: Property names should clearly express their meaning to help the LLM understand and use them.  
2. **Avoid Ambiguous Method Descriptions**: Provide clear natural language descriptions for each method to help the LLM accurately understand and invoke them.