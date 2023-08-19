<h1> Command format:
#<command>:param1:param2:param3:param4

<h1> Reply format:
#OK:<value1>:<value2>:<value3>:<value4>
#ERROR:<error_code>:<error_message>

<h1>Command list for wheels:

up – up wheels

up <wheel number> – up specific wheel

up <wheel number> <level number> – up specific wheel to the level

down – the same as `up`, but `down`

stop – stops the command execution

auto <level number> <timeout seconds (optional)> – automatically `up` or `down` wheels depending on the target <level number> cell.

<h1> Memory commands:

lsave <cell number> – saves the current level to the <cell number>. Max 3 cells can be used.

lget <cell number> – shows the current levels in <cell number>

lmaxsave – saves the current level as `max` for all wheels. The controller tries to not exceed this level.

lminsave – saves the current levels as `min` for all wheels.

lmaxget – shows the saved max level.

lminget – shows the saved min level.

lshow – shows the current `real time` sensors values of wheels levels. These values are used in `lsave` calls.

memclear – clears the memory.

memclear <param> – clears the memory and sets the <param> as a default value.

<h1> Compressor commands:

getcompr – displays the pressure in the compressor.

cminsave – saves the current pressure as minimum allowable before start pumping the air.

cmaxsave – saves the current pressure value as maximum to turn off the compressor.

<h1> Version commands

ver – returns the current version number

<h1> Other commands

bat – shows the battery voltage.
