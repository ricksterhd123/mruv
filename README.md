# mruv
**mruv = mruby + libuv**

> mruv is a proof-of-concept for a scriptable web server using mruby. It's designed as single binary toolkit for creating (hopefully fast and secure) dynamic web applications.

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
- [x] libuv TCP socket handling
- [x] http request parsing & response generation
- [x] proof of concept testing in postman
- [] improve c handling + research valgrind / ASAT and other dynamic memory testing tools
- [] unit testing
- [] JSON handling library
- [] MIME handling
- [] chunked content + streaming

## Examples
![Return the request body reversed](image.png)

`config.rb`
```rb
def handler(body)
  body.reverse
end
```
