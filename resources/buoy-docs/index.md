---
title: "ChatGPTAuth"
summary: "Host-owned OAuth PKCE sign-in with verified identity and renewable in-memory profiles."
category: "Overview"
order: 0
keywords: ["oauth", "pkce", "chatgpt", "identity", "tokens", "profiles"]
related: ["getting-started.md", "api.md", "typed-api.md"]
---

# ChatGPTAuth

ChatGPTAuth implements the supported public-client Sign in with ChatGPT flow for a Buoy host. It creates authorization-code PKCE attempts, verifies RS256 ID tokens using the issuer's JWKS, binds identity to an issued client registration, and renews in-memory profiles.

The host supplies the loopback listener, system-browser integration, timeout/cancellation UI, and protected persistence. Constructing or using the package does not automatically open a browser, select an account, save credentials, launch Codex, or request inference.

## Identity and usage

Use a real application name and one persisted `NewChatGPTHostID()` per host installation. That host identifier is distinct from an account's issued OAuth client ID. Keep profiles separate by issued-client and verified-subject pair; matching email text does not merge accounts or prove workspace identity.

A verified identity is not permission to use every model. `PlanEnabled` and `AccessToken()` depend on the actual granted scopes and token expiry, and the service authorizes each request. Account eligibility and model availability remain server-controlled.

## Platform requirements

The package requires Buoy 0.72+, the bundled `crypto` package, and its native RS256 bridge. macOS uses Security/CoreFoundation, Windows uses BCrypt, and Linux requires OpenSSL 3 at runtime. Current source documentation records runtime verification on Linux x86_64 and macOS arm64; Windows x86_64 was cross-linked, while Windows and Intel Mac runtime execution remain unverified.

Start with the [getting-started guide](getting-started.md), then read the [API reference](api.md) for profile renewal, persistence, and token handoff.
