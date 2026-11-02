# ChatGPT public-client PKCE sign-in and renewable sessions

A public-client OAuth package implementing OpenAI's supported open-source
Sign in with ChatGPT flow. It uses authorization-code PKCE, verifies RS256 ID
tokens against the issuer's JWKS, binds identities to issued client registrations,
and manages renewable in-memory sessions. It neither opens a browser nor writes
credentials unless the host explicitly requests and implements those actions.

The implementation follows the current official [registration and sign-in](https://developers.openai.com/siwc/token-sharing-open-source/sign-in),
[token reference](https://developers.openai.com/siwc/token-sharing-open-source/token-reference),
and [accounts and sessions](https://developers.openai.com/siwc/token-sharing-open-source/profiles-and-sessions)
documentation. Use the approved **Continue with ChatGPT** button branding in the
host UI. Preview account eligibility, model access and usage permissions remain
server-controlled; a verified identity alone does not grant plan usage.

## Install

From a Buoy consumer project with BPM installed:

```sh
bpm add chatgptauth --git https://github.com/BuoyBay/chatgptauth.git --rev refs/heads/main
```

BPM records the resolved commit in its lockfile. Keep that lockfile with the
consumer project for reproducible installs.

## Example

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

## Build and API reference

Source is in `modules/`, with native/runtime inputs where required. Use the
[package guide](docs/API.md) for backend-specific build prerequisites, ownership
rules and supported targets. Rebuilding a native backend requires that target's
compiler and SDK; sealing an archive alone does not rebuild the backend.

## Local Buoy setup

On your own machine, install Buoy and run:

```sh
buoy ai init --both
```


## Contributing

[Contributing](CONTRIBUTING.md) · [License](LICENSE)
