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
- [x] proof of concept ruby dsl scripting for callbacks to events, similar to ruby rack config.ru
```rb
##
# mruv socket handler function fizzbuzz example
#
def handler(env)
  n = env.chomp.to_i
  if n > 0 and n < 100 then
    if n % 15 == 0 then
      "fizzbuzz"
    elsif n % 3 == 0 then
      "fizz"
    elsif n % 5 == 0 then
      "buzz"
    else
      env
    end
  else
    "Must be number between 1 and 99"
  end
end

```
- [ ] support libuv TCP/IP and maybe some file stuff
