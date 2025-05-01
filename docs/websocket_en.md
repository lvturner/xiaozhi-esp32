**Protocol Documentation**

**1. Transmission**  
The protocol uses JSON messages and Opus-encoded binary audio frames over a WebSocket connection. It supports functionalities such as audio streaming, TTS playback, speech recognition, state management, and IoT command delivery.

**2. JSON Message Structure**  
Each message includes a "type" field to identify the business logic, such as TTS, STT, IoT, or WakeWord. Additional fields like "session_id" are used to distinguish sessions. Example:
```json
{
  "session_id": "12345",
  "type": "listen",
  "state": "start"
}
```

**3. WebSocket Handshake**  
- **Client → Server**:  
  ```json
  {
    "type": "hello",
    "version": 1,
    "transport": "websocket",
    "audio_params": {
      "format": "opus",
      "sample_rate": 16000,
      "channels": 1,
      "frame_duration": 60
    }
  }
  ```
- **Server → Client**:  
  ```json
  {
    "type": "hello",
    "transport": "websocket",
    "audio_params": {
      "sample_rate": 16000
    }
  }
  ```

**4. Audio Streaming**  
- **Client** sends Opus-encoded binary frames over the WebSocket.  
- **Server** processes these frames for speech recognition (STT) or TTS synthesis.

**5. State Management**  
- **"listen"**: Initiates audio input.  
- **"stop"**: Ends the session.  
- **"play"**: Triggers TTS output.  

**6. IoT Commands**  
- Example:  
  ```json
  {
    "type": "iot",
    "command": "toggle_light",
    "device_id": "light_001"
  }
  ```

**7. Error Handling**  
- **Error Logs**:  
  ```c
  ESP_LOGE(TAG, "Audio stream failed: %d", error_code);
  ```
- **Common Errors**:  
  - `400`: Invalid JSON format.  
  - `503`: Server unavailable.  

**8. Session Control**  
- **"session_id"**: Unique identifier for tracking interactions.  
- **Timeout**: Sessions expire after 30 seconds of inactivity.  

**9. Example Workflow**  
1. **Handshake**: Client and server negotiate audio parameters.  
2. **Audio Input**: Client sends Opus frames for STT.  
3. **TTS Output**: Server responds with synthesized audio.  
4. **IoT Command**: Server executes device control commands.  

**10. Security**  
- **Authentication**: Requires a token in the handshake.  
  ```json
  {
    "type": "auth",
    "token": "abc123"
  }
  ```

**11. Extensibility**  
- New "type" values can be added (e.g., `video_stream`, `sensor_data`).  
- Custom fields are allowed for application-specific data.  

**12. Testing**  
- **Tools**: Use `curl` or WebSocket clients to simulate interactions.  
- **Validation**: JSON schema checks ensure message integrity.  

**13. Troubleshooting**  
- **Logs**: Check `ESP_LOGE` outputs for runtime errors.  
- **Network**: Verify WebSocket connectivity and latency.  

**14. References**  
- [WebSocket Protocol](https://tools.ietf.org/html/rfc6455)  
- [Opus Codec](https://tools.ietf.org/html/rfc6718)  

This document outlines the protocol for real-time communication systems, ensuring compatibility and scalability for diverse applications.
