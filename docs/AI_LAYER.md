# AI Layer

The Tracklab AI co-producer is designed as an orchestration layer, not as an audio engine.

It helps the user reason about project state, search connected services, organize assets, and choose next actions. It works through structured data and explicit tools.

## Input Streams

The AI layer receives three main input streams.

### Project State

Project state includes structured information from the DAW, such as tempo, key if known, tracks, clips, arrangement markers, selected regions, and recent actions.

### Connector State

Connector state includes available connectors, authenticated services, search results, local asset records, reference metadata, and connector permissions.

### User Chat

User chat captures intent, constraints, feedback, and creative direction. It is combined with structured state before the model is called.

## Deterministic Layer

The deterministic layer runs underneath the AI model. It handles decisions that should be repeatable and explainable.

- Camelot wheel matching for musical key compatibility
- BPM compatibility checks for tempo fit
- Connector capability checks before tool use
- Permission checks before external or file system actions

The LLM handles fuzzy decisions after deterministic filtering, such as whether a sound feels darker, cleaner, wider, more aggressive, or better aligned with a reference.

## Tool Use Pattern

Tracklab will use Anthropic's API with tool use. The app exposes approved tools from the engine and connector layers, then validates every tool request before execution.

Typical tool flow:

1. Build structured context from project state, connector state, and user chat.
2. Run deterministic filters where applicable.
3. Ask the model to choose or call tools.
4. Validate tool calls against schemas, permissions, and current app state.
5. Execute approved tools.
6. Return structured results to the model or present them to the user.

## No Raw Audio Access

The AI never operates on raw audio in v1. It only sees structured metadata.

This keeps the system fast, predictable, and explainable. It also avoids pretending that the AI can hear or analyze audio when v1 is not designed around waveform analysis.

## System Prompt Structure

The system prompt will be defined later. At a high level it will include:

- Role and boundaries for the AI co-producer
- Safety and confirmation rules
- Tool-use rules
- Music theory constraints
- Connector privacy constraints
- Response style for creative and technical tasks

No production prompt is included in this scaffolding pass.

## Privacy

In v1, API calls go directly from the desktop app to Anthropic. There is no Tracklab-owned backend.

Connector authentication and local asset access should remain under user control, with explicit permissions for services and folders.
