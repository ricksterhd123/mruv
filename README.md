# mruv
mruv = mruby + libuv

## Notes
Want ruby dsl scripting for callbacks to events, similar to ruby rack config.ru
```rb
get '/' do
  'Hello world!'
end
```
supporting libuv features (primarily TCP/IP, maybe some file stuff)
