puts $mruv

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
