# Connectors

Connectors are adapters that give Tracklab and its AI co-producer access to external services or local resources.

They are not part of the audio engine. They provide structured capabilities that can be called by the app and the AI orchestration layer.

## Connector Manifest

Each connector will define a manifest with:

```json
{
  "id": "splice",
  "name": "Splice",
  "authType": "oauth",
  "capabilities": ["search_samples", "preview_sample", "import_sample"],
  "aiTools": ["splice.searchSamples", "splice.importSample"]
}
```

### Fields

- `id`: Stable machine-readable connector id.
- `name`: User-facing connector name.
- `authType`: Authentication model used by the connector.
- `capabilities`: App-level capabilities exposed by the connector.
- `aiTools`: Tools registered with the AI orchestration layer.

## Auth Types

Planned auth types:

- OAuth
- API key
- Folder watch
- Embedded webview

Some connectors may support more than one auth type if the service requires it.

## AI Tool Registration

Connectors register tools with the AI orchestration layer at startup. Each tool must provide:

- A stable tool name
- A typed input schema
- A typed output schema
- A capability description
- A permission and safety policy

The AI layer can request tool calls, but the app controls execution. User confirmation is required for destructive or externally visible actions.

## MCP Alignment

This connector pattern is inspired by the Model Context Protocol. Tracklab may eventually adopt MCP directly if it fits the desktop app architecture and user permission model.

## Launch Connectors

### Splice

Planned capabilities:

- Search samples by text, tag, BPM, key, and instrument
- Preview samples
- Import downloaded samples into the current project
- Preserve source and license metadata where available

### Spotify

Planned capabilities:

- Search reference tracks
- Read track metadata
- Collect references for project context
- Provide high-level playlist and artist context where allowed by the API

### Google Drive

Planned capabilities:

- Browse project folders
- Search stems, references, and exports
- Import files into the local project
- Link external project assets

### Local Library

Planned capabilities:

- Watch user-selected folders
- Index local samples and loops
- Search metadata and filenames
- Import local files into the timeline

### Suno

Planned capabilities:

- Track generated ideas
- Import generated audio where available
- Preserve generation metadata
- Use generated material as reference context

## Custom Connectors

Long term, users should be able to add their own custom connectors. The target model is a documented manifest, typed tools, explicit permissions, and a local development workflow for testing connectors before they are enabled in production projects.
