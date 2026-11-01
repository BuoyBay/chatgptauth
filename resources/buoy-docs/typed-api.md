---
title: "chatgptauth typed API"
summary: "Source-derived public signatures, properties, parameters, defaults, and return values."
category: "Reference"
order: 21
types: ["ChatGPTAuthClient", "ChatGPTProfile", "ChatGPTSignIn"]
dependencies: []
symbols: ["ChatGPTAuthClient", "ChatGPTProfile", "ChatGPTSignIn"]
related: ["api.md", "getting-started.md", "index.md"]
---

# chatgptauth typed API

Generated from the package source declarations. Platform guards are shown where present; inherited members are documented on the base type. See the [API contracts](api.md) for behavior and limitations.

## ChatGPTAuth functions

### Base64URL

`Public Function Base64URL(data As Bytes) As String`

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `data` | `Bytes` | `ByVal` | Required |

#### Returns

| Type |
| --- |
| `String` |

### PKCEChallenge

`Public Function PKCEChallenge(verifier As String) As String`

RFC 7636 S256 challenge; verifier must contain 43..128 unreserved ASCII characters.

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `verifier` | `String` | `ByVal` | Required |

#### Returns

| Type |
| --- |
| `String` |

### NewChatGPTHostID

`Public Function NewChatGPTHostID() As String`

Generate once per installation/host and save separately from account registrations.

No parameters.

#### Returns

| Type |
| --- |
| `String` |

## ChatGPTProfile

`Public Class ChatGPTProfile`

One issued client/account registration. Secrets have explicit accessors and are never logged or persisted.

### Properties

| Name | Type | Read-Only | Shared | Default | Description |
| --- | --- | --- | --- | --- | --- |
| `Subject` | `String` | Yes | No | — | — |
| `Email` | `String` | Yes | No | — | — |
| `ClientID` | `String` | Yes | No | — | — |
| `HostID` | `String` | Yes | No | — | — |
| `Issuer` | `String` | Yes | No | — | — |
| `PlanEnabled` | `Boolean` | Yes | No | — | — |
| `ExpiresAt` | `Integer` | Yes | No | — | — |
| `GrantedScopes` | `String` | Yes | No | — | — |

### AccessToken

`Public Function AccessToken() As String`

Explicit bearer-token access. Requires current plan permission and an unexpired session.

No parameters.

#### Returns

| Type |
| --- |
| `String` |

### ChildEnvironment

`Public Function ChildEnvironment() As Map Of String, String`

Explicit per-child Codex env_key handoff; caller must never log this map.

No parameters.

#### Returns

| Type |
| --- |
| `Map Of String, String` |

### ExportCredentials

`Public Function ExportCredentials() As JSONItem`

Explicit secret export for host-managed protected storage; never include this in diagnostics.

No parameters.

#### Returns

| Type |
| --- |
| `JSONItem` |

### ClearCredentials

`Public Sub ClearCredentials()`

Local sign-out fallback. A caller must separately report unconfirmed remote revocation.

No parameters.

Returns: no value.

## ChatGPTSignIn

`Public Class ChatGPTSignIn`

A single-use pending attempt, independent of the active account.

### Properties

| Name | Type | Read-Only | Shared | Default | Description |
| --- | --- | --- | --- | --- | --- |
| `AuthorizationURL` | `String` | Yes | No | — | Contains an ID-token hint for returning sign-in; do not log authorization URLs. |
| `CallbackURI` | `String` | Yes | No | — | — |

### Cancel

`Public Sub Cancel()`

No parameters.

Returns: no value.

### AcceptCallback

`Public Sub AcceptCallback(callbackURL As String)`

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `callbackURL` | `String` | `ByVal` | Required |

Returns: no value.

## ChatGPTAuthClient

`Public Class ChatGPTAuthClient`

Public-client OAuth coordinator. Production endpoints are fixed to auth.openai.com.

### Constructor

`Public Sub Constructor(appName As String, hostID As String)`

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `appName` | `String` | `ByVal` | Required |
| `hostID` | `String` | `ByVal` | Required |

Returns: no value.

### ForLoopbackTesting

`Public Shared Function ForLoopbackTesting(appName As String, hostID As String, issuerURL As String) As ChatGPTAuthClient`

Fixture-only HTTP origin, accepted only for exact 127.0.0.1 plus an explicit port.

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `appName` | `String` | `ByVal` | Required |
| `hostID` | `String` | `ByVal` | Required |
| `issuerURL` | `String` | `ByVal` | Required |

#### Returns

| Type |
| --- |
| `ChatGPTAuthClient` |

### Begin

`Public Function Begin(port As Integer, callbackPath As String = "/auth/callback") As ChatGPTSignIn`

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `port` | `Integer` | `ByVal` | Required |
| `callbackPath` | `String` | `ByVal` | `"/auth/callback"` |

#### Returns

| Type |
| --- |
| `ChatGPTSignIn` |

### BeginForProfile

`Public Function BeginForProfile(profile As ChatGPTProfile, port As Integer) As ChatGPTSignIn`

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `profile` | `ChatGPTProfile` | `ByVal` | Required |
| `port` | `Integer` | `ByVal` | Required |

#### Returns

| Type |
| --- |
| `ChatGPTSignIn` |

### Exchange

`Public Function Exchange(attempt As ChatGPTSignIn) As ChatGPTProfile`

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `attempt` | `ChatGPTSignIn` | `ByVal` | Required |

#### Returns

| Type |
| --- |
| `ChatGPTProfile` |

### RestoreProfile

`Public Function RestoreProfile(record As JSONItem) As ChatGPTProfile`

Restore a protected credential record as refresh-only; no saved bearer or permission is trusted.

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `record` | `JSONItem` | `ByVal` | Required |

#### Returns

| Type |
| --- |
| `ChatGPTProfile` |

### Refresh

`Public Sub Refresh(profile As ChatGPTProfile)`

Caller must serialize refreshes per profile across all threads/processes.

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `profile` | `ChatGPTProfile` | `ByVal` | Required |

Returns: no value.

### Revoke

`Public Sub Revoke(profile As ChatGPTProfile)`

Remote revocation must succeed before this method clears local tokens.

#### Parameters

| Name | Type | Passing | Default |
| --- | --- | --- | --- |
| `profile` | `ChatGPTProfile` | `ByVal` | Required |

Returns: no value.
