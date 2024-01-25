def method_name(req)
  puts req
end

def get_ten(&block)
  block.call 10
end
get_ten { |x| puts x + 10 }

# add_event_handler(&method(:method_name))
