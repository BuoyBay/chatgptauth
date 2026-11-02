---
title: "ChatGPTAuth getting started"
summary: "Create and cancel an offline sign-in attempt, then integrate the real callback lifecycle."
category: "Guides"
order: 10
keywords: ["callback", "loopback", "host", "signin", "cancel"]
related: ["index.md", "api.md", "typed-api.md"]
---

# Getting started

Place `chatgptauth-0.1.0.buipkg` and Buoy's bundled `crypto-1.0.0.buipkg` in the consumer's `packages/`, with native runtime slices appropriate to the target. Use `Import chatgptauth`.

## Construct an attempt without signing in

This complete offline example demonstrates host identity, the client, the expected callback address, and cancellation. A production host persists its host ID rather than generating a new one every launch. No listener is bound and no browser or token exchange occurs here.

```basic
Import chatgptauth

Sub Main()
    ' Persist this identifier once per host installation in a real application.
    Dim hostID As String = NewChatGPTHostID()
    Dim auth As New ChatGPTAuthClient("Example Buoy application", hostID)
    Dim pending As ChatGPTSignIn = auth.Begin(1455)
    Print "Expected callback: " + pending.CallbackURI
    ' Bind the loopback listener before opening pending.AuthorizationURL.
    ' This offline example cancels without opening a browser or exchanging tokens.
    pending.Cancel()
End Sub
```

## Complete a real sign-in

1. Bind a listener to `127.0.0.1` on an available port before opening the browser. Keep the callback path stable across registrations and returning sign-ins.
2. Call `Begin(port)` for a new registration, or `BeginForProfile(profile, port)` to reauthorize a selected existing profile.
3. Open `attempt.AuthorizationURL` in the system browser. It can contain an ID-token hint; do not log it.
4. Pass the complete callback URL received on that exact listener to `attempt.AcceptCallback(url)`.
5. Run `auth.Exchange(attempt)` on a worker thread. Activate the returned profile only after exchange and identity validation succeed.

Only loopback HTTP callbacks are accepted. `localhost` is not an alias for `127.0.0.1`. Scheme, host, port, path, duplicate parameters, random state, issuer, and issued client are validated. Attempts expire after ten minutes and are single-use. Cancel the attempt when the listener closes or sign-in is abandoned.

## Renew and store carefully

`Refresh(profile)` rotates credentials and updates identity/scopes/expiry together only after the response validates. Serialize refreshes for one session across both threads and processes; rotating refresh tokens cannot safely be consumed twice. Respect the service's earliest-refresh time.

Persistence is explicit. `ExportCredentials()` returns secret JSON for protected host storage. `RestoreProfile(record)` revalidates retained signed identity and returns a refresh-only profile; saved access-token, expiry, and scope claims are not trusted. Refresh successfully before using a restored profile for plan access.

See [API reference](api.md) for token access and revocation, or return to [ChatGPTAuth](index.md).
