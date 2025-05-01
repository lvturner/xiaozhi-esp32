<details type="reasoning" done="true" duration="13">
<summary>Thought for 13 seconds</summary>
> Okay, I need to translate this technical document from Chinese to English. Let me start by reading through the entire text to understand the context and technical terms. The document is about a protocol for a WebSocket-based communication system involving JSON messages and Opus audio frames. It covers handshake, audio streaming, STT, TTS, IoT commands, error handling, and other details.
> 
> First, I'll go through each section one by one. The title is "Protocol Documentation," so I'll translate that. Then, the sections like "Translation" might be a typo, but looking at the original, it's actually "Translate" as part of the user's request. Wait, no, the user provided the Chinese text and wants it translated. So the entire content is in Chinese, and I need to translate it into English.
> 
> I need to be careful with technical terms. For example, "WebSocket" should stay as is. Terms like "鉴权" are "authentication," "会话控制" is "session control," "错误处理" is "error handling," etc. Some terms like "Opus" are proper nouns and should remain the same.
> 
> I'll start translating each section. The user provided the Chinese text, so I need to make sure that the translation is accurate. For example, the first line after the title is "Translate this document," which the user wants translated. But the actual content is the document itself. Wait, no. The user provided the Chinese text, which is the document they want translated. So the entire text from "Protocol Documentation" onwards is the content to be translated.
> 
> I need to maintain the structure, including the numbered sections and bullet points. For example, "1. 传输" becomes "1. Transmission," but in the original, it's "传输" which is "Transmission." However, in the user's provided text, the first line after the title is "Translate this document," which might be a mistake. Wait, looking at the user's message, they said "Translate this document" as the first line, but the actual content is the Chinese text. Maybe the user intended to provide the Chinese text for translation. So the entire text from "Protocol Documentation" down is the content to translate.
> 
> I'll proceed to translate each section, ensuring that technical terms are correctly rendered. For example, "会话" becomes "session," "IoT指令" becomes "IoT commands," "错误日志" becomes "error logs." I'll also check for consistency in terminology throughout the translation.
> 
> Another thing to watch for is the JSON examples. The keys like "session_id," "type," "state," etc., should remain in English. The values might need translation if they are in Chinese, but in the examples, they are in English. For instance, "state": "start" remains as is.
> 
> I'll also check for proper nouns like "ESP_LOGE" which is a macro in ESP-IDF, so it should stay as is. "TAG" is a variable name, so it remains.
> 
> I need to ensure that the translated document maintains the original structure, including headers, bullet points, and code blocks. For example, the "消息示例" section has examples in JSON, which should be preserved with proper indentation and syntax.
> 
> After translating each section, I'll review the entire document to ensure that the flow is correct, technical terms are consistent, and that all information is accurately conveyed in English without losing the original meaning. I'll also check for any grammatical errors or awkward phrasings that might arise during the translation process.
</details>
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
