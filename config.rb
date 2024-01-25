def method_name(req)
  puts req
end
puts add_event_handler(&method(:method_name))
