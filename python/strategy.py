from alien_functions import *


def try_describe(command):
    if isinstance(command, list) and len(command) >= 3:
        command_type = command[0]
        if command_type == 4:
            player_key = command[1]
            actions = command[2]
            return {
                'command_type': command_type,
                'player_key': player_key,
                'actions': actions
            }
        else:
            return command
    return command
