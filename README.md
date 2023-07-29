# Shitty JSON Parser
This is a crappy, inefficient JSON parser written in C (in like 4 hours) for self-satisfaction and personal user.

It takes inspiration form [jsmn](https://github.com/zserge/jsmn) and divides the json in tokens with start and end points.
It does a little bit more though, and also provides functions for parsing the tokens into int, float, string, boolean and null types.

# Why I made it
I was doing [Sokcet Programming](https://github.com/skadewdl3/computer-networks) and got tired of sending and receiving data
multiple times just to send some related data. So I wrote this parser to send and receive data as json.

# Why to not use it
It's quirky, there's no error handling and it'll work even if you give it shitty JSON (hence the name).
