# ChatGPTAuth · Sign in with ChatGPT

A Buoy public-client OAuth package implementing OpenAI's supported open-source
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

## Native prerequisites

The package requires Buoy 0.72+, the bundled `crypto` package and the native
RS256 bridge in `native/`. Put the package and crypto archives in the consumer's
`packages/` directory, then `Import chatgptauth`.

macOS uses Security/CoreFoundation and Windows uses BCrypt. Linux builds need
OpenSSL development headers; deployed Linux consumers need OpenSSL 3's
`libcrypto.so.3`. Rebuild the bridge for the consumer's architecture and use a
matching Windows import library where required. A cross-compiler or SDK must be
installed separately.

## Host integration

Create and persist one `NewChatGPTHostID()` per installation/host. It is an opaque
random UUID, distinct from each account's issued OAuth client ID. Construct
`ChatGPTAuthClient(appName, stableHostID)` with your application's actual name.
Its production issuer and authorization/token endpoints are fixed to
`https://auth.openai.com`.

1. Bind a callback listener to **127.0.0.1**, selecting an available port. Keep the
   listener path stable across registrations and returning sign-ins.
2. Call `auth.Begin(port)` for a new registration, or
   `auth.BeginForProfile(selectedProfile, port)` to reauthorize a selected account.
   The default callback path is `/auth/callback`; a new registration can explicitly
   choose another path with `Begin(port, callbackPath)`.
3. Open `attempt.AuthorizationURL` in the system browser only after the listener
   is ready. This URL can contain an ID-token hint; never log it.
4. Pass the complete callback URL received on that exact listener to
   `attempt.AcceptCallback(url)`. The package checks the exact scheme, host,
   port and path, duplicate parameters, the random state, OAuth errors and the
   registration's client ID. Errors are sanitized and do not contain tokens.
5. Call `profile = auth.Exchange(attempt)` on a worker thread. Only make this
   profile active after exchange and identity validation succeed. A failed attempt
   never replaces another profile's credentials.

Initial registration uses `dynamic_agent_client` only at authorization, with
`agent_name_hint` and the stable host ID. Code exchange uses the callback-issued
client ID. Returning authorization reuses that exact issued client, host, callback
path and validated identity; only the callback port may change. State, nonce and
PKCE verifier are freshly generated for every attempt. Attempts expire after ten
minutes and are single-use; `attempt.Cancel()` invalidates and clears a pending
attempt when the host closes its listener or cancels sign-in.

The caller owns the listener, system-browser integration, timeout UI and callback
response page. Only loopback HTTP callbacks are accepted; `localhost` is not an
alias for `127.0.0.1`. The package intentionally does not intercept another
application's callbacks or use a client's private OAuth credentials.

## Profiles and token renewal

A `ChatGPTProfile` exposes nonsecret `Subject`, `Email`, `Issuer`, `ClientID`,
`HostID`, `GrantedScopes`, `PlanEnabled` and `ExpiresAt`. Keep separate profiles
for every issued-client/validated-subject pair, even when emails match. Email is
only a display hint; it does not establish an account or workspace identity.
Access-token contents remain opaque and no workspace/account ID is guessed from
them.

`profile.AccessToken()` explicitly returns the bearer token only when
`chatgpt.tokens.use.direct` and `resource.invoke` are granted and the access token
has not expired. `profile.ChildEnvironment()` explicitly returns a new map
containing `ACCESS_TOKEN` for a host's **per-child** process environment; it never
sets the global environment and must never be logged. Neither accessor checks
account entitlement beyond the actual returned scopes; the service authorizes
each request.

`auth.Refresh(profile)` uses the issued client ID and rotating refresh token.
It validates replacement identity tokens when present, checks the registration's
subject, and replaces access/refresh/ID tokens, granted scopes and expiry together
only after the entire response passes validation. An omitted refresh `scope`
retains the existing grant; an omitted refresh ID token retains already validated
identity evidence. Respect `earliest_refresh_at`, if returned. Serialize refreshes
for each session across threads and processes; the package does not coordinate
multiple processes or retry a consumed rotating refresh token automatically.

