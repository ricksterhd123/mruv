# mruv
mruv = mruby + libuv

## System Requirements
- git
- Docker
- GNU/Linux x86

## Setup Development
- Clone repository
- Load the dev container in vscode

## Installation
TBC...

## TODO
- [x] statically linked mruby + libuv (single binary)
- [ ] proof of concept ruby dsl scripting for callbacks to events, similar to ruby rack config.ru
```rb
get '/' do
  'Hello world!'
end
```
- [ ] support libuv TCP/IP and maybe some file stuff
