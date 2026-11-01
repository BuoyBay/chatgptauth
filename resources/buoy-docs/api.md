---
title: "ChatGPTAuth API"
summary: "Sign-in attempts, verified profiles, renewal, revocation, and explicit credential access."
category: "Reference"
order: 20
keywords: ["oauth", "refresh", "revoke", "credentials", "oidc", "scopes"]
symbols: ["Base64URL", "PKCEChallenge", "NewChatGPTHostID", "ChatGPTAuthClient", "ChatGPTAuthClient.Begin", "ChatGPTAuthClient.BeginForProfile", "ChatGPTAuthClient.Exchange", "ChatGPTAuthClient.Refresh", "ChatGPTAuthClient.Revoke", "ChatGPTAuthClient.RestoreProfile", "ChatGPTProfile", "ChatGPTProfile.AccessToken", "ChatGPTProfile.ChildEnvironment", "ChatGPTProfile.ExportCredentials", "ChatGPTProfile.ClearCredentials", "ChatGPTSignIn", "ChatGPTSignIn.AcceptCallback", "ChatGPTSignIn.Cancel"]
related: ["index.md", "getting-started.md", "typed-api.md"]
---

# API reference

## Client and attempt

| Symbol | Contract |
| --- | --- |
| `NewChatGPTHostID()` | Random opaque UUID for one host installation; persist/reuse it |
| `Base64URL(data)` / `PKCEChallenge(verifier)` | Base64url encoding and PKCE S256 challenge helpers |
| `ChatGPTAuthClient(appName, hostID)` | Production client with fixed issuer and authorization/token endpoints |
| `Begin(port, callbackPath = "/auth/callback")` | New registration attempt with fresh state, nonce, PKCE verifier |
| `BeginForProfile(profile, port)` | Returning authorization retaining issued client, host, path, and verified identity |
| `ChatGPTSignIn.AuthorizationURL` / `CallbackURI` | Read-only browser authorization and exact callback addresses |
| `AcceptCallback(callbackURL)` | Validate the actual callback and retain its one-time authorization result |
| `Cancel()` | Invalidate/clear a pending attempt |
| `Exchange(attempt)` | Exchange and verify on a worker; return a `ChatGPTProfile` |
| `ForLoopbackTesting(appName, hostID, issuerURL)` | Explicit fixture-only factory, restricted to bare HTTP 127.0.0.1 origins |

The test factory is for local synthetic issuers, never real credentials. The production API does not expose an arbitrary issuer override.

## Profile and credentials

| Symbol | Contract |
| --- | --- |
| `Subject`, `Email`, `Issuer`, `ClientID`, `HostID` | Nonsecret identity/registration properties; email is a display hint |
| `GrantedScopes`, `PlanEnabled`, `ExpiresAt` | Actual grant and expiry information |
| `AccessToken()` | Explicit bearer-token access only with required direct-use/invoke scopes and unexpired token |
| `ChildEnvironment()` | Fresh map containing `ACCESS_TOKEN` for one child process; no global environment change |
| `ExportCredentials()` | Explicit secret JSON object for protected host storage |
| `ClearCredentials()` | Clear local credentials while retaining registration/account mapping |
| `RestoreProfile(record)` | Verify saved signed identity, host, callback path, and nonce; return refresh-only state |
| `Refresh(profile)` | Renew with issued client and rotating refresh token; validate replacement identity when supplied |
| `Revoke(profile)` | Discover and invoke same-origin revocation; clear credentials only after confirmed HTTP 200 |

Do not place tokens, authorization URLs, child environments, or exported credential records in logs, command arguments, diagnostics, source control, or browser storage. File persistence needs atomic owner-only storage; a native credential store is another host choice.

Failed refresh preserves the previous profile rather than partially replacing credentials. An omitted refresh scope retains the existing grant; restored profiles start without trusted saved grant claims. Failed revocation retains credentials for retry. If a host chooses local sign-out anyway, it must explicitly clear them and communicate that remote revocation was unconfirmed.

## Verification and execution

ID tokens allow RS256, require an unambiguous exact key ID, and validate issuer, issued-client audience/authorized party, subject, nonce, and time claims. RSA moduli are 2048–8192 bits. State/nonce comparisons use constant-time byte comparison; issuer, client IDs, and scopes compare case-sensitively.

Untrusted JSON rejects excessive nesting, NUL, malformed Unicode/numbers, and non-UTF-8 input before native parsing. Endpoint responses and complete ID tokens are capped at 1 MiB after synchronous transport buffering. TLS verification remains enabled, redirects are refused, and discovery/JWKS/revocation stay on the configured issuer origin. Network calls have 30-second timeouts and belong on workers.

For Codex custom-provider integration, inject `ChildEnvironment()` into that one explicitly configured process and restart/resume after renewal. Never reinterpret the verified subject as a different API's account/workspace ID. The package does not translate plan credentials into an unrestricted API key or select models.

Continue with [Getting started](getting-started.md) or return to the [overview](index.md).


[Full typed API: properties, parameters, defaults, and returns](typed-api.md)