`auth.Revoke(profile)` discovers the issuer's revocation endpoint and revokes the
renewable session. Only a confirmed HTTP 200 clears local credentials. Failure
retains them for a retry. A host that elects to sign out locally despite a failed
revocation can call `profile.ClearCredentials()` and tell the user that remote
revocation was not confirmed. The client/account mapping remains intact for later
reauthorization. Signing out does not delete the registered client.

## Protected persistence

No file or keychain storage is automatic. `profile.ExportCredentials()` explicitly
returns a secret JSON object for a host's protected storage. Do not serialize it
to logs, analytics, command arguments, diagnostics, source control or browser
storage. If using a file, the host must write atomically with owner-only access;
a native credential store is another option. The export includes the original
OIDC nonce as well as the signed ID token so a later restore can check their
association.

`auth.RestoreProfile(record)` validates the retained ID token's signature,
issuer, issued-client audience, subject and nonce, and checks the saved host and
callback path. It permits an expired retained ID token solely as signed identity
evidence. The returned profile is **refresh-only**: it ignores saved access-token,
expiry and scope claims, and `AccessToken()` remains disabled until a successful
`auth.Refresh(profile)` obtains a current grant from the issuer. If that refresh
omits `scope`, the restored profile remains without plan permission until a
subsequent response grants it. Restore does not trust an unsigned saved email or
subject to replace verified claims.

## Verification and transport boundaries

ID tokens allow only `RS256`, select an unambiguous RSA key by exact `kid`, and
validate signature, exact issuer/audience/authorized party, expiry, issued-at,
optional not-before and the attempt's nonce. RSA moduli are 2048–8192 bits.
Refresh ID-token nonces, when present, must match the original sign-in. The nonce
is required on initial exchange. State/nonce comparisons use constant-time byte
comparison; client IDs, issuer strings and scopes are case-sensitive.

Untrusted JSON is size-checked and scanned before parsing: nesting above 64,
raw/escaped NUL, invalid Unicode escapes, malformed numeric tokens and non-UTF-8
text reject. JWT segments use canonical unpadded base64url. Endpoint responses
and complete ID tokens are limited to 1 MiB. HTTP response-size checks occur after
the synchronous runtime has received the body, rather than during streaming.
The host should run network methods on a worker thread and provide cancellation
UI; per-request transport timeouts are 30 seconds.

TLS verification is always enabled and redirects are never followed. Discovery,
JWKS and revocation URLs must remain on the exact configured issuer origin; token
exchange always uses the documented fixed token route. There is no custom public
production endpoint. `ForLoopbackTesting(appName, hostID, issuerURL)` is an explicit
fixture-only factory accepting only `http://127.0.0.1:<port>` with no path, userinfo
or query. Never use test configuration for real credentials.

## Codex and Responses integration

This package implements authentication; it does not launch Codex, infer models or
assume a plan includes a particular model. The current official
[Codex app-server handoff](https://developers.openai.com/siwc/token-sharing-open-source/codex-app-server)
uses a custom provider with `env_key = "ACCESS_TOKEN"`, `requires_openai_auth = false`,
`wire_api = "responses"` and `supports_websockets = false`, pointed at
`https://api.openai.com/v1`. Inject `ChildEnvironment()` into that one child process;
keep credentials out of its command arguments. After renewing the token, restart
that app-server with its new child environment and resume the saved thread. Do
not assume the normal ChatGPT-login RPC accepts SIWC account tokens or invent an
account/workspace identifier.

For direct HTTP Responses requests, follow the current
[preview limitations](https://developers.openai.com/siwc/token-sharing-open-source/preview-limitations):
`store: false`, `stream: true`, an `input` array containing needed history, and only
the fields/tools supported by this route. This package deliberately does not
translate plan credentials into a general unrestricted API key. User-visible
usage links can point to ChatGPT Settings → Usage.

The native verifier source is included for supported platform builds. Deploy an architecture-matching verifier and its native dependencies with the consumer.

The Linux verifier dynamically links OpenSSL 3 (`libcrypto.so.3`) and its system
dependencies; install a compatible OpenSSL runtime on consumer hosts.
