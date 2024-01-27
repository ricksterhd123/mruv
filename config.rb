# frozen_string_literal: true

##
# mruv socket handler function fizzbuzz example
#
def handler(event, context)
  puts event, context
  n = event[:body].chomp.to_i
  if n.positive? && (n < 100)
    if (n % 15).zero?
      'fizzbuzz'
    elsif (n % 3).zero?
      'fizz'
    elsif (n % 5).zero?
      'buzz'
    else
      env
    end
  else
    'Must be number between 1 and 99'
  end
end
