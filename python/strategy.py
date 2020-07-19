def try_describe_request(command):
    if isinstance(command, list) and len(command) == 3:
        command_type = command[0]
        if command_type == 4:
            player_key = command[1]
            actions = command[2]
            # known action samples
            # [1, 0] - destruct
            # [0, 0, (x, y)] - change speed
            return {
                'command_type': command_type,
                'player_key': player_key,
                'actions': actions
            }
        else:
            return command
    return command


def try_describe_ship(ship):
    if len(ship) != 8:
        return ship

    d = {}
    d['role'] = ship[0]
    d['id'] = ship[1]
    d['pos'] = ship[2]
    d['speed'] = ship[3]
    d['params'] = ship[4]
    d['unk6'] = ship[5]
    d['unk7'] = ship[6]
    d['unk8'] = ship[7]
    return d


def try_describe_game_state(state):
    if len(state) != 3:
        return state

    d = {}
    d['step'] = state[0]
    d['unk2'] = state[1]
    d['ships'] = [
        {
            'ship': try_describe_ship(block[0]),
            'commands': block[1]
        }
        for block in state[2]]
    return d


def try_describe_game_params(params):
    if len(params) != 5:
        return params

    d = {}
    d['max time'] = params[0]
    d['is_defender'] = params[1]
    d['unk3'] = params[2]
    d['unk4'] = params[3]
    d['enemy init ship params?? (only attacker??)'] = params[4]
    return d


def try_describe_response(response):
    if isinstance(response, list) and len(response) == 4:
        d = {}
        d['success'] = response[0]
        d['game_state'] = response[1]
        d['game params'] = try_describe_game_params(response[2])
        d['game state'] = try_describe_game_state(response[3])
        return d
    else:
        return response
