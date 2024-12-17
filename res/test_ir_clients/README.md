# IR clients

It is fine to remove this folder.
This is still work in progress.

The judging algorithm doesn't yet match LR2 100% correctly either.
Some data sent to IR client is bogus stub.

## Users:

Lunatic Vibes F IR clients are not any different from any other executables.
Only use Lua scripts from sources you trust.
Dropping scripts into this folder means they will be run by the game!

## Developers:

Lunatic Vibes F follows LR2 for it's scoring mechanics.

You can get an idea of what the interface should look like from `./_meta.lua` and `./_test.lua`.

### Redistributing recommendations

It is advised to redistribute IR clients in compiled form to avoid cheating.

1. Install Lua 5.1.5 interpreter.
2. Run `luac -v` and make sure it says `Lua 5.1.5`.
3. Run `luac -o ./my_client.lua ./my_client_source.lua`.
4. Try using new `./my_client.lua`. Compiled Lua files can be used drop-in like usual Lua scripts.
