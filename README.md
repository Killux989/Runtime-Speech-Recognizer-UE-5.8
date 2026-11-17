# Runtime Speech Recognizer (UE 5.8)

Offline, real-time speech-to-text for Unreal Engine 5.8, powered by [whisper.cpp](https://github.com/ggml-org/whisper.cpp).

Maintained by Killux. Based on the open-source [Runtime Speech Recognizer](https://github.com/gtreshchev/RuntimeSpeechRecognizer) by Georgy Treshchev (MIT). This is an independent community upgrade, not the Fab version of the plugin.

## Changes from the original
- UE 5.8 support, whisper.cpp 1.9.5 / ggml 0.29
- Vulkan GPU acceleration built in, with CPU (AVX2) fallback
- Built-in microphone capture with device selection (no extra plugin needed)
- `Find Best Matching Command` node for fuzzy keyword and phrase voice commands
- New nodes: Set GPU Acceleration, Set GPU Device ID, Get Detected Language, Get Language Code / Full Name
- Fixed recognition parameter setters and Start Speech Recognition always failing
- Blueprint events now fire on the game thread

## Requirements
- UE 5.8, Windows (prebuilt binaries). Other 5.x versions need a C++ project and may need small build-file changes.
- CPU with AVX2. GPU acceleration needs a Vulkan-capable GPU driver.
- Building from source: the generated Vulkan shaders are not in the repo. Run `Scripts/GenerateVulkanShaders.ps1` (needs the Vulkan SDK) for GPU support; without it the plugin builds CPU-only.

## Documentation
See the [Wiki](https://github.com/Killux989/Runtime-Speech-Recognizer-UE-5.8/wiki) for setup, every Blueprint node, and demos for single-keyword and sentence voice commands.

## License
MIT. See [LICENSE](LICENSE). Third-party licenses are in `Source/ThirdParty`.
